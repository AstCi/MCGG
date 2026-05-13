#include <jni.h>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "structures/Structures.hpp"
#include "xdl.h"
#include "dobby/dobby.h"

#include "imgui/imconfig.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imstb_rectpack.h"
#include "imgui/imstb_textedit.h"
#include "imgui/imstb_truetype.h"
#include "imgui/backends/imgui_impl_android.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/misc/cpp/imgui_stdlib.h"

#define DO_API(ret, name, args) ret (*name) args;
#include "Il2CppVersions/api/2019.4.22f1.h"
#undef DO_API

enum class TouchPhase {
    Began,
    Moved,
    Stationary,
    Ended,
    Canceled
};

enum class TouchType {
    Direct,
    Indirect,
    Stylus
};

struct Touch {
    int m_FingerId;
    Unity::Vector2 m_Position;
    Unity::Vector2 m_RawPosition;
    Unity::Vector2 m_PositionDelta;
    float m_TimeDelta;
    int m_TapCount;
    TouchPhase m_Phase;
    TouchType m_Type;
    float m_Pressure;
    float m_maximumPossiblePressure;
    float m_Radius;
    float m_RadiusVariance;
    float m_AltitudeAngle;
    float m_AzimuthAngle;
};

struct {
    void* libil2cpp = nullptr;
    void* liblogic = nullptr;
} handle;

int GLWidth = 0;
int GLHeight = 0;

void* UnityLibraryHandle = nullptr;

std::unordered_map<std::string, std::vector<MethodInfo*>> MultiMethodCache;
std::unordered_map<std::string, FieldInfo*> FieldCache;

namespace Originals {
    EGLBoolean (*EglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
    Touch (*Input_GetTouch)(int index);

    Il2CppString* (*MCLogicBattleData_ILOGIC_GetSelfChessPlayerName)(
        void* instance,
        uint64_t accID
    );
    MonoStructures::Dictionary<uint64_t, void*>* (*MCLogicBattleData_ILOGIC_GetAllBattleMgr)(
        void* instance
    );
    uint64_t (*MCLogicBattleData_ILOGIC_GetCurrentOpponentAccountID)(
        void* instance,
        uint64_t accID
    );
}

// Checks whether the current process is the Unity target process.
bool IsUnityProcess() {
    FILE* fp = fopen("/proc/self/cmdline", "r");
    if (!fp) {
        return false;
    }

    char buffer[1024];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, fp);
    fclose(fp);

    if (len == 0) {
        return false;
    }

    buffer[len] = '\0';
    return strstr(buffer, ":UnityKillsMe") != nullptr;
}

// Returns true when needle appears in haystack without case sensitivity.
bool StringIncludesCaseInsensitive(
    const std::string& haystack,
    const std::string& needle
) {
    auto it = std::search(
        haystack.begin(),
        haystack.end(),
        needle.begin(),
        needle.end(),
        [](char ch1, char ch2) {
            return std::toupper(static_cast<unsigned char>(ch1)) ==
                   std::toupper(static_cast<unsigned char>(ch2));
        }
    );

    return it != haystack.end();
}

// Builds a stable key for caching resolved IL2CPP methods.
std::string GenerateCacheKey(
    const char* ns,
    const char* className,
    const char* methodName,
    const std::vector<const char*>& paramTypes
) {
    std::string key =
        (ns ? std::string(ns) : "") +
        "::" +
        className +
        "::" +
        methodName +
        "(";

    for (size_t i = 0; i < paramTypes.size(); ++i) {
        key += paramTypes[i];

        if (i < paramTypes.size() - 1) {
            key += ",";
        }
    }

    key += ")";
    return key;
}

// Builds a stable key for caching resolved IL2CPP fields.
std::string GenerateFieldCacheKey(
    const char* ns,
    const char* className,
    const char* fieldName
) {
    return (ns ? std::string(ns) : "") +
        "::" +
        className +
        "::" +
        fieldName;
}

// Resolves a class name, including nested class paths.
Il2CppClass* ResolveClassFromName(
    const Il2CppImage* image,
    const char* namespaze,
    const char* className
) {
    Il2CppClass* klass = il2cpp_class_from_name(image, namespaze, className);
    if (klass) {
        return klass;
    }

    char nameCopy[512]{0};
    strncpy(nameCopy, className, sizeof(nameCopy) - 1);

    char* ctx = nullptr;
    char* token = strtok_r(nameCopy, ".+/", &ctx);

    if (!token) {
        return nullptr;
    }

    Il2CppClass* current = il2cpp_class_from_name(image, namespaze, token);

    while (current && (token = strtok_r(nullptr, ".+/", &ctx))) {
        void* iter = nullptr;
        Il2CppClass* nested = nullptr;
        Il2CppClass* found = nullptr;

        while ((nested = il2cpp_class_get_nested_types(current, &iter))) {
            const char* nestedName = il2cpp_class_get_name(nested);

            if (nestedName && strcmp(nestedName, token) == 0) {
                found = nested;
                break;
            }
        }

        current = found;
    }

    return current;
}

// Searches a class and its parents for a field.
FieldInfo* FindFieldInClassHierarchy(Il2CppClass* klass, const char* fieldName) {
    if (!klass || !fieldName) {
        return nullptr;
    }

    Il2CppClass* currentKlass = klass;

    while (currentKlass) {
        FieldInfo* field = nullptr;

        if (il2cpp_class_get_field_from_name) {
            field = il2cpp_class_get_field_from_name(currentKlass, fieldName);
            if (field) {
                return field;
            }
        }

        if (il2cpp_class_get_fields && il2cpp_field_get_name) {
            void* iter = nullptr;

            while ((field = il2cpp_class_get_fields(currentKlass, &iter))) {
                const char* currentFieldName = il2cpp_field_get_name(field);

                if (currentFieldName && strcmp(currentFieldName, fieldName) == 0) {
                    return field;
                }
            }
        }

        if (!il2cpp_class_get_parent) {
            break;
        }

        currentKlass = il2cpp_class_get_parent(currentKlass);
    }

    return nullptr;
}

// Resolves and caches IL2CPP field metadata by class and field name.
FieldInfo* GetFieldInfoFromName(
    const char* ns,
    const char* className,
    const char* fieldName
) {
    if (!className || !fieldName || !il2cpp_domain_get || !il2cpp_domain_get_assemblies) {
        return nullptr;
    }

    std::string cacheKey = GenerateFieldCacheKey(ns, className, fieldName);

    auto cached = FieldCache.find(cacheKey);
    if (cached != FieldCache.end()) {
        return cached->second;
    }

    size_t size = 0;
    Il2CppDomain* domain = il2cpp_domain_get();

    if (!domain) {
        return nullptr;
    }

    const Il2CppAssembly** assemblies = il2cpp_domain_get_assemblies(domain, &size);
    if (!assemblies || size == 0) {
        return nullptr;
    }

    for (size_t i = 0; i < size; ++i) {
        const Il2CppImage* image = il2cpp_assembly_get_image(assemblies[i]);
        if (!image) {
            continue;
        }

        Il2CppClass* klass = ResolveClassFromName(image, ns, className);
        if (!klass) {
            continue;
        }

        FieldInfo* field = FindFieldInClassHierarchy(klass, fieldName);
        if (field) {
            FieldCache[cacheKey] = field;
            return field;
        }
    }

    FieldCache[cacheKey] = nullptr;
    return nullptr;
}

// Reads an instance field by metadata into a caller-provided buffer.
bool GetFieldRaw(Il2CppObject* instance, FieldInfo* field, void* outValue) {
    if (!instance || !field || !outValue || !il2cpp_field_get_value) {
        return false;
    }

    il2cpp_field_get_value(instance, field, outValue);
    return true;
}

// Reads an instance field by name into a caller-provided buffer.
bool GetFieldRaw(
    Il2CppObject* instance,
    const char* ns,
    const char* className,
    const char* fieldName,
    void* outValue
) {
    return GetFieldRaw(
        instance,
        GetFieldInfoFromName(ns, className, fieldName),
        outValue
    );
}

// Reads a static field by metadata into a caller-provided buffer.
bool GetStaticFieldRaw(FieldInfo* field, void* outValue) {
    if (!field || !outValue || !il2cpp_field_static_get_value) {
        return false;
    }

    il2cpp_field_static_get_value(field, outValue);
    return true;
}

// Reads a static field by name into a caller-provided buffer.
bool GetStaticFieldRaw(
    const char* ns,
    const char* className,
    const char* fieldName,
    void* outValue
) {
    return GetStaticFieldRaw(
        GetFieldInfoFromName(ns, className, fieldName),
        outValue
    );
}

// Writes an instance field by metadata from a caller-provided buffer.
bool SetFieldRaw(Il2CppObject* instance, FieldInfo* field, const void* value) {
    if (!instance || !field || !value || !il2cpp_field_set_value) {
        return false;
    }

    il2cpp_field_set_value(instance, field, const_cast<void*>(value));
    return true;
}

// Writes an instance field by name from a caller-provided buffer.
bool SetFieldRaw(
    Il2CppObject* instance,
    const char* ns,
    const char* className,
    const char* fieldName,
    const void* value
) {
    return SetFieldRaw(
        instance,
        GetFieldInfoFromName(ns, className, fieldName),
        value
    );
}

// Writes a static field by metadata from a caller-provided buffer.
bool SetStaticFieldRaw(FieldInfo* field, const void* value) {
    if (!field || !value || !il2cpp_field_static_set_value) {
        return false;
    }

    il2cpp_field_static_set_value(field, const_cast<void*>(value));
    return true;
}

// Writes a static field by name from a caller-provided buffer.
bool SetStaticFieldRaw(
    const char* ns,
    const char* className,
    const char* fieldName,
    const void* value
) {
    return SetStaticFieldRaw(
        GetFieldInfoFromName(ns, className, fieldName),
        value
    );
}

// Reads a typed instance field by metadata.
template <typename T>
T GetField(Il2CppObject* instance, FieldInfo* field) {
    T value{};
    GetFieldRaw(instance, field, &value);
    return value;
}

// Reads a typed instance field by name.
template <typename T>
T GetField(
    Il2CppObject* instance,
    const char* ns,
    const char* className,
    const char* fieldName
) {
    T value{};
    GetFieldRaw(instance, ns, className, fieldName, &value);
    return value;
}

// Reads a typed static field by metadata.
template <typename T>
T GetStaticField(FieldInfo* field) {
    T value{};
    GetStaticFieldRaw(field, &value);
    return value;
}

// Reads a typed static field by name.
template <typename T>
T GetStaticField(
    const char* ns,
    const char* className,
    const char* fieldName
) {
    T value{};
    GetStaticFieldRaw(ns, className, fieldName, &value);
    return value;
}

// Writes a typed instance field by metadata.
template <typename T>
bool SetField(Il2CppObject* instance, FieldInfo* field, const T& value) {
    return SetFieldRaw(instance, field, &value);
}

// Writes a typed instance field by name.
template <typename T>
bool SetField(
    Il2CppObject* instance,
    const char* ns,
    const char* className,
    const char* fieldName,
    const T& value
) {
    return SetFieldRaw(instance, ns, className, fieldName, &value);
}

// Writes a typed static field by metadata.
template <typename T>
bool SetStaticField(FieldInfo* field, const T& value) {
    return SetStaticFieldRaw(field, &value);
}

// Writes a typed static field by name.
template <typename T>
bool SetStaticField(
    const char* ns,
    const char* className,
    const char* fieldName,
    const T& value
) {
    return SetStaticFieldRaw(ns, className, fieldName, &value);
}

// Finds all matching IL2CPP method metadata entries by class, method, and params.
std::vector<MethodInfo*> GetAllMethodInfosFromName(
    const char* ns,
    const char* className,
    const char* methodName,
    const std::vector<const char*>& paramTypes
) {
    std::string cacheKey = GenerateCacheKey(ns, className, methodName, paramTypes);

    auto cached = MultiMethodCache.find(cacheKey);
    if (cached != MultiMethodCache.end()) {
        return cached->second;
    }

    std::vector<MethodInfo*> foundMethods;

    size_t size = 0;
    Il2CppDomain* domain = il2cpp_domain_get();

    if (!domain) {
        return foundMethods;
    }

    const Il2CppAssembly** assemblies = il2cpp_domain_get_assemblies(domain, &size);
    if (!assemblies || size == 0) {
        return foundMethods;
    }

    for (size_t i = 0; i < size; ++i) {
        const Il2CppImage* image = il2cpp_assembly_get_image(assemblies[i]);
        if (!image) {
            continue;
        }

        Il2CppClass* klass = ResolveClassFromName(image, ns, className);
        if (!klass) {
            continue;
        }

        Il2CppClass* currentKlass = klass;

        while (currentKlass) {
            void* iter = nullptr;
            MethodInfo* method = nullptr;

            while ((method = (MethodInfo*)il2cpp_class_get_methods(currentKlass, &iter))) {
                const char* currentMethodName = il2cpp_method_get_name(method);

                if (!currentMethodName || strcmp(currentMethodName, methodName) != 0) {
                    continue;
                }

                uint32_t paramCount = il2cpp_method_get_param_count(method);
                if (paramCount != paramTypes.size()) {
                    continue;
                }

                bool paramsMatch = true;

                for (uint32_t p = 0; p < paramCount; ++p) {
                    const Il2CppType* paramType = il2cpp_method_get_param(method, p);
                    Il2CppClass* paramClass = il2cpp_class_from_type(paramType);
                    const char* paramName = paramClass ? il2cpp_class_get_name(paramClass) : "";

                    if (!StringIncludesCaseInsensitive(paramName, paramTypes[p])) {
                        paramsMatch = false;
                        break;
                    }
                }

                if (paramsMatch) {
                    foundMethods.push_back(method);
                    break;
                }
            }

            currentKlass = il2cpp_class_get_parent(currentKlass);
        }

        if (!foundMethods.empty()) {
            break;
        }
    }

    MultiMethodCache[cacheKey] = foundMethods;
    return foundMethods;
}

// Converts resolved IL2CPP method metadata entries into callable pointers.
std::vector<void*> GetAllMethodsFromName(
    const char* ns,
    const char* className,
    const char* methodName,
    const std::vector<const char*>& paramTypes
) {
    std::vector<MethodInfo*> infos =
        GetAllMethodInfosFromName(ns, className, methodName, paramTypes);

    std::vector<void*> pointers;

    for (MethodInfo* info : infos) {
        if (info && info->methodPointer) {
            pointers.push_back((void*)info->methodPointer);
        }
    }

    return pointers;
}

namespace Hooks {
    // Renders the ImGui overlay before the frame is swapped.
    EGLBoolean EglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
        static bool ImGui_Init = false;

        eglQuerySurface(dpy, surface, EGL_WIDTH, &GLWidth);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &GLHeight);

        if (!ImGui_Init) {
            IMGUI_CHECKVERSION();

            ImGui::CreateContext();

            ImGuiIO& io = ImGui::GetIO();
            io.IniFilename = nullptr;
            io.ConfigWindowsMoveFromTitleBarOnly = true;
            io.ConfigWindowsResizeFromEdges = false;

            ImGui_ImplOpenGL3_Init("#version 300 es");
            ImGui::StyleColorsDark();

            ImGuiStyle& style = ImGui::GetStyle();
            style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
            style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
            style.ScaleAllSizes(1.0f);

            il2cpp_thread_attach(il2cpp_domain_get());

            ImGui_Init = true;
        }

        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)GLWidth, (float)GLHeight);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_Once);

        if (ImGui::Begin("MCGG", nullptr)) {
            if (ImGui::BeginTabBar("##MainTabBar")) {
                if (ImGui::BeginTabItem("Info")) {
                    ImGui::SeparatorText("Players");

                    if (!Originals::MCLogicBattleData_ILOGIC_GetAllBattleMgr ||
                        !Originals::MCLogicBattleData_ILOGIC_GetCurrentOpponentAccountID ||
                        !Originals::MCLogicBattleData_ILOGIC_GetSelfChessPlayerName) {
                        ImGui::TextUnformatted("Waiting for battle data");
                    } else {
                        MonoStructures::Dictionary<uint64_t, void*>* battleManagers =
                            Originals::MCLogicBattleData_ILOGIC_GetAllBattleMgr(nullptr);

                        if (!battleManagers || battleManagers->empty()) {
                            ImGui::TextUnformatted("No players found");
                        } else if (ImGui::BeginTable(
                            "##EnemyPredictorTable",
                            4,
                            ImGuiTableFlags_Borders |
                                ImGuiTableFlags_RowBg |
                                ImGuiTableFlags_SizingStretchProp |
                                ImGuiTableFlags_ScrollY,
                            ImVec2(0.0f, 300.0f)
                        )) {
                            ImGui::TableSetupColumn("Player");
                            ImGui::TableSetupColumn("Account");
                            ImGui::TableSetupColumn("Enemy");
                            ImGui::TableSetupColumn("Enemy Account");
                            ImGui::TableHeadersRow();

                            auto entries = battleManagers->toVector();

                            for (const auto& entry : entries) {
                                uint64_t accountId = entry.first;

                                if (accountId == 0) {
                                    continue;
                                }

                                uint64_t enemyId = 0;
                                enemyId = Originals::MCLogicBattleData_ILOGIC_GetCurrentOpponentAccountID(
                                    nullptr,
                                    accountId
                                );

                                std::string playerName;
                                std::string enemyName;

                                Il2CppString* managedPlayerName =
                                    Originals::MCLogicBattleData_ILOGIC_GetSelfChessPlayerName(
                                        nullptr,
                                        accountId
                                    );

                                if (managedPlayerName) {
                                    playerName =
                                        reinterpret_cast<MonoStructures::String*>(managedPlayerName)->str();
                                }

                                if (enemyId != 0) {
                                    Il2CppString* managedEnemyName =
                                        Originals::MCLogicBattleData_ILOGIC_GetSelfChessPlayerName(
                                            nullptr,
                                            enemyId
                                        );

                                    if (managedEnemyName) {
                                        enemyName =
                                            reinterpret_cast<MonoStructures::String*>(managedEnemyName)->str();
                                    }
                                }

                                ImGui::TableNextRow();

                                ImGui::TableSetColumnIndex(0);
                                ImGui::TextUnformatted(playerName.empty() ? "Unknown" : playerName.c_str());

                                ImGui::TableSetColumnIndex(1);
                                ImGui::Text("%llu", static_cast<unsigned long long>(accountId));

                                ImGui::TableSetColumnIndex(2);
                                ImGui::TextUnformatted(enemyName.empty() ? "Unknown" : enemyName.c_str());

                                ImGui::TableSetColumnIndex(3);
                                if (enemyId != 0) {
                                    ImGui::Text("%llu", static_cast<unsigned long long>(enemyId));
                                } else {
                                    ImGui::TextUnformatted("-");
                                }
                            }

                            ImGui::EndTable();
                        }
                    }
                    
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        return Originals::EglSwapBuffers(dpy, surface);
    }

    // Forwards Unity touch input into ImGui mouse input.
    Touch Input_GetTouch(int index) {
        Touch ret = Originals::Input_GetTouch(index);

        if (ImGui::GetCurrentContext() != nullptr && index == 0) {
            ImGuiIO& io = ImGui::GetIO();

            float x = ret.m_Position.x;
            float y = io.DisplaySize.y - ret.m_Position.y;

            switch (ret.m_Phase) {
                case TouchPhase::Began:
                    io.AddMousePosEvent(x, y);
                    io.AddMouseButtonEvent(0, true);
                    break;

                case TouchPhase::Moved:
                case TouchPhase::Stationary:
                    io.AddMousePosEvent(x, y);
                    break;

                case TouchPhase::Ended:
                case TouchPhase::Canceled:
                    io.AddMousePosEvent(x, y);
                    io.AddMouseButtonEvent(0, false);
                    break;

                default:
                    break;
            }
        }

        return ret;
    }
}

// Waits for game libraries, resolves IL2CPP APIs, and installs hooks.
void SetupThread() {
    while (!handle.libil2cpp) {
        sleep(2);
        handle.libil2cpp = xdl_open("libil2cpp.so", XDL_DEFAULT);
    }

    while (!handle.liblogic) {
        sleep(2);
        handle.liblogic = xdl_open("liblogic.so", XDL_DEFAULT);
    }

    sleep(2);

#define DO_API(ret, name, args) \
    name = reinterpret_cast<decltype(name)>(xdl_sym(handle.libil2cpp, #name, nullptr));

#include "Il2CppVersions/api/2019.4.22f1.h"

#undef DO_API

    if (!il2cpp_domain_get || !il2cpp_thread_attach) {
        return;
    }

    Il2CppDomain* domain = il2cpp_domain_get();
    if (!domain) {
        return;
    }

    il2cpp_thread_attach(domain);

    DobbyHook(
        DobbySymbolResolver(nullptr, "eglSwapBuffers"),
        (void*)Hooks::EglSwapBuffers,
        (void**)&Originals::EglSwapBuffers
    );

    auto GetTouch_Methods =
        GetAllMethodsFromName("UnityEngine", "Input", "GetTouch", {"int"});

    if (!GetTouch_Methods.empty()) {
        DobbyHook(
            GetTouch_Methods[0],
            (void*)Hooks::Input_GetTouch,
            (void**)&Originals::Input_GetTouch
        );
    }

    auto ILOGIC_GetSelfChessPlayerName_Methods =
        GetAllMethodsFromName("", "MCLogicBattleData", "ILOGIC_GetSelfChessPlayerName", {"UInt64"});

    if (!ILOGIC_GetSelfChessPlayerName_Methods.empty()) {
        Originals::MCLogicBattleData_ILOGIC_GetSelfChessPlayerName = reinterpret_cast<decltype(Originals::MCLogicBattleData_ILOGIC_GetSelfChessPlayerName)>(ILOGIC_GetSelfChessPlayerName_Methods[0]);
    }

    auto ILOGIC_GetAllBattleMgr_Methods =
        GetAllMethodsFromName("", "MCLogicBattleData", "ILOGIC_GetAllBattleMgr", {});

    if (!ILOGIC_GetAllBattleMgr_Methods.empty()) {
        Originals::MCLogicBattleData_ILOGIC_GetAllBattleMgr = reinterpret_cast<decltype(Originals::MCLogicBattleData_ILOGIC_GetAllBattleMgr)>(ILOGIC_GetAllBattleMgr_Methods[0]);
    }

    auto ILOGIC_GetCurrentOpponentAccountID_Methods =
        GetAllMethodsFromName("", "MCLogicBattleData", "ILOGIC_GetCurrentOpponentAccountID", {"UInt64"});

    if (!ILOGIC_GetCurrentOpponentAccountID_Methods.empty()) {
        Originals::MCLogicBattleData_ILOGIC_GetCurrentOpponentAccountID = reinterpret_cast<decltype(Originals::MCLogicBattleData_ILOGIC_GetCurrentOpponentAccountID)>(ILOGIC_GetCurrentOpponentAccountID_Methods[0]);
    }
}

// Starts hook setup when this shared library is loaded in the target process.
__attribute__((constructor))
void InitLibrary() {
    if (!IsUnityProcess()) {
        return;
    }

    std::thread(SetupThread).detach();
}

// Loads the original Unity library and forwards its JNI_OnLoad.
jboolean LoadOriginalLibrary(JNIEnv* env, jobject, jstring path) {
    if (!env || !path) {
        return JNI_FALSE;
    }

    const char* libraryPath = env->GetStringUTFChars(path, nullptr);
    if (!libraryPath) {
        return JNI_FALSE;
    }

    char fullPath[1024];
    snprintf(fullPath, sizeof(fullPath), "%s/%s", libraryPath, "libunity.so");

    env->ReleaseStringUTFChars(path, libraryPath);

    UnityLibraryHandle = dlopen(fullPath, RTLD_LAZY | RTLD_LOCAL);
    if (!UnityLibraryHandle) {
        return JNI_FALSE;
    }

    auto jniOnLoad =
        (jint (*)(JavaVM*, void*))dlsym(UnityLibraryHandle, "JNI_OnLoad");

    if (!jniOnLoad) {
        dlclose(UnityLibraryHandle);
        UnityLibraryHandle = nullptr;
        return JNI_FALSE;
    }

    JavaVM* vm = nullptr;

    if (env->GetJavaVM(&vm) != JNI_OK || !vm) {
        dlclose(UnityLibraryHandle);
        UnityLibraryHandle = nullptr;
        return JNI_FALSE;
    }

    jint result = jniOnLoad(vm, nullptr);

    if (result < JNI_VERSION_1_6) {
        dlclose(UnityLibraryHandle);
        UnityLibraryHandle = nullptr;
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

// Unloads the original Unity library handle if it is open.
jboolean UnloadOriginalLibrary(JNIEnv*, jclass) {
    if (UnityLibraryHandle) {
        dlclose(UnityLibraryHandle);
        UnityLibraryHandle = nullptr;
    }

    return JNI_TRUE;
}

// Registers the native loader bridge used by the Unity Java side.
extern "C" __attribute__((used, visibility("default")))
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* key) {
    if (!vm) {
        return JNI_VERSION_1_6;
    }

    JNIEnv* env = nullptr;

    if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK || !env) {
        return JNI_VERSION_1_6;
    }

    jclass clazz = env->FindClass("com/unity3d/player/NativeLoader");
    if (!clazz) {
        return JNI_VERSION_1_6;
    }

    const JNINativeMethod methods[] = {
        {
            "load",
            "(Ljava/lang/String;)Z",
            (void*)LoadOriginalLibrary
        },
        {
            "unload",
            "()Z",
            (void*)UnloadOriginalLibrary
        }
    };

    if (env->RegisterNatives(
            clazz,
            methods,
            sizeof(methods) / sizeof(JNINativeMethod)
        ) != JNI_OK) {
        return JNI_VERSION_1_6;
    }

    return JNI_VERSION_1_6;
}
