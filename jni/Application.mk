APP_ABI := arm64-v8a
APP_PLATFORM := android-21
APP_STL := c++_static
APP_OPTIM := release
APP_THIN_ARCHIVE := false
APP_PIE := true

APP_CFLAGS += \
    -fstack-protector-strong \
    -D_FORTIFY_SOURCE=2 \
    -fno-strict-aliasing \
    -fno-strict-overflow \
    -fno-delete-null-pointer-checks \
    -funwind-tables

APP_CPPFLAGS += \
    -fvisibility-inlines-hidden

APP_LDFLAGS += \
    -Wl,-z,relro \
    -Wl,-z,now \
    -Wl,--as-needed
