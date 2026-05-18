#!/usr/bin/env bash
# Builds pinned OpenSSL, libpsl, and curl as static Android libraries for ndk-build.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
CURL_SRC_DIR="${CURL_SRC_DIR:-$PROJECT_ROOT/jni/curl}"
CURL_WORK_DIR="$PROJECT_ROOT/obj/curl-source"
OPENSSL_SRC_DIR="${OPENSSL_SRC_DIR:-$PROJECT_ROOT/jni/openssl}"
OPENSSL_WORK_DIR="$PROJECT_ROOT/obj/openssl-source"
LIBPSL_SRC_DIR="${LIBPSL_SRC_DIR:-$PROJECT_ROOT/jni/libpsl}"
LIBPSL_WORK_DIR="$PROJECT_ROOT/obj/libpsl-source"

NDK="${NDK:-${ANDROID_NDK_ROOT:-${ANDROID_HOME:-}}}"
if [[ -z "$NDK" ]]; then
    for candidate in \
        "$HOME/android-sdk/ndk/android-ndk-r29" \
        "$HOME/android-sdk/ndk/android-ndk-r28" \
        "$HOME/android-sdk/ndk/android-ndk-r27" \
        "$HOME/android-sdk/ndk/android-ndk-r26" \
        "$HOME/android-sdk/ndk/android-ndk-r25" \
        "/opt/android-sdk/ndk/android-ndk-r29" \
        "/usr/local/android-sdk/ndk/android-ndk-r29"; do
        if [[ -d "$candidate" ]]; then
            NDK="$candidate"
            break
        fi
    done
fi

if [[ -z "$NDK" || ! -d "$NDK" ]]; then
    echo "ERROR: Android NDK not found. Set NDK=/path/to/ndk" >&2
    exit 1
fi

if [[ ! -d "$CURL_SRC_DIR" ]]; then
    echo "ERROR: curl source directory not found: $CURL_SRC_DIR" >&2
    echo "Run: git submodule update --init --recursive" >&2
    exit 1
fi

if [[ ! -d "$OPENSSL_SRC_DIR" ]]; then
    echo "ERROR: OpenSSL source directory not found: $OPENSSL_SRC_DIR" >&2
    echo "Run: git submodule update --init --recursive" >&2
    exit 1
fi

if [[ ! -d "$LIBPSL_SRC_DIR" ]]; then
    echo "ERROR: libpsl source directory not found: $LIBPSL_SRC_DIR" >&2
    echo "Run: git submodule update --init --recursive" >&2
    exit 1
fi

ABI="${ABI:-arm64-v8a}"
API="${API:-21}"
JOBS="${JOBS:-$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)}"

case "$ABI" in
    arm64-v8a) TARGET="aarch64-linux-android"; OPENSSL_TARGET="android-arm64" ;;
    armeabi-v7a) TARGET="armv7a-linux-androideabi"; OPENSSL_TARGET="android-arm" ;;
    x86_64) TARGET="x86_64-linux-android"; OPENSSL_TARGET="android-x86_64" ;;
    x86) TARGET="i686-linux-android"; OPENSSL_TARGET="android-x86" ;;
    *) echo "ERROR: Unknown ABI: $ABI" >&2; exit 1 ;;
esac

case "$(uname -s)-$(uname -m)" in
    Linux-aarch64|Linux-arm64) HOST_TAGS="linux-arm64 linux-aarch64 linux-x86_64" ;;
    Linux-x86_64|Linux-amd64) HOST_TAGS="linux-x86_64 linux-arm64 linux-aarch64" ;;
    Darwin-arm64) HOST_TAGS="darwin-arm64 darwin-x86_64" ;;
    Darwin-x86_64) HOST_TAGS="darwin-x86_64 darwin-arm64" ;;
    *) HOST_TAGS="linux-x86_64 linux-arm64 linux-aarch64 darwin-arm64 darwin-x86_64" ;;
esac

TOOLCHAIN=""
for host_tag in $HOST_TAGS; do
    candidate="$NDK/toolchains/llvm/prebuilt/$host_tag"
    if [[ -d "$candidate" ]]; then
        TOOLCHAIN="$candidate"
        break
    fi
done

if [[ -z "$TOOLCHAIN" ]]; then
    echo "ERROR: LLVM toolchain not found in NDK: $NDK" >&2
    exit 1
fi

CC="$TOOLCHAIN/bin/${TARGET}${API}-clang"
AR="$TOOLCHAIN/bin/llvm-ar"
RANLIB="$TOOLCHAIN/bin/llvm-ranlib"
NM="$TOOLCHAIN/bin/llvm-nm"

if [[ ! -x "$CC" ]]; then
    echo "ERROR: C compiler not found: $CC" >&2
    exit 1
fi

OPENSSL_BUILD_DIR="$PROJECT_ROOT/obj/openssl-build"
OPENSSL_INSTALL_DIR="$PROJECT_ROOT/obj/openssl-install"
OPENSSL_LIB_DIR="$OPENSSL_INSTALL_DIR/lib"
LIBPSL_BUILD_DIR="$PROJECT_ROOT/obj/libpsl-build"
LIBPSL_INSTALL_DIR="$PROJECT_ROOT/obj/libpsl-install"
LIBPSL_LIB_DIR="$LIBPSL_INSTALL_DIR/lib"
CURL_BUILD_DIR="$PROJECT_ROOT/obj/curl-build"
INSTALL_DIR="$PROJECT_ROOT/obj/curl-install"

rm -rf "$OPENSSL_WORK_DIR" "$OPENSSL_BUILD_DIR" "$OPENSSL_INSTALL_DIR" \
    "$LIBPSL_WORK_DIR" "$LIBPSL_BUILD_DIR" "$LIBPSL_INSTALL_DIR" \
    "$CURL_WORK_DIR" "$CURL_BUILD_DIR" "$INSTALL_DIR"
mkdir -p "$OPENSSL_WORK_DIR" "$OPENSSL_BUILD_DIR" "$OPENSSL_INSTALL_DIR" \
    "$LIBPSL_WORK_DIR" "$LIBPSL_BUILD_DIR" "$LIBPSL_INSTALL_DIR" \
    "$CURL_WORK_DIR" "$CURL_BUILD_DIR" "$INSTALL_DIR"

echo "=> Preparing OpenSSL source..."
if git -C "$OPENSSL_SRC_DIR" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    git -C "$OPENSSL_SRC_DIR" archive --format=tar HEAD | tar -x -C "$OPENSSL_WORK_DIR"
else
    (cd "$OPENSSL_SRC_DIR" && tar --exclude=.git -cf - .) | tar -x -C "$OPENSSL_WORK_DIR"
fi

echo "=> Configuring OpenSSL for $ABI (API $API)"
cd "$OPENSSL_BUILD_DIR"
PATH="$TOOLCHAIN/bin:$PATH" \
ANDROID_NDK_ROOT="$NDK" \
perl "$OPENSSL_WORK_DIR/Configure" \
    "$OPENSSL_TARGET" \
    -D__ANDROID_API__="$API" \
    --prefix="$OPENSSL_INSTALL_DIR" \
    --openssldir="$OPENSSL_INSTALL_DIR/ssl" \
    no-tests \
    no-shared

echo "=> Building OpenSSL with $JOBS jobs..."
PATH="$TOOLCHAIN/bin:$PATH" make -C "$OPENSSL_BUILD_DIR" -j"$JOBS" build_sw 2>&1 | tail -10

echo "=> Installing OpenSSL into $OPENSSL_INSTALL_DIR..."
PATH="$TOOLCHAIN/bin:$PATH" make -C "$OPENSSL_BUILD_DIR" install_sw 2>&1 | tail -10

if [[ ! -f "$OPENSSL_LIB_DIR/libssl.a" || ! -f "$OPENSSL_LIB_DIR/libcrypto.a" ]]; then
    echo "ERROR: OpenSSL static libraries were not produced" >&2
    exit 1
fi

if [[ ! -f "$LIBPSL_SRC_DIR/list/public_suffix_list.dat" ]]; then
    echo "=> Initializing libpsl public suffix list submodule..."
    git -C "$LIBPSL_SRC_DIR" submodule update --init --recursive
fi

echo "=> Preparing libpsl source..."
(cd "$LIBPSL_SRC_DIR" && tar --exclude=.git -cf - .) | tar -x -C "$LIBPSL_WORK_DIR"

cd "$LIBPSL_WORK_DIR"
if [[ ! -f configure || ! -f build-aux/config.guess || ! -f build-aux/config.sub || ! -f build-aux/install-sh ]]; then
    echo "=> Generating libpsl configure script..."
    mkdir -p m4
    if command -v gtkdocize >/dev/null 2>&1; then
        gtkdocize >/dev/null
    else
        rm -f gtk-doc.make
        {
            echo "EXTRA_DIST ="
            echo "CLEANFILES ="
        } > gtk-doc.make
    fi
    autoreconf --install --force --symlink >/dev/null
fi

echo "=> Configuring libpsl for $ABI (API $API)"
cd "$LIBPSL_BUILD_DIR"
PKG_CONFIG_LIBDIR="" \
PKG_CONFIG_PATH="" \
"$LIBPSL_WORK_DIR/configure" \
    --host="$TARGET" \
    --prefix="$LIBPSL_INSTALL_DIR" \
    --with-sysroot="$TOOLCHAIN/sysroot" \
    --enable-static \
    --disable-shared \
    --disable-runtime \
    --disable-builtin \
    CC="$CC" \
    AR="$AR" \
    RANLIB="$RANLIB" \
    CFLAGS="-Oz -fvisibility=hidden -ffunction-sections -fdata-sections" \
    LDFLAGS="-Wl,--gc-sections" \
    2>&1 | tail -30

echo "=> Building libpsl with $JOBS jobs..."
make -C "$LIBPSL_BUILD_DIR" -j"$JOBS" 2>&1 | tail -10

echo "=> Installing libpsl into $LIBPSL_INSTALL_DIR..."
make -C "$LIBPSL_BUILD_DIR" install 2>&1 | tail -10

if [[ ! -f "$LIBPSL_LIB_DIR/libpsl.a" ]]; then
    echo "ERROR: libpsl static library was not produced" >&2
    exit 1
fi

echo "=> Preparing curl source..."
if git -C "$CURL_SRC_DIR" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    git -C "$CURL_SRC_DIR" archive --format=tar HEAD | tar -x -C "$CURL_WORK_DIR"
else
    (cd "$CURL_SRC_DIR" && tar --exclude=.git -cf - .) | tar -x -C "$CURL_WORK_DIR"
fi

cd "$CURL_WORK_DIR"
if [[ ! -f configure ]]; then
    echo "=> Generating curl configure script..."
    autoreconf -fi
fi

echo "=> Configuring curl for $ABI (API $API)"
echo "   NDK:       $NDK"
echo "   Toolchain: $TOOLCHAIN"
echo "   CC:        $CC"

cd "$CURL_BUILD_DIR"

PKG_CONFIG_LIBDIR="$OPENSSL_LIB_DIR/pkgconfig:$LIBPSL_LIB_DIR/pkgconfig" \
PKG_CONFIG_PATH="" \
"$CURL_WORK_DIR/configure" \
    --host="$TARGET" \
    --prefix="$INSTALL_DIR" \
    --with-sysroot="$TOOLCHAIN/sysroot" \
    --enable-static \
    --disable-shared \
    --with-openssl="$OPENSSL_INSTALL_DIR" \
    --with-libpsl="$LIBPSL_INSTALL_DIR" \
    CC="$CC" \
    AR="$AR" \
    RANLIB="$RANLIB" \
    CPPFLAGS="-I$OPENSSL_INSTALL_DIR/include -I$LIBPSL_INSTALL_DIR/include" \
    CFLAGS="-Oz -fvisibility=hidden -ffunction-sections -fdata-sections" \
    LDFLAGS="-L$OPENSSL_LIB_DIR -L$LIBPSL_LIB_DIR -Wl,--gc-sections" \
    2>&1 | tail -30

if ! grep -q "^#define USE_LIBPSL 1" "$CURL_BUILD_DIR/lib/curl_config.h"; then
    echo "ERROR: curl was not configured with libpsl support" >&2
    exit 1
fi

echo "=> Building curl with $JOBS jobs..."
make -C "$CURL_BUILD_DIR" -j"$JOBS" 2>&1 | tail -10

echo "=> Installing curl into $INSTALL_DIR..."
make -C "$CURL_BUILD_DIR" install 2>&1 | tail -10

if [[ ! -f "$INSTALL_DIR/lib/libcurl.a" ]]; then
    echo "ERROR: curl static library was not produced" >&2
    exit 1
fi

symbol_count="unknown"
if [[ -x "$NM" ]]; then
    symbol_count="$("$NM" "$INSTALL_DIR/lib/libcurl.a" 2>/dev/null | grep -c " T " || true)"
fi

echo "=> curl build complete"
echo "   OpenSSL libs:   $OPENSSL_LIB_DIR/"
echo "   libpsl:         $LIBPSL_LIB_DIR/libpsl.a"
echo "   Static library: $INSTALL_DIR/lib/libcurl.a"
echo "   Headers:        $INSTALL_DIR/include/curl/"
echo "   Size:           $(du -h "$INSTALL_DIR/lib/libcurl.a" | cut -f1)"
echo "   Text symbols:   $symbol_count"
