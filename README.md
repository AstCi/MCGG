# MCGG

![Platform](https://img.shields.io/badge/platform-Android-brightgreen)
![ABI](https://img.shields.io/badge/ABI-arm64--v8a-blue)
![Unity](https://img.shields.io/badge/Unity-2019.4.22f1-black)
![License](https://img.shields.io/badge/license-MIT-green)

MCGG is a native Android modding project for Magic Chess Go Go. It builds an
`arm64-v8a` shared library for a Unity IL2CPP runtime, with runtime method
resolution, Dobby hooks, xDL symbol lookup, and a Dear ImGui overlay.

This project is intended for research, reverse engineering practice, and
authorized local testing. It is not affiliated with, endorsed by, or sponsored by
the Magic Chess Go Go developers or publishers.

## Highlights

- Native Android shared library built with `ndk-build`
- Unity `2019.4.22f1` IL2CPP headers and API declarations
- `arm64-v8a` only build target
- Dobby-based native hooks
- xDL-based Android ELF and symbol lookup
- Dear ImGui overlay rendered through OpenGL ES 3
- Unity touch input forwarding into ImGui mouse input
- Local `Structures.hpp` helpers for Unity, Mono, delegate, event, list, array,
  dictionary, and string layouts
- IL2CPP method and field resolver helpers in `jni/Main.cpp`
- Enemy Predictor overlay powered by:
  - `ILOGIC_GetAllBattleMgr`
  - `ILOGIC_GetCurrentOpponentAccountID`
  - `ILOGIC_GetSelfChessPlayerName`

## Requirements

- Git
- Git LFS
- Android SDK
- Android NDK r29 or compatible
- `ndk-build` available in `PATH`
- An `arm64-v8a` Android target environment

The GitHub Actions workflow uses:

```sh
ANDROID_NDK_VERSION=29.0.14206865
```

## Quick Start

Clone with submodules:

```sh
git clone --recursive https://github.com/Yan-0001/MCGG.git
cd MCGG
```

If the repository was cloned without submodules:

```sh
git submodule update --init --recursive
```

Pull Git LFS assets:

```sh
git lfs install
git lfs pull
```

Build from the repository root:

```sh
ndk-build -C jni
```

The main output is:

```text
libs/arm64-v8a/libmain.so
```

## Repository Layout

```text
.github/workflows/            GitHub Actions CI workflow
dump/dump.cs                  IL2CPP dump used as a signature reference
jni/Android.mk                Native module build configuration
jni/Application.mk            ABI, platform, STL, and NDK settings
jni/Main.cpp                  Hook setup, IL2CPP helpers, and ImGui overlay
jni/structures/Structures.hpp Unity, Mono, delegate, event, and collection types
jni/dobby/                    Dobby header and arm64 static library
jni/Il2CppVersions/           Unity IL2CPP headers and API declarations
jni/imgui/                    Dear ImGui source
jni/xDL/                      xDL Android dynamic loader utilities
libs/                         Installed shared library output
obj/                          NDK intermediate build output
```

## Build Configuration

The native module is defined as:

```make
LOCAL_MODULE := main
```

The active Android target is configured in `jni/Application.mk`:

```make
APP_ABI := arm64-v8a
APP_PLATFORM := android-21
APP_STL := c++_static
```

Unity compatibility defines are configured in `jni/Android.mk`:

```make
-DUNITY_VERSION_MAJOR=2019
-DUNITY_VERSION_MINOR=4
-DUNITY_VERSION_PATCH=22
-DUNITY_VER=194
```

## Runtime Flow

At load time, `jni/Main.cpp`:

1. Confirms the current process is the Unity target process.
2. Starts a setup thread.
3. Waits for `libil2cpp.so` and `liblogic.so`.
4. Resolves IL2CPP API exports.
5. Attaches to the IL2CPP domain.
6. Hooks `eglSwapBuffers`.
7. Hooks `UnityEngine.Input.GetTouch`.
8. Renders the ImGui overlay during frame presentation.
9. Forwards Unity touch input into ImGui mouse input.

## IL2CPP Notes

The project currently targets Magic Chess Go Go builds using Unity
`2019.4.22f1`. Method signatures, return types, parameters, and class names
should be checked against `dump/dump.cs` before adding or changing native calls.

Useful rules:

- Use `uint64_t` for `System.UInt64`
- Use `uint32_t` for `System.UInt32`
- Use `void*` for managed object instances unless a local layout is required
- Keep function pointer signatures aligned with the dump
- Rebuild after changing `jni/Main.cpp` or `jni/structures/Structures.hpp`

## Troubleshooting

If submodule files are missing:

```sh
git submodule update --init --recursive
```

If `dump/dump.cs` is missing or appears as a Git LFS pointer:

```sh
git lfs pull
```

If `ndk-build` is not found:

```sh
export ANDROID_SDK_ROOT=/path/to/android-sdk
export PATH="$ANDROID_SDK_ROOT/ndk/29.0.14206865:$PATH"
```

If Dobby cannot be linked, confirm this file exists:

```text
jni/dobby/lib/arm64-v8a/libdobby.a
```

## Contributing

Focused pull requests are easiest to review. Keep changes scoped, document new
IL2CPP methods with their dump signatures, and run:

```sh
ndk-build -C jni
```

before submitting native code changes.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for the
full text.
