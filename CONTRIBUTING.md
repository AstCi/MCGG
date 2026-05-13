# Contributing

Thanks for helping improve MCGG. This project is focused on native Android
modding research for Magic Chess Go Go, so contributions should stay readable,
scoped, and easy to verify.

## Responsible Use

Only contribute changes intended for learning, research, reverse engineering
practice, and authorized local testing. Do not add game APKs, copyrighted game
assets, paid content, bypasses, or
instructions for abusing online services.

## Project Setup

Clone with submodules:

```sh
git clone --recursive https://github.com/Yan-0001/MCGG.git
cd MCGG
```

If submodules are missing:

```sh
git submodule update --init --recursive
```

Pull Git LFS files:

```sh
git lfs install
git lfs pull
```

## Development Guidelines

- Keep source changes focused on the requested feature or fix.
- Use `dump/dump.cs` to verify IL2CPP class names, method signatures, return
  types, and fields before changing native calls.
- Keep the default target `arm64-v8a`.
- Keep Unity compatibility aligned with `2019.4.22f1`.
- Do not commit generated `libs/` or `obj/` output.
- Do not edit vendored directories such as `jni/Il2CppVersions/`, `jni/imgui/`,
  or `jni/xDL/` unless the change explicitly requires it.

## Coding Style

Follow the existing C++ style in `jni/Main.cpp`:

- Use 4-space indentation.
- Prefer explicit pointer types.
- Keep helper code small and direct.
- Add short comments above simple functions or non-obvious blocks.
- Keep existing UI element names, method names, and hook names stable unless a
  rename is part of the requested change.

## Build Verification

Run this before submitting native code changes:

```sh
ndk-build -C jni
```

The expected output is:

```text
libs/arm64-v8a/libmain.so
```

Documentation-only changes do not require a native build, but mention that in
the pull request.

## Commit Messages

Use short typed commit messages when possible:

```text
feat/main: add player sorting
fix/main: guard missing battle data
perf/main: optimize player table rendering
docs/readme: update setup guide
chore/repo: ignore build outputs
```

## Pull Requests

Pull requests should include:

- A short summary of the change.
- The files or areas affected.
- The build command result, usually `ndk-build -C jni`.
- Notes about any IL2CPP signatures, fields, or hooks changed.
- Screenshots only when the ImGui overlay changes visually.

Small, focused pull requests are easiest to review and merge.
