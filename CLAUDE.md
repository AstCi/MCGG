# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Development Commands

### Build & Setup
- **Initialize submodules**: `git submodule update --init --recursive`
- **Pull LFS assets**: `git lfs pull`
- **Build native library**: `ndk-build -C jni`
- **Build Output**: `libs/arm64-v8a/libmain.so`

### Verification
- There is no unit test framework. Verification is a successful build: `ndk-build -C jni`.
- For IL2CPP changes, verify method signatures and field offsets against `dump/dump.cs`.

## Code Architecture & Standards

### High-Level Architecture
- **Core Logic**: `jni/Main.cpp` handles the entire mod lifecycle: process verification → setup thread → dependency resolution (`libil2cpp.so`, `liblogic.so`) → IL2CPP API attachment → game method resolution → function hooking (via Dobby) → ImGui overlay rendering.
- **Hooking Strategy**: Uses Dobby to hook `eglSwapBuffers` for frame-by-frame UI injection and `UnityEngine.Input.GetTouch` for touch-to-mouse forwarding.
- **Memory Mapping**: `jni/structures/Structures.hpp` defines the layout of Unity/Mono types to allow safe native interaction with managed objects.
- **Reference**: `dump/dump.cs` serves as the source of truth for the target game's internal C# structure.

### Project Constraints
- **Target ABI**: `arm64-v8a`
- **Unity Version**: `2019.4.22f1` (Refer to `UNITY_` macros in `jni/Android.mk`)
- **C++ Standard**: C++26 (defined in `jni/Android.mk`)

### Coding Style
- **Indentation**: 4 spaces.
- **Naming**: Unity compatibility macros must use the `UNITY_` prefix.
- **Pointers**: Prefer `void*` for managed objects unless a specific structure from `Structures.hpp` is required.
- **Scope**: Do not modify vendored directories (`jni/imgui/`, `jni/xDL/`, `jni/dobby/`, `jni/Il2CppVersions/`) unless explicitly requested.
