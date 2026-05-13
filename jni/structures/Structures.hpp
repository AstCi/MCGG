#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "../Il2CppVersions/headers/2019.4.22f1.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Unity {
    struct Color;
    struct Quaternion;
    struct Vector2;
    struct Vector3;
    struct Vector4;

    inline constexpr float Deg2Rad = static_cast<float>(M_PI) / 180.0f;
    inline constexpr float Rad2Deg = 180.0f / static_cast<float>(M_PI);

    inline float Clamp01(float value) {
        return std::clamp(value, 0.0f, 1.0f);
    }

    inline bool IsFinite(float value) {
        return std::isfinite(value);
    }

    struct Color {
        union {
            struct { float r, g, b, a; };
            float data[4];
        };

        constexpr Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
        constexpr Color(float r, float g, float b) : r(r), g(g), b(b), a(1.0f) {}
        constexpr Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
        explicit Color(Vector4 v);

        static Color HSVToRGB(float h, float s, float v, bool hdr = true);
        static Color Lerp(Color a, Color b, float t);

        Color RGBMultiplied(float multiplier) const { return {r * multiplier, g * multiplier, b * multiplier, a}; }
        Color RGBMultiplied(Color multiplier) const { return {r * multiplier.r, g * multiplier.g, b * multiplier.b, a}; }

        friend bool operator==(const Color& lhs, const Color& rhs) {
            return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
        }

        friend bool operator!=(const Color& lhs, const Color& rhs) {
            return !(lhs == rhs);
        }

        static const Color black;
        static const Color red;
        static const Color green;
        static const Color blue;
        static const Color white;
        static const Color orange;
        static const Color yellow;
        static const Color cyan;
        static const Color magenta;
    };

    struct Color32 {
        union {
            struct { uint8_t r, g, b, a; };
            uint32_t rgba;
        };

        constexpr Color32() : r(0), g(0), b(0), a(255) {}
        constexpr Color32(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}
    };

    struct Vector2 {
        union {
            struct { float x, y; };
            float data[2];
        };

        constexpr Vector2() : x(0.0f), y(0.0f) {}
        constexpr Vector2(float x, float y) : x(x), y(y) {}

        float* GetPtr() { return data; }
        const float* GetPtr() const { return data; }

        float& operator[](int index) { return data[index]; }
        const float& operator[](int index) const { return data[index]; }

        static float Angle(Vector2 from, Vector2 to);
        static Vector2 ClampMagnitude(Vector2 vector, float maxLength);
        static float Component(Vector2 a, Vector2 b) { return Dot(a, b) / Magnitude(b); }
        static float Distance(Vector2 a, Vector2 b) { return Magnitude(a - b); }
        static float Dot(Vector2 lhs, Vector2 rhs) { return lhs.x * rhs.x + lhs.y * rhs.y; }
        static Vector2 FromPolar(float radius, float theta);
        static Vector2 Lerp(Vector2 from, Vector2 to, float t) { return LerpUnclamped(from, to, Clamp01(t)); }
        static Vector2 LerpUnclamped(Vector2 from, Vector2 to, float t) { return (to - from) * t + from; }
        static float Magnitude(Vector2 vector) { return std::sqrt(SqrMagnitude(vector)); }
        static Vector2 Max(Vector2 lhs, Vector2 rhs);
        static Vector2 Min(Vector2 lhs, Vector2 rhs);
        static Vector2 MoveTowards(Vector2 current, Vector2 target, float maxDistanceDelta);
        static Vector2 Normalize(Vector2 vector);
        static void OrthoNormalize(Vector2& normal, Vector2& tangent);
        static Vector2 Perpendicular(Vector2 direction) { return {-direction.y, direction.x}; }
        static Vector2 Project(Vector2 a, Vector2 b);
        static Vector2 Reflect(Vector2 direction, Vector2 normal);
        static Vector2 Reject(Vector2 a, Vector2 b) { return a - Project(a, b); }
        static Vector2 RotateTowards(Vector2 current, Vector2 target, float maxRadiansDelta, float maxMagnitudeDelta);
        static Vector2 Scale(Vector2 a, Vector2 b) { return a * b; }
        static Vector2 Slerp(Vector2 a, Vector2 b, float t);
        static Vector2 SlerpUnclamped(Vector2 a, Vector2 b, float t);
        static Vector2 SmoothDamp(Vector2 current, Vector2 target, Vector2& currentVelocity, float smoothTime, float maxSpeed, float deltaTime);
        static float SqrMagnitude(Vector2 vector) { return vector.x * vector.x + vector.y * vector.y; }
        static void ToPolar(Vector2 vector, float& radius, float& theta);

        void Normalize() { *this = Normalize(*this); }
        void Scale(Vector2 scale) { *this = *this * scale; }
        float magnitude() const { return Magnitude(*this); }
        Vector2 normalized() const { return Normalize(*this); }
        float sqrMagnitude() const { return SqrMagnitude(*this); }

        bool operator==(Vector2 value) const { return x == value.x && y == value.y; }
        bool operator!=(Vector2 value) const { return !(*this == value); }
        Vector2& operator+=(float value) { x += value; y += value; return *this; }
        Vector2& operator-=(float value) { x -= value; y -= value; return *this; }
        Vector2& operator*=(float value) { x *= value; y *= value; return *this; }
        Vector2& operator/=(float value) { float inv = 1.0f / value; x *= inv; y *= inv; return *this; }
        Vector2& operator+=(Vector2 value) { x += value.x; y += value.y; return *this; }
        Vector2& operator-=(Vector2 value) { x -= value.x; y -= value.y; return *this; }
        operator Vector3() const;

        friend Vector2 operator+(Vector2 lhs, float rhs) { return lhs += rhs; }
        friend Vector2 operator-(Vector2 lhs, float rhs) { return lhs -= rhs; }
        friend Vector2 operator*(Vector2 lhs, float rhs) { return lhs *= rhs; }
        friend Vector2 operator/(Vector2 lhs, float rhs) { return lhs /= rhs; }
        friend Vector2 operator+(float lhs, Vector2 rhs) { return rhs + lhs; }
        friend Vector2 operator-(float lhs, Vector2 rhs) { return {lhs - rhs.x, lhs - rhs.y}; }
        friend Vector2 operator*(float lhs, Vector2 rhs) { return rhs * lhs; }
        friend Vector2 operator/(float lhs, Vector2 rhs) { return {lhs / rhs.x, lhs / rhs.y}; }
        friend Vector2 operator+(Vector2 lhs, Vector2 rhs) { return {lhs.x + rhs.x, lhs.y + rhs.y}; }
        friend Vector2 operator-(Vector2 lhs, Vector2 rhs) { return {lhs.x - rhs.x, lhs.y - rhs.y}; }
        friend Vector2 operator*(Vector2 lhs, Vector2 rhs) { return {lhs.x * rhs.x, lhs.y * rhs.y}; }
        friend Vector2 operator/(Vector2 lhs, Vector2 rhs) { return {lhs.x / rhs.x, lhs.y / rhs.y}; }
        Vector2 operator-() const { return {-x, -y}; }

        static const Vector2 positiveInfinity;
        static const Vector2 negativeInfinity;
        static const Vector2 down;
        static const Vector2 left;
        static const Vector2 one;
        static const Vector2 right;
        static const Vector2 up;
        static const Vector2 zero;
    };

    struct Vector2Int {
        union {
            struct { int x, y; };
            int data[2];
        };

        constexpr Vector2Int() : x(0), y(0) {}
        constexpr Vector2Int(int x, int y) : x(x), y(y) {}
    };

    struct Vector3 {
        union {
            struct { float x, y, z; };
            float data[3];
        };

        constexpr Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
        constexpr Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

        operator Vector4() const;
        operator Vector2() const { return {x, y}; }
        operator Vector2&() { return *reinterpret_cast<Vector2*>(this); }

        float* GetPtr() { return data; }
        const float* GetPtr() const { return data; }

        float& operator[](int index) { return data[index]; }
        const float& operator[](int index) const { return data[index]; }

        static float Angle(Vector3 from, Vector3 to);
        static float AngleBetween(Vector3 from, Vector3 to) { return Angle(Normalize(from), Normalize(to)); }
        static Vector3 ClampMagnitude(Vector3 vector, float maxLength);
        static Vector3 Cross(Vector3 lhs, Vector3 rhs);
        static float Component(Vector3 a, Vector3 b) { return Dot(a, b) / Magnitude(b); }
        static float Distance(Vector3 a, Vector3 b) { return Magnitude(a - b); }
        static float Dot(Vector3 lhs, Vector3 rhs) { return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z; }
        static Vector3 FromSpherical(float radius, float theta, float phi);
        static Vector3 FromString(const std::string& value);
        static Vector3 Lerp(Vector3 from, Vector3 to, float t) { return LerpUnclamped(from, to, Clamp01(t)); }
        static Vector3 LerpUnclamped(Vector3 from, Vector3 to, float t) { return (to - from) * t + from; }
        static float Magnitude(Vector3 vector) { return std::sqrt(SqrMagnitude(vector)); }
        static Vector3 Max(Vector3 lhs, Vector3 rhs);
        static Vector3 Min(Vector3 lhs, Vector3 rhs);
        static Vector3 MoveTowards(Vector3 current, Vector3 target, float maxDistanceDelta);
        static Vector3 Normalize(Vector3 vector);
        static float NormalizeAngle(float value, bool is180 = true);
        static Vector3 NormalizeEuler(Vector3 vector, bool is180 = true);
        static Vector3 Orthogonal(Vector3 vector);
        static void OrthoNormalize(Vector3& normal, Vector3& tangent);
        static void OrthoNormalize(Vector3& normal, Vector3& tangent, Vector3& binormal);
        static void OrthoNormalizeFast(Vector3& normal, Vector3& tangent, Vector3& binormal);
        static Vector3 OrthoNormalVectorFast(const Vector3& normal);
        static Vector3 Project(Vector3 vector, Vector3 onNormal);
        static Vector3 ProjectOnPlane(Vector3 vector, Vector3 planeNormal);
        static Vector3 Reflect(Vector3 direction, Vector3 normal);
        static Vector3 RotateTowards(Vector3 current, Vector3 target, float maxRadiansDelta, float maxMagnitudeDelta);
        static Vector3 Scale(Vector3 a, Vector3 b) { return a * b; }
        static float SignedAngle(Vector3 from, Vector3 to, Vector3 axis);
        static Vector3 Slerp(Vector3 a, Vector3 b, float t);
        static Vector3 SlerpUnclamped(Vector3 a, Vector3 b, float t);
        static Vector3 SmoothDamp(Vector3 current, Vector3 target, Vector3& currentVelocity, float smoothTime, float maxSpeed, float deltaTime);
        static float SqrMagnitude(Vector3 vector) { return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z; }
        static void ToSpherical(Vector3 vector, float& radius, float& theta, float& phi);

        void Normalize() { *this = Normalize(*this); }
        void Scale(Vector3 scale) { *this = *this * scale; }
        Vector3 orthogonal() const { return Orthogonal(*this); }
        float magnitude() const { return Magnitude(*this); }
        Vector3 normalized() const { return Normalize(*this); }
        Vector3 normalizedEuler(bool is180 = true) const { return NormalizeEuler(*this, is180); }
        float sqrMagnitude() const { return SqrMagnitude(*this); }

        bool operator==(Vector3 value) const { return x == value.x && y == value.y && z == value.z; }
        bool operator!=(Vector3 value) const { return !(*this == value); }
        Vector3& operator+=(Vector3 value) { x += value.x; y += value.y; z += value.z; return *this; }
        Vector3& operator-=(Vector3 value) { x -= value.x; y -= value.y; z -= value.z; return *this; }
        Vector3& operator*=(float value) { x *= value; y *= value; z *= value; return *this; }
        Vector3& operator/=(float value) { float inv = 1.0f / value; x *= inv; y *= inv; z *= inv; return *this; }
        Vector3& operator*=(int value) { return *this *= static_cast<float>(value); }
        Vector3& operator/=(int value) { return *this /= static_cast<float>(value); }
        Vector3& operator/=(Vector3 value) { x /= value.x; y /= value.y; z /= value.z; return *this; }

        friend Vector3 operator+(Vector3 lhs, Vector3 rhs) { return lhs += rhs; }
        friend Vector3 operator-(Vector3 lhs, Vector3 rhs) { return lhs -= rhs; }
        friend Vector3 operator*(Vector3 lhs, float rhs) { return lhs *= rhs; }
        friend Vector3 operator*(Vector3 lhs, int rhs) { return lhs *= rhs; }
        friend Vector3 operator*(float lhs, Vector3 rhs) { return rhs * lhs; }
        friend Vector3 operator*(int lhs, Vector3 rhs) { return rhs * lhs; }
        friend Vector3 operator*(Vector3 lhs, Vector3 rhs) { return {lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z}; }
        friend Vector3 operator/(Vector3 lhs, float rhs) { return lhs /= rhs; }
        friend Vector3 operator/(Vector3 lhs, int rhs) { return lhs /= rhs; }
        friend Vector3 operator/(Vector3 lhs, Vector3 rhs) { return lhs /= rhs; }
        Vector3 operator-() const { return {-x, -y, -z}; }

        static constexpr float kEpsilon = 1E-05f;
        static constexpr float kEpsilonNormalSqrt = 1E-15f;
        static const Vector3 positiveInfinity;
        static const Vector3 negativeInfinity;
        static const Vector3 back;
        static const Vector3 down;
        static const Vector3 forward;
        static const Vector3 left;
        static const Vector3 one;
        static const Vector3 right;
        static const Vector3 up;
        static const Vector3 zero;
    };

    struct Vector3Int {
        union {
            struct { int x, y, z; };
            int data[3];
        };

        constexpr Vector3Int() : x(0), y(0), z(0) {}
        constexpr Vector3Int(int x, int y, int z) : x(x), y(y), z(z) {}
    };

    struct Vector4 {
        union {
            struct { float x, y, z, w; };
            float data[4];
        };

        constexpr Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
        constexpr Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
        constexpr Vector4(Vector3 value, float w) : x(value.x), y(value.y), z(value.z), w(w) {}
        explicit Vector4(Color value);

        float* GetPtr() { return data; }
        const float* GetPtr() const { return data; }

        float& operator[](int index) { return data[index]; }
        const float& operator[](int index) const { return data[index]; }

        static bool CompareApproximately(Vector4 a, Vector4 b, float maxDistance = Vector3::kEpsilon) {
            return SqrMagnitude(b - a) <= maxDistance * maxDistance;
        }

        static float Component(Vector4 a, Vector4 b) { return Dot(a, b) / Magnitude(b); }
        static float Distance(Vector4 a, Vector4 b) { return Magnitude(a - b); }
        static float Dot(Vector4 lhs, Vector4 rhs) { return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w; }
        static bool IsFinite(Vector4 value) { return Unity::IsFinite(value.x) && Unity::IsFinite(value.y) && Unity::IsFinite(value.z) && Unity::IsFinite(value.w); }
        static Vector4 Lerp(Vector4 from, Vector4 to, float t) { return LerpUnclamped(from, to, Clamp01(t)); }
        static Vector4 LerpUnclamped(Vector4 from, Vector4 to, float t) { return (to - from) * t + from; }
        static float Magnitude(Vector4 value) { return std::sqrt(Dot(value, value)); }
        static Vector4 Normalize(Vector4 value);
        static Vector4 Project(Vector4 a, Vector4 b) { return b * (Dot(a, b) / Dot(b, b)); }
        static float SqrMagnitude(Vector4 value) { return Dot(value, value); }

        void Normalize() { *this = Normalize(*this); }
        bool operator==(Vector4 value) const { return x == value.x && y == value.y && z == value.z && w == value.w; }
        bool operator!=(Vector4 value) const { return !(*this == value); }
        operator Vector3() const { return {x, y, z}; }

        friend Vector4 operator*(Vector4 lhs, Vector4 rhs) { return {lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w}; }
        friend Vector4 operator*(Vector4 lhs, float rhs) { return {lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs}; }
        friend Vector4 operator+(Vector4 lhs, Vector4 rhs) { return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w}; }
        friend Vector4 operator-(Vector4 lhs, Vector4 rhs) { return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w}; }
        friend Vector4 operator/(Vector4 lhs, float rhs) { float inv = 1.0f / rhs; return {lhs.x * inv, lhs.y * inv, lhs.z * inv, lhs.w * inv}; }
        friend Vector4 operator/(Vector4 lhs, Vector4 rhs) { return {lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w}; }
        Vector4 operator-() const { return {-x, -y, -z, -w}; }

        static const Vector4 positiveInfinity;
        static const Vector4 negativeInfinity;
        static const Vector4 zero;
        static const Vector4 one;
    };

    struct Quaternion {
        union {
            struct { float x, y, z, w; };
            float data[4];
        };

        constexpr Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
        explicit Quaternion(const float values[]) : x(values[0]), y(values[1]), z(values[2]), w(values[3]) {}
        constexpr Quaternion(Vector3 vector, float scalar) : x(vector.x), y(vector.y), z(vector.z), w(scalar) {}
        constexpr Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
        Quaternion(float yaw, float pitch, float roll) { *this = FromEuler(yaw, pitch, roll); }

        static Vector3 Up(Quaternion value) { return value * Vector3::up; }
        static Vector3 Down(Quaternion value) { return value * Vector3::down; }
        static Vector3 Left(Quaternion value) { return value * Vector3::left; }
        static Vector3 Right(Quaternion value) { return value * Vector3::right; }
        static Vector3 Forward(Quaternion value) { return value * Vector3::forward; }
        static Vector3 Back(Quaternion value) { return value * Vector3::back; }
        static float Angle(Quaternion a, Quaternion b);
        static Quaternion Conjugate(Quaternion rotation) { return {-rotation.x, -rotation.y, -rotation.z, rotation.w}; }
        static float Dot(Quaternion lhs, Quaternion rhs) { return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w; }
        static Quaternion FromAngleAxis(float angle, Vector3 axis);
        static Quaternion FromEuler(Vector3 rotation) { return FromEuler(rotation.x, rotation.y, rotation.z); }
        static Quaternion FromEuler(float yaw, float pitch, float roll, bool fromDeg = true);
        static Quaternion FromToRotation(Vector3 fromVector, Vector3 toVector);
        static Quaternion Inverse(Quaternion rotation);
        static Quaternion Lerp(Quaternion a, Quaternion b, float t);
        static Quaternion LerpUnclamped(Quaternion a, Quaternion b, float t);
        static Quaternion LookRotation(Vector3 forward) { return LookRotation(forward, Vector3::up); }
        static Quaternion LookRotation(Vector3 forward, Vector3 upwards);
        static float Norm(Quaternion rotation) { return std::sqrt(Dot(rotation, rotation)); }
        static Quaternion Normalize(Quaternion rotation);
        static Quaternion RotateTowards(Quaternion from, Quaternion to, float maxRadiansDelta);
        static Quaternion Slerp(Quaternion a, Quaternion b, float t);
        static Quaternion SlerpUnclamped(Quaternion a, Quaternion b, float t);
        static void ToAngleAxis(Quaternion rotation, float& angle, Vector3& axis);
        static Vector3 ToEuler(Quaternion value, bool toDeg = true);
        static Vector3 RotateVectorByQuaternion(Quaternion lhs, Vector3 rhs);

        Vector3 eulerAngles() const { return ToEuler(*this); }
        Quaternion normalized() const { return Normalize(*this); }

        Quaternion& operator+=(Quaternion value) { x += value.x; y += value.y; z += value.z; w += value.w; return *this; }
        Quaternion& operator-=(Quaternion value) { x -= value.x; y -= value.y; z -= value.z; w -= value.w; return *this; }
        Quaternion& operator*=(Quaternion value);
        Quaternion& operator*=(float value) { x *= value; y *= value; z *= value; w *= value; return *this; }
        Quaternion& operator/=(Quaternion value);
        Quaternion& operator/=(float value) { x /= value; y /= value; z /= value; w /= value; return *this; }
        Quaternion operator-() const { return {-x, -y, -z, -w}; }
        Quaternion operator*(float value) const { return {x * value, y * value, z * value, w * value}; }

        friend Quaternion operator+(Quaternion lhs, Quaternion rhs) { return lhs += rhs; }
        friend Quaternion operator-(Quaternion lhs, Quaternion rhs) { return lhs -= rhs; }
        friend Quaternion operator*(Quaternion lhs, Quaternion rhs) { return lhs *= rhs; }
        friend Quaternion operator*(float lhs, Quaternion rhs) { return rhs *= lhs; }
        friend Quaternion operator/(Quaternion lhs, Quaternion rhs) { return lhs /= rhs; }
        friend Quaternion operator/(Quaternion lhs, float rhs) { return lhs /= rhs; }
        friend Vector3 operator*(Quaternion lhs, Vector3 rhs) { return RotateVectorByQuaternion(lhs, rhs); }

        static const Quaternion identity;
    };

    inline const Color Color::black{0.0f, 0.0f, 0.0f, 1.0f};
    inline const Color Color::red{1.0f, 0.0f, 0.0f, 1.0f};
    inline const Color Color::green{0.0f, 1.0f, 0.0f, 1.0f};
    inline const Color Color::blue{0.0f, 0.0f, 1.0f, 1.0f};
    inline const Color Color::white{1.0f, 1.0f, 1.0f, 1.0f};
    inline const Color Color::orange{1.0f, 0.5f, 0.0f, 1.0f};
    inline const Color Color::yellow{1.0f, 0.92156863f, 0.01568628f, 1.0f};
    inline const Color Color::cyan{0.0f, 1.0f, 1.0f, 1.0f};
    inline const Color Color::magenta{1.0f, 0.0f, 1.0f, 1.0f};

    inline const Vector2 Vector2::positiveInfinity{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()};
    inline const Vector2 Vector2::negativeInfinity{-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()};
    inline const Vector2 Vector2::down{0.0f, -1.0f};
    inline const Vector2 Vector2::left{-1.0f, 0.0f};
    inline const Vector2 Vector2::one{1.0f, 1.0f};
    inline const Vector2 Vector2::right{1.0f, 0.0f};
    inline const Vector2 Vector2::up{0.0f, 1.0f};
    inline const Vector2 Vector2::zero{0.0f, 0.0f};

    inline const Vector3 Vector3::positiveInfinity{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()};
    inline const Vector3 Vector3::negativeInfinity{-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()};
    inline const Vector3 Vector3::back{0.0f, 0.0f, -1.0f};
    inline const Vector3 Vector3::down{0.0f, -1.0f, 0.0f};
    inline const Vector3 Vector3::forward{0.0f, 0.0f, 1.0f};
    inline const Vector3 Vector3::left{-1.0f, 0.0f, 0.0f};
    inline const Vector3 Vector3::one{1.0f, 1.0f, 1.0f};
    inline const Vector3 Vector3::right{1.0f, 0.0f, 0.0f};
    inline const Vector3 Vector3::up{0.0f, 1.0f, 0.0f};
    inline const Vector3 Vector3::zero{0.0f, 0.0f, 0.0f};

    inline const Vector4 Vector4::positiveInfinity{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()};
    inline const Vector4 Vector4::negativeInfinity{-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()};
    inline const Vector4 Vector4::zero{0.0f, 0.0f, 0.0f, 0.0f};
    inline const Vector4 Vector4::one{1.0f, 1.0f, 1.0f, 1.0f};

    inline const Quaternion Quaternion::identity{0.0f, 0.0f, 0.0f, 1.0f};

    inline Color::Color(Vector4 value) : r(value.x), g(value.y), b(value.z), a(value.w) {}

    inline Color Color::HSVToRGB(float h, float s, float v, bool hdr) {
        if (s == 0.0f) return {v, v, v};
        if (v == 0.0f) return {0.0f, 0.0f, 0.0f};

        float sector = h * 6.0f;
        int index = static_cast<int>(std::floor(sector));
        float fraction = sector - static_cast<float>(index);
        float p = v * (1.0f - s);
        float q = v * (1.0f - s * fraction);
        float t = v * (1.0f - s * (1.0f - fraction));

        Color result{};
        switch (index) {
            case -1: result = {v, p, q}; break;
            case 0:
            case 6: result = {v, t, p}; break;
            case 1: result = {q, v, p}; break;
            case 2: result = {p, v, t}; break;
            case 3: result = {p, q, v}; break;
            case 4: result = {t, p, v}; break;
            case 5: result = {v, p, q}; break;
            default: result = {0.0f, 0.0f, 0.0f}; break;
        }

        if (hdr) return result;
        return {Clamp01(result.r), Clamp01(result.g), Clamp01(result.b), result.a};
    }

    inline Color Color::Lerp(Color a, Color b, float t) {
        t = Clamp01(t);
        return {
            a.r + (b.r - a.r) * t,
            a.g + (b.g - a.g) * t,
            a.b + (b.b - a.b) * t,
            a.a + (b.a - a.a) * t
        };
    }

    inline float Vector2::Angle(Vector2 from, Vector2 to) {
        float denominatorSq = SqrMagnitude(from) * SqrMagnitude(to);
        if (denominatorSq == 0.0f) return 0.0f;
        return std::acos(std::clamp(Dot(from, to) * (1.0f / std::sqrt(denominatorSq)), -1.0f, 1.0f));
    }

    inline Vector2 Vector2::ClampMagnitude(Vector2 vector, float maxLength) {
        float length = Magnitude(vector);
        return length > maxLength ? vector * (maxLength / length) : vector;
    }

    inline Vector2 Vector2::FromPolar(float radius, float theta) {
        return {radius * std::cos(theta), radius * std::sin(theta)};
    }

    inline Vector2 Vector2::Max(Vector2 lhs, Vector2 rhs) {
        return {std::max(lhs.x, rhs.x), std::max(lhs.y, rhs.y)};
    }

    inline Vector2 Vector2::Min(Vector2 lhs, Vector2 rhs) {
        return {std::min(lhs.x, rhs.x), std::min(lhs.y, rhs.y)};
    }

    inline Vector2 Vector2::MoveTowards(Vector2 current, Vector2 target, float maxDistanceDelta) {
        Vector2 delta = target - current;
        float distance = Magnitude(delta);
        if (distance <= maxDistanceDelta || distance == 0.0f) return target;
        return current + delta * (maxDistanceDelta / distance);
    }

    inline Vector2 Vector2::Normalize(Vector2 vector) {
        float magnitude = Magnitude(vector);
        return magnitude == 0.0f ? zero : vector * (1.0f / magnitude);
    }

    inline void Vector2::OrthoNormalize(Vector2& normal, Vector2& tangent) {
        normal = Normalize(normal);
        tangent = Normalize(Reject(tangent, normal));
    }

    inline Vector2 Vector2::Project(Vector2 a, Vector2 b) {
        float sqrMagnitude = SqrMagnitude(b);
        return sqrMagnitude == 0.0f ? zero : b * (Dot(a, b) / sqrMagnitude);
    }

    inline Vector2 Vector2::Reflect(Vector2 direction, Vector2 normal) {
        return direction - 2.0f * Project(direction, normal);
    }

    inline Vector2 Vector2::RotateTowards(Vector2 current, Vector2 target, float maxRadiansDelta, float maxMagnitudeDelta) {
        float currentMagnitude = Magnitude(current);
        float targetMagnitude = Magnitude(target);
        float newMagnitude = std::clamp(currentMagnitude + maxMagnitudeDelta * static_cast<float>((targetMagnitude > currentMagnitude) - (currentMagnitude > targetMagnitude)),
                                        std::min(currentMagnitude, targetMagnitude),
                                        std::max(currentMagnitude, targetMagnitude));
        float totalAngle = Angle(current, target) - maxRadiansDelta;
        if (totalAngle <= 0.0f) return Normalize(target) * newMagnitude;
        if (totalAngle >= static_cast<float>(M_PI)) return -Normalize(target) * newMagnitude;

        float axis = current.x * target.y - current.y * target.x;
        axis = std::fabs(axis) > 0.00001f ? axis / std::fabs(axis) : 1.0f;
        current = Normalize(current);
        return (current * std::cos(maxRadiansDelta) + Perpendicular(current) * std::sin(maxRadiansDelta) * axis) * newMagnitude;
    }

    inline Vector2 Vector2::Slerp(Vector2 a, Vector2 b, float t) {
        if (t < 0.0f) return a;
        if (t > 1.0f) return b;
        return SlerpUnclamped(a, b, t);
    }

    inline Vector2 Vector2::SlerpUnclamped(Vector2 a, Vector2 b, float t) {
        float magA = Magnitude(a);
        float magB = Magnitude(b);
        if (magA == 0.0f || magB == 0.0f) return LerpUnclamped(a, b, t);
        a /= magA;
        b /= magB;
        float dot = std::clamp(Dot(a, b), -1.0f, 1.0f);
        float theta = std::acos(dot) * t;
        Vector2 relative = Normalize(b - a * dot);
        return (a * std::cos(theta) + relative * std::sin(theta)) * (magA + (magB - magA) * t);
    }

    inline Vector2 Vector2::SmoothDamp(Vector2 current, Vector2 target, Vector2& currentVelocity, float smoothTime, float maxSpeed, float deltaTime) {
        smoothTime = std::max(0.0001f, smoothTime);
        float omega = 2.0f / smoothTime;
        float x = omega * deltaTime;
        float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
        Vector2 change = current - target;
        Vector2 originalTarget = target;
        float maxChange = maxSpeed * smoothTime;
        float maxChangeSq = maxChange * maxChange;
        if (SqrMagnitude(change) > maxChangeSq) change = Normalize(change) * maxChange;
        target = current - change;
        Vector2 temp = (currentVelocity + omega * change) * deltaTime;
        currentVelocity = (currentVelocity - omega * temp) * exp;
        Vector2 output = target + (change + temp) * exp;
        if (Dot(originalTarget - current, output - originalTarget) > 0.0f) {
            currentVelocity = (output - originalTarget) / deltaTime;
            return originalTarget;
        }
        return output;
    }

    inline void Vector2::ToPolar(Vector2 vector, float& radius, float& theta) {
        radius = Magnitude(vector);
        theta = std::atan2(vector.y, vector.x);
    }

    inline Vector2::operator Vector3() const {
        return {x, y, 0.0f};
    }

    inline Vector3::operator Vector4() const {
        return {x, y, z, 0.0f};
    }

    inline Vector3 Vector3::Cross(Vector3 lhs, Vector3 rhs) {
        return {
            lhs.y * rhs.z - lhs.z * rhs.y,
            lhs.z * rhs.x - lhs.x * rhs.z,
            lhs.x * rhs.y - lhs.y * rhs.x
        };
    }

    inline float Vector3::Angle(Vector3 from, Vector3 to) {
        float denominatorSq = SqrMagnitude(from) * SqrMagnitude(to);
        if (denominatorSq == 0.0f) return 0.0f;
        return std::acos(std::clamp(Dot(from, to) * (1.0f / std::sqrt(denominatorSq)), -1.0f, 1.0f));
    }

    inline Vector3 Vector3::ClampMagnitude(Vector3 vector, float maxLength) {
        float length = Magnitude(vector);
        return length > maxLength ? vector * (maxLength / length) : vector;
    }

    inline Vector3 Vector3::FromSpherical(float radius, float theta, float phi) {
        return {
            radius * std::sin(theta) * std::cos(phi),
            radius * std::sin(theta) * std::sin(phi),
            radius * std::cos(theta)
        };
    }

    inline Vector3 Vector3::FromString(const std::string& value) {
        std::vector<float> values;
        std::stringstream stream(value);
        std::string part;

        while (std::getline(stream, part, ',')) {
            if (!part.empty() && part.back() == 'f') part.pop_back();
            if (!part.empty()) values.push_back(std::stof(part));
        }

        return values.size() == 3 ? Vector3(values[0], values[1], values[2]) : zero;
    }

    inline Vector3 Vector3::Max(Vector3 lhs, Vector3 rhs) {
        return {std::max(lhs.x, rhs.x), std::max(lhs.y, rhs.y), std::max(lhs.z, rhs.z)};
    }

    inline Vector3 Vector3::Min(Vector3 lhs, Vector3 rhs) {
        return {std::min(lhs.x, rhs.x), std::min(lhs.y, rhs.y), std::min(lhs.z, rhs.z)};
    }

    inline Vector3 Vector3::MoveTowards(Vector3 current, Vector3 target, float maxDistanceDelta) {
        Vector3 delta = target - current;
        float distance = Magnitude(delta);
        if (distance <= maxDistanceDelta || distance == 0.0f) return target;
        return current + delta * (maxDistanceDelta / distance);
    }

    inline Vector3 Vector3::Normalize(Vector3 vector) {
        float magnitude = Magnitude(vector);
        return magnitude == 0.0f ? zero : vector * (1.0f / magnitude);
    }

    inline float Vector3::NormalizeAngle(float value, bool is180) {
        float res = std::fmod(value, 360.0f);
        if (res > 180.0f) res -= 360.0f;
        else if (res < -180.0f) res += 360.0f;

        if (is180) return res;
        return res < 0.0f ? res + 360.0f : res;
    }

    inline Vector3 Vector3::NormalizeEuler(Vector3 vector, bool is180) {
        return {
            NormalizeAngle(vector.x, is180),
            NormalizeAngle(vector.y, is180),
            NormalizeAngle(vector.z, is180)
        };
    }

    inline Vector3 Vector3::Orthogonal(Vector3 vector) {
        return vector.z < vector.x ? Vector3(vector.y, -vector.x, 0.0f) : Vector3(0.0f, -vector.z, vector.y);
    }

    inline void Vector3::OrthoNormalize(Vector3& normal, Vector3& tangent) {
        Vector3 n = normal;
        Vector3 t = tangent;

        float magN = Magnitude(n);
        n = magN > kEpsilon ? n * (1.0f / magN) : right;

        t -= Dot(n, t) * n;
        float magT = Magnitude(t);
        t = magT > kEpsilon ? t * (1.0f / magT) : OrthoNormalVectorFast(n);

        normal = n;
        tangent = t;
    }

    inline void Vector3::OrthoNormalize(Vector3& normal, Vector3& tangent, Vector3& binormal) {
        Vector3 n = normal;
        Vector3 t = tangent;
        Vector3 b = binormal;

        float magN = Magnitude(n);
        n = magN > kEpsilon ? n * (1.0f / magN) : right;

        t -= Dot(n, t) * n;
        float magT = Magnitude(t);
        t = magT > kEpsilon ? t * (1.0f / magT) : OrthoNormalVectorFast(n);

        b -= Dot(n, b) * n + Dot(t, b) * t;
        float magB = Magnitude(b);
        b = magB > kEpsilon ? b * (1.0f / magB) : Cross(n, t);

        normal = n;
        tangent = t;
        binormal = b;
    }

    inline void Vector3::OrthoNormalizeFast(Vector3& normal, Vector3& tangent, Vector3& binormal) {
        Vector3 n = Normalize(normal);
        Vector3 t = Normalize(tangent - Dot(n, tangent) * n);
        Vector3 b = Normalize(binormal - Dot(n, binormal) * n - Dot(t, binormal) * t);

        normal = n;
        tangent = t;
        binormal = b;
    }

    inline Vector3 Vector3::OrthoNormalVectorFast(const Vector3& normal) {
        constexpr float oneOverSqrt2 = 0.7071067811865475244f;
        if (std::fabs(normal.z) > oneOverSqrt2) {
            float scale = 1.0f / std::sqrt(normal.y * normal.y + normal.z * normal.z);
            return {0.0f, -normal.z * scale, normal.y * scale};
        }

        float scale = 1.0f / std::sqrt(normal.x * normal.x + normal.y * normal.y);
        return {-normal.y * scale, normal.x * scale, 0.0f};
    }

    inline Vector3 Vector3::Project(Vector3 vector, Vector3 onNormal) {
        float sqrMagnitude = Dot(onNormal, onNormal);
        if (sqrMagnitude < kEpsilon) return zero;
        return onNormal * (Dot(vector, onNormal) / sqrMagnitude);
    }

    inline Vector3 Vector3::ProjectOnPlane(Vector3 vector, Vector3 planeNormal) {
        float sqrMagnitude = Dot(planeNormal, planeNormal);
        if (sqrMagnitude < kEpsilon) return vector;
        return vector - planeNormal * (Dot(vector, planeNormal) / sqrMagnitude);
    }

    inline Vector3 Vector3::Reflect(Vector3 direction, Vector3 normal) {
        return direction - 2.0f * Dot(normal, direction) * normal;
    }

    inline Vector3 Vector3::RotateTowards(Vector3 current, Vector3 target, float maxRadiansDelta, float maxMagnitudeDelta) {
        float currentMagnitude = Magnitude(current);
        float targetMagnitude = Magnitude(target);
        float newMagnitude = std::clamp(currentMagnitude + maxMagnitudeDelta * static_cast<float>((targetMagnitude > currentMagnitude) - (currentMagnitude > targetMagnitude)),
                                        std::min(currentMagnitude, targetMagnitude),
                                        std::max(currentMagnitude, targetMagnitude));
        float totalAngle = Angle(current, target) - maxRadiansDelta;
        if (totalAngle <= 0.0f) return Normalize(target) * newMagnitude;
        if (totalAngle >= static_cast<float>(M_PI)) return -Normalize(target) * newMagnitude;

        Vector3 axis = Cross(current, target);
        float axisMagnitude = Magnitude(axis);
        axis = axisMagnitude == 0.0f ? Normalize(Cross(current, current + Vector3(3.95f, 5.32f, -4.24f))) : axis / axisMagnitude;
        current = Normalize(current);
        return (current * std::cos(maxRadiansDelta) + Cross(axis, current) * std::sin(maxRadiansDelta)) * newMagnitude;
    }

    inline float Vector3::SignedAngle(Vector3 from, Vector3 to, Vector3 axis) {
        float sign = Dot(axis, Cross(from, to)) >= 0.0f ? 1.0f : -1.0f;
        return Angle(from, to) * sign;
    }

    inline Vector3 Vector3::Slerp(Vector3 a, Vector3 b, float t) {
        if (t < 0.0f) return a;
        if (t > 1.0f) return b;
        return SlerpUnclamped(a, b, t);
    }

    inline Vector3 Vector3::SlerpUnclamped(Vector3 a, Vector3 b, float t) {
        float magA = Magnitude(a);
        float magB = Magnitude(b);
        if (magA == 0.0f || magB == 0.0f) return LerpUnclamped(a, b, t);
        a /= magA;
        b /= magB;
        float dot = std::clamp(Dot(a, b), -1.0f, 1.0f);
        float theta = std::acos(dot) * t;
        Vector3 relative = Normalize(b - a * dot);
        return (a * std::cos(theta) + relative * std::sin(theta)) * (magA + (magB - magA) * t);
    }

    inline Vector3 Vector3::SmoothDamp(Vector3 current, Vector3 target, Vector3& currentVelocity, float smoothTime, float maxSpeed, float deltaTime) {
        smoothTime = std::max(0.0001f, smoothTime);
        float omega = 2.0f / smoothTime;
        float x = omega * deltaTime;
        float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
        Vector3 change = current - target;
        Vector3 originalTarget = target;
        float maxChange = maxSpeed * smoothTime;
        float maxChangeSq = maxChange * maxChange;
        if (SqrMagnitude(change) > maxChangeSq) change = Normalize(change) * maxChange;
        target = current - change;
        Vector3 temp = (currentVelocity + omega * change) * deltaTime;
        currentVelocity = (currentVelocity - omega * temp) * exp;
        Vector3 output = target + (change + temp) * exp;
        if (Dot(originalTarget - current, output - originalTarget) > 0.0f) {
            currentVelocity = (output - originalTarget) / deltaTime;
            return originalTarget;
        }
        return output;
    }

    inline void Vector3::ToSpherical(Vector3 vector, float& radius, float& theta, float& phi) {
        radius = Magnitude(vector);
        if (radius == 0.0f) {
            theta = 0.0f;
            phi = 0.0f;
            return;
        }
        theta = std::acos(std::clamp(vector.z / radius, -1.0f, 1.0f));
        phi = std::atan2(vector.y, vector.x);
    }


    inline Vector4::Vector4(Color value) : x(value.r), y(value.g), z(value.b), w(value.a) {}

    inline Vector4 Vector4::Normalize(Vector4 value) {
        float magnitude = Magnitude(value);
        return magnitude > Vector3::kEpsilon ? value * (1.0f / magnitude) : zero;
    }

    inline Quaternion& Quaternion::operator*=(Quaternion value) {
        float newX = w * value.x + x * value.w + y * value.z - z * value.y;
        float newY = w * value.y + y * value.w + z * value.x - x * value.z;
        float newZ = w * value.z + z * value.w + x * value.y - y * value.x;
        float newW = w * value.w - x * value.x - y * value.y - z * value.z;
        x = newX;
        y = newY;
        z = newZ;
        w = newW;
        return *this;
    }

    inline Quaternion& Quaternion::operator/=(Quaternion value) {
        float newX = w / value.x + x / value.w + y / value.z - z / value.y;
        float newY = w / value.y + y / value.w + z / value.x - x / value.z;
        float newZ = w / value.z + z / value.w + x / value.y - y / value.x;
        float newW = w / value.w - x / value.x - y / value.y - z / value.z;
        x = newX;
        y = newY;
        z = newZ;
        w = newW;
        return *this;
    }

    inline float Quaternion::Angle(Quaternion a, Quaternion b) {
        return std::acos(std::min(std::fabs(Dot(a, b)), 1.0f)) * 2.0f;
    }

    inline Quaternion Quaternion::FromAngleAxis(float angle, Vector3 axis) {
        float magnitude = Vector3::Magnitude(axis);
        if (magnitude == 0.0f) return identity;
        float scale = std::sin(angle * 0.5f) / magnitude;
        return {axis.x * scale, axis.y * scale, axis.z * scale, std::cos(angle * 0.5f)};
    }

    inline Quaternion Quaternion::FromEuler(float yaw, float pitch, float roll, bool fromDeg) {
        if (fromDeg) {
            yaw *= Deg2Rad;
            pitch *= Deg2Rad;
            roll *= Deg2Rad;
        }

        float cy = std::cos(yaw * 0.5f);
        float sy = std::sin(yaw * 0.5f);
        float cp = std::cos(pitch * 0.5f);
        float sp = std::sin(pitch * 0.5f);
        float cr = std::cos(roll * 0.5f);
        float sr = std::sin(roll * 0.5f);

        return {
            cp * sy * cr + sp * cy * sr,
            sp * cy * cr - cp * sy * sr,
            cp * cy * sr - sp * sy * cr,
            cp * cy * cr + sp * sy * sr
        };
    }

    inline Quaternion Quaternion::FromToRotation(Vector3 fromVector, Vector3 toVector) {
        float dot = Vector3::Dot(fromVector, toVector);
        float magnitude = std::sqrt(Vector3::SqrMagnitude(fromVector) * Vector3::SqrMagnitude(toVector));
        if (magnitude == 0.0f) return identity;
        if (std::fabs(dot / magnitude + 1.0f) < 0.00001f) {
            return {Vector3::Normalize(Vector3::Orthogonal(fromVector)), 0.0f};
        }
        return Normalize(Quaternion(Vector3::Cross(fromVector, toVector), dot + magnitude));
    }

    inline Quaternion Quaternion::Inverse(Quaternion rotation) {
        float norm = Norm(rotation);
        return norm == 0.0f ? identity : Conjugate(rotation) / (norm * norm);
    }

    inline Quaternion Quaternion::Lerp(Quaternion a, Quaternion b, float t) {
        if (t < 0.0f) return Normalize(a);
        if (t > 1.0f) return Normalize(b);
        return LerpUnclamped(a, b, t);
    }

    inline Quaternion Quaternion::LerpUnclamped(Quaternion a, Quaternion b, float t) {
        Quaternion result = Dot(a, b) >= 0.0f ? a * (1.0f - t) + b * t : a * (1.0f - t) - b * t;
        return Normalize(result);
    }

    inline Quaternion Quaternion::LookRotation(Vector3 forward, Vector3 upwards) {
        forward = Vector3::Normalize(forward);
        upwards = Vector3::Normalize(upwards);
        constexpr float small = 0.0000000001f;
        if (Vector3::SqrMagnitude(forward) < small || Vector3::SqrMagnitude(upwards) < small) return identity;
        if (1.0f - std::fabs(Vector3::Dot(forward, upwards)) < small) return FromToRotation(Vector3::forward, forward);

        Vector3 right = Vector3::Normalize(Vector3::Cross(upwards, forward));
        upwards = Vector3::Cross(forward, right);
        Quaternion result{};
        float trace = right.x + upwards.y + forward.z;

        if (trace > 0.0f) {
            result.w = std::sqrt(1.0f + trace) * 0.5f;
            float reciprocal = 1.0f / (4.0f * result.w);
            result.x = (upwards.z - forward.y) * reciprocal;
            result.y = (forward.x - right.z) * reciprocal;
            result.z = (right.y - upwards.x) * reciprocal;
        } else if (right.x >= upwards.y && right.x >= forward.z) {
            result.x = std::sqrt(1.0f + right.x - upwards.y - forward.z) * 0.5f;
            float reciprocal = 1.0f / (4.0f * result.x);
            result.w = (upwards.z - forward.y) * reciprocal;
            result.z = (forward.x + right.z) * reciprocal;
            result.y = (right.y + upwards.x) * reciprocal;
        } else if (upwards.y > forward.z) {
            result.y = std::sqrt(1.0f - right.x + upwards.y - forward.z) * 0.5f;
            float reciprocal = 1.0f / (4.0f * result.y);
            result.z = (upwards.z + forward.y) * reciprocal;
            result.w = (forward.x - right.z) * reciprocal;
            result.x = (right.y + upwards.x) * reciprocal;
        } else {
            result.z = std::sqrt(1.0f - right.x - upwards.y + forward.z) * 0.5f;
            float reciprocal = 1.0f / (4.0f * result.z);
            result.y = (upwards.z + forward.y) * reciprocal;
            result.x = (forward.x + right.z) * reciprocal;
            result.w = (right.y - upwards.x) * reciprocal;
        }

        return result;
    }

    inline Quaternion Quaternion::Normalize(Quaternion rotation) {
        float norm = Norm(rotation);
        return norm == 0.0f ? identity : rotation / norm;
    }

    inline Quaternion Quaternion::RotateTowards(Quaternion from, Quaternion to, float maxRadiansDelta) {
        float angle = Angle(from, to);
        if (angle == 0.0f) return to;
        maxRadiansDelta = std::max(maxRadiansDelta, angle - static_cast<float>(M_PI));
        return SlerpUnclamped(from, to, std::min(1.0f, maxRadiansDelta / angle));
    }

    inline Quaternion Quaternion::Slerp(Quaternion a, Quaternion b, float t) {
        if (t < 0.0f) return Normalize(a);
        if (t > 1.0f) return Normalize(b);
        return SlerpUnclamped(a, b, t);
    }

    inline Quaternion Quaternion::SlerpUnclamped(Quaternion a, Quaternion b, float t) {
        float dot = Dot(a, b);
        if (dot < 0.0f) {
            b = -b;
            dot = -dot;
        }

        if (dot < 0.95f) {
            float angle = std::acos(dot);
            return (a * std::sin(angle * (1.0f - t)) + b * std::sin(angle * t)) * (1.0f / std::sin(angle));
        }

        return LerpUnclamped(a, b, t);
    }

    inline void Quaternion::ToAngleAxis(Quaternion rotation, float& angle, Vector3& axis) {
        if (rotation.w > 1.0f) rotation = Normalize(rotation);
        angle = 2.0f * std::acos(rotation.w);
        float scale = std::sqrt(1.0f - rotation.w * rotation.w);
        axis = scale < 0.00001f ? Vector3::right : Vector3(rotation.x / scale, rotation.y / scale, rotation.z / scale);
    }

    inline Vector3 Quaternion::ToEuler(Quaternion value, bool toDeg) {
        Vector3 rotation{};
        float xy = value.x * value.y;
        float xw = value.x * value.w;
        float yz = value.y * value.z;
        float zw = value.z * value.w;
        float singularity = yz - xw;

        rotation.x = -std::asin(std::clamp(2.0f * singularity, -1.0f, 1.0f));

        if (std::fabs(singularity) < 0.499999f) {
            float xx = value.x * value.x;
            float yy = value.y * value.y;
            float yw = value.y * value.w;
            float zz = value.z * value.z;
            float ww = value.w * value.w;
            rotation.y = std::atan2(2.0f * (value.x * value.z + yw), zz - xx - yy + ww);
            rotation.z = std::atan2(2.0f * (xy + zw), yy - zz - xx + ww);
        } else {
            float a = xy + zw;
            float b = -yz + xw;
            float c = xy - zw;
            float e = yz + xw;
            rotation.y = std::atan2(a * e + b * c, b * e - a * c);
            rotation.z = 0.0f;
        }

        return toDeg ? rotation * Rad2Deg : rotation;
    }

    inline Vector3 Quaternion::RotateVectorByQuaternion(Quaternion lhs, Vector3 rhs) {
        float x2 = lhs.x * 2.0f;
        float y2 = lhs.y * 2.0f;
        float z2 = lhs.z * 2.0f;
        float xx = lhs.x * x2;
        float yy = lhs.y * y2;
        float zz = lhs.z * z2;
        float xy = lhs.x * y2;
        float xz = lhs.x * z2;
        float yz = lhs.y * z2;
        float wx = lhs.w * x2;
        float wy = lhs.w * y2;
        float wz = lhs.w * z2;

        return {
            (1.0f - (yy + zz)) * rhs.x + (xy - wz) * rhs.y + (xz + wy) * rhs.z,
            (xy + wz) * rhs.x + (1.0f - (xx + zz)) * rhs.y + (yz - wx) * rhs.z,
            (xz - wy) * rhs.x + (yz + wx) * rhs.y + (1.0f - (xx + yy)) * rhs.z
        };
    }
}
#ifndef UNITY_VERSION_MAJOR
#define UNITY_VERSION_MAJOR 2019
#endif

#ifndef UNITY_VERSION_MINOR
#define UNITY_VERSION_MINOR 4
#endif

#ifndef UNITY_VERSION_PATCH
#define UNITY_VERSION_PATCH 22
#endif

#ifndef UNITY_VERSION_TYPE
#define UNITY_VERSION_TYPE f1
#endif

#ifndef UNITY_VER
#define UNITY_VER 194
#endif

namespace StructureUtils {
    template <typename T>
    struct DataIterator {
        T* value = nullptr;

        constexpr DataIterator() = default;
        constexpr explicit DataIterator(T* value) : value(value) {}

        // Returns true when the iterator points at a valid value.
        explicit operator bool() const {
            return value != nullptr;
        }

        // Returns a writable reference to the pointed value.
        T& operator*() {
            return *value;
        }

        // Returns a read-only reference to the pointed value.
        const T& operator*() const {
            return *value;
        }

        // Returns the raw pointer held by this iterator.
        T* get() const {
            return value;
        }
    };
}

namespace MonoStructures {
    struct decimal {
        int flags = 0;
        int hi = 0;
        int lo = 0;
        int mid = 0;
    };

    struct String : Il2CppString {
        // Returns the UTF-16 character buffer owned by the managed string.
        const Il2CppChar* getChars() const {
            return chars;
        }

        // Returns the managed string length in UTF-16 code units.
        int getLength() const {
            return length;
        }

        // Returns true when the managed string has no characters.
        bool empty() const {
            return length <= 0;
        }

        // Converts the managed UTF-16 string into a UTF-8 std::string.
        std::string str() const {
            std::string out;

            if (length <= 0) {
                return out;
            }

            out.reserve(static_cast<size_t>(length));

            for (int i = 0; i < length; ++i) {
                uint32_t code = chars[i];

                if (code >= 0xD800 && code <= 0xDBFF && i + 1 < length) {
                    uint32_t low = chars[i + 1];

                    if (low >= 0xDC00 && low <= 0xDFFF) {
                        code = 0x10000 + ((code - 0xD800) << 10) + (low - 0xDC00);
                        ++i;
                    }
                }

                if (code <= 0x7F) {
                    out.push_back(static_cast<char>(code));
                } else if (code <= 0x7FF) {
                    out.push_back(static_cast<char>(0xC0 | (code >> 6)));
                    out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
                } else if (code <= 0xFFFF) {
                    out.push_back(static_cast<char>(0xE0 | (code >> 12)));
                    out.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
                    out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
                } else {
                    out.push_back(static_cast<char>(0xF0 | (code >> 18)));
                    out.push_back(static_cast<char>(0x80 | ((code >> 12) & 0x3F)));
                    out.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
                    out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
                }
            }

            return out;
        }

        // Converts the managed string into a UTF-8 std::string.
        std::string toString() const {
            return str();
        }

        // Returns a stable hash for the managed string contents.
        uint32_t getHash() const {
            uint32_t hash = 2166136261u;

            for (int i = 0; i < length; ++i) {
                hash ^= chars[i];
                hash *= 16777619u;
            }

            return hash;
        }

        // Returns true when a managed string pointer is null or empty.
        static bool IsNullOrEmpty(const String* value) {
            return !value || value->empty();
        }
    };

    template <typename T>
    struct Array : Il2CppArray {
        T m_Items[1];

        // Returns the managed array capacity.
        il2cpp_array_size_t GetCapacity() const {
            return bounds ? bounds->length : max_length;
        }

        // Returns the managed array size.
        il2cpp_array_size_t GetSize() const {
            return GetCapacity();
        }

        // Returns the managed array capacity as an int.
        int getCapacity() const {
            return static_cast<int>(GetCapacity());
        }

        // Returns a writable pointer to the first array item.
        T* GetData() {
            return m_Items;
        }

        // Returns a read-only pointer to the first array item.
        const T* GetData() const {
            return m_Items;
        }

        // Returns a writable pointer to the first array item.
        T* getPointer() {
            return GetData();
        }

        // Returns a read-only pointer to the first array item.
        const T* getPointer() const {
            return GetData();
        }

        // Returns true when the managed array has no items.
        bool Empty() const {
            return GetCapacity() == 0;
        }

        // Returns true when the managed array has no items.
        bool empty() const {
            return Empty();
        }

        // Copies the managed array items into a native vector.
        std::vector<T> ToVector() const {
            std::vector<T> items;
            il2cpp_array_size_t capacity = GetCapacity();

            items.reserve(static_cast<size_t>(capacity));

            for (il2cpp_array_size_t i = 0; i < capacity; ++i) {
                items.push_back(m_Items[i]);
            }

            return items;
        }

        // Copies the managed array items into a native vector.
        std::vector<T> toVector() const {
            return ToVector();
        }

        // Copies native vector data into this array when it fits.
        bool CopyFrom(const std::vector<T>& values) {
            return CopyFrom(values.data(), values.size());
        }

        // Copies native pointer data into this array when it fits.
        bool CopyFrom(const T* values, size_t size) {
            if (!values || size > static_cast<size_t>(GetCapacity())) {
                return false;
            }

            std::memcpy(m_Items, values, size * sizeof(T));
            return true;
        }

        // Copies this array into a native pointer.
        void CopyTo(T* values) const {
            if (!values) {
                return;
            }

            std::memcpy(values, m_Items, static_cast<size_t>(GetCapacity()) * sizeof(T));
        }

        // Returns an iterator pointing at an array item.
        StructureUtils::DataIterator<T> At(il2cpp_array_size_t index) {
            if (index >= GetCapacity()) {
                return {};
            }

            return StructureUtils::DataIterator<T>(&m_Items[index]);
        }

        // Returns an iterator pointing at a read-only array item.
        StructureUtils::DataIterator<const T> At(il2cpp_array_size_t index) const {
            if (index >= GetCapacity()) {
                return {};
            }

            return StructureUtils::DataIterator<const T>(&m_Items[index]);
        }

        // Returns a writable item reference by index.
        T& operator[](il2cpp_array_size_t index) {
            return m_Items[index];
        }

        // Returns a read-only item reference by index.
        const T& operator[](il2cpp_array_size_t index) const {
            return m_Items[index];
        }

        // Allocates an unmanaged array-shaped buffer for native-side scratch usage.
        static Array<T>* Create(size_t capacity) {
            size_t extraItems = capacity > 0 ? capacity - 1 : 0;
            size_t bytes = sizeof(Array<T>) + extraItems * sizeof(T);
            auto* array = static_cast<Array<T>*>(std::calloc(1, bytes));

            if (array) {
                array->max_length = static_cast<il2cpp_array_size_t>(capacity);
            }

            return array;
        }

        // Allocates an unmanaged array-shaped buffer and fills it from a vector.
        static Array<T>* Create(const std::vector<T>& values) {
            Array<T>* array = Create(values.size());

            if (array) {
                array->CopyFrom(values);
            }

            return array;
        }

        // Releases an unmanaged array-shaped buffer created by Create.
        void Destroy() {
            std::free(this);
        }
    };

    template <typename T>
    struct List : Il2CppObject {
        struct Enumerator {
            List<T>* list = nullptr;
            int index = 0;
            int version = 0;
            T current{};

            // Returns a pointer to the first list item.
            T* begin() {
                return list ? list->GetData() : nullptr;
            }

            // Returns a pointer one past the last list item.
            T* end() {
                return list && list->GetData() ? list->GetData() + list->size : nullptr;
            }
        };

        Array<T>* items = nullptr;
        int size = 0;
        int version = 0;
        void* syncRoot = nullptr;

        // Returns the active managed list size.
        int GetSize() const {
            return size;
        }

        // Returns the active managed list size.
        int getSize() const {
            return GetSize();
        }

        // Returns the backing managed array capacity.
        int GetCapacity() const {
            return items ? static_cast<int>(items->GetCapacity()) : 0;
        }

        // Returns the backing managed array capacity.
        int getCapacity() const {
            return GetCapacity();
        }

        // Returns the list version.
        int GetVersion() const {
            return version;
        }

        // Returns a writable pointer to the backing list data.
        T* GetData() {
            return items ? items->GetData() : nullptr;
        }

        // Returns a read-only pointer to the backing list data.
        const T* GetData() const {
            return items ? items->GetData() : nullptr;
        }

        // Returns a writable pointer to the backing list data.
        T* getData() {
            return GetData();
        }

        // Returns a read-only pointer to the backing list data.
        const T* getData() const {
            return GetData();
        }

        // Returns true when the managed list has no active items.
        bool Empty() const {
            return size <= 0;
        }

        // Returns true when the managed list has no active items.
        bool empty() const {
            return Empty();
        }

        // Copies the active list items into a native vector.
        std::vector<T> ToVector() const {
            std::vector<T> values;
            const T* data = GetData();

            if (!data || size <= 0) {
                return values;
            }

            values.reserve(static_cast<size_t>(size));

            for (int i = 0; i < size; ++i) {
                values.push_back(data[i]);
            }

            return values;
        }

        // Copies the active list items into a native vector.
        std::vector<T> toVector() const {
            return ToVector();
        }

        // Returns an iterator pointing at a list item.
        StructureUtils::DataIterator<T> At(int index) {
            if (index < 0 || index >= size || !GetData()) {
                return {};
            }

            return StructureUtils::DataIterator<T>(&GetData()[index]);
        }

        // Returns an iterator pointing at a read-only list item.
        StructureUtils::DataIterator<const T> At(int index) const {
            if (index < 0 || index >= size || !GetData()) {
                return {};
            }

            return StructureUtils::DataIterator<const T>(&GetData()[index]);
        }

        // Returns a writable list item reference by index.
        T& operator[](int index) {
            return GetData()[index];
        }

        // Returns a read-only list item reference by index.
        const T& operator[](int index) const {
            return GetData()[index];
        }

        // Returns a list item by index or a default value when out of bounds.
        T get_Item(int index) const {
            if (index < 0 || index >= size || !GetData()) {
                return {};
            }

            return GetData()[index];
        }

        // Writes a list item when the index is in bounds.
        void set_Item(int index, T item) {
            if (index < 0 || index >= size || !GetData()) {
                return;
            }

            GetData()[index] = item;
            ++version;
        }

        // Returns the first index of a value in the list.
        int IndexOf(T item) const {
            const T* data = GetData();

            if (!data) {
                return -1;
            }

            for (int i = 0; i < size; ++i) {
                if (data[i] == item) {
                    return i;
                }
            }

            return -1;
        }

        // Returns true when the list contains a value.
        bool Contains(T item) const {
            return IndexOf(item) != -1;
        }

        // Clears the active list range.
        void Clear() {
            T* data = GetData();

            if (data && size > 0) {
                std::memset(data, 0, static_cast<size_t>(size) * sizeof(T));
            }

            size = 0;
            ++version;
        }

        // Resizes the backing array for unmanaged scratch lists.
        bool Resize(int newCapacity) {
            if (newCapacity <= GetCapacity()) {
                return false;
            }

            Array<T>* newItems = Array<T>::Create(static_cast<size_t>(newCapacity));

            if (!newItems) {
                return false;
            }

            if (items && GetCapacity() > 0) {
                std::memcpy(newItems->GetData(), items->GetData(), static_cast<size_t>(GetCapacity()) * sizeof(T));
            }

            items = newItems;
            return true;
        }

        // Ensures enough backing capacity for additional items.
        void GrowIfNeeded(int additionalItems) {
            if (size + additionalItems > GetCapacity()) {
                int newCapacity = std::max(size + additionalItems, std::max(4, GetCapacity() * 2));
                Resize(newCapacity);
            }
        }

        // Adds an item to the end of the list.
        void Add(T item) {
            GrowIfNeeded(1);

            if (!GetData()) {
                return;
            }

            GetData()[size++] = item;
            ++version;
        }

        // Removes an item at the requested index.
        void RemoveAt(int index) {
            if (index < 0 || index >= size || !GetData()) {
                return;
            }

            if (index < size - 1) {
                std::memmove(GetData() + index, GetData() + index + 1, static_cast<size_t>(size - index - 1) * sizeof(T));
            }

            --size;
            std::memset(GetData() + size, 0, sizeof(T));
            ++version;
        }

        // Removes the first matching item.
        bool Remove(T item) {
            int index = IndexOf(item);

            if (index == -1) {
                return false;
            }

            RemoveAt(index);
            return true;
        }

        // Inserts an item at the requested index.
        void Insert(int index, T item) {
            if (index < 0 || index > size) {
                return;
            }

            GrowIfNeeded(1);

            if (!GetData()) {
                return;
            }

            if (index < size) {
                std::memmove(GetData() + index + 1, GetData() + index, static_cast<size_t>(size - index) * sizeof(T));
            }

            GetData()[index] = item;
            ++size;
            ++version;
        }

        // Copies native vector data into this list.
        bool CopyFrom(const std::vector<T>& values) {
            return CopyFrom(values.data(), static_cast<int>(values.size()));
        }

        // Copies native pointer data into this list.
        bool CopyFrom(const T* values, int valueCount) {
            if (!values || valueCount < 0) {
                return false;
            }

            if (valueCount > GetCapacity()) {
                Resize(valueCount);
            }

            if (!GetData()) {
                return false;
            }

            std::memcpy(GetData(), values, static_cast<size_t>(valueCount) * sizeof(T));
            size = valueCount;
            ++version;
            return true;
        }

        // Copies this list into a managed array at a target index.
        void CopyTo(Array<T>* target, int targetIndex = 0) const {
            if (!target || targetIndex < 0 || targetIndex + size > static_cast<int>(target->GetCapacity())) {
                return;
            }

            std::memcpy(target->GetData() + targetIndex, GetData(), static_cast<size_t>(size) * sizeof(T));
        }

        // Returns an enumerator over this list.
        Enumerator GetEnumerator() {
            return Enumerator{this, 0, version, {}};
        }
    };

    template <typename TKey, typename TValue>
    struct Dictionary : Il2CppObject {
        struct Entry {
            int hashCode = 0;
            int next = 0;
            TKey key{};
            TValue value{};
        };

        struct KeyCollection : Il2CppObject {
            Dictionary<TKey, TValue>* dictionary = nullptr;
        };

        struct ValueCollection : Il2CppObject {
            Dictionary<TKey, TValue>* dictionary = nullptr;
        };

        Array<int>* buckets = nullptr;
        Array<Entry>* entries = nullptr;
        int count = 0;
        int version = 0;
        int freeList = 0;
        int freeCount = 0;
        void* comparer = nullptr;
        KeyCollection* keys = nullptr;
        ValueCollection* values = nullptr;
        void* syncRoot = nullptr;

        // Returns the number of active dictionary entries.
        int GetSize() const {
            int activeCount = count - freeCount;
            return activeCount > 0 ? activeCount : 0;
        }

        // Returns the number of active dictionary entries.
        int getSize() const {
            return GetSize();
        }

        // Returns the raw dictionary slot count.
        int GetRawCount() const {
            return count;
        }

        // Returns the dictionary version.
        int GetVersion() const {
            return version;
        }

        // Returns true when the dictionary has no active entries.
        bool Empty() const {
            return GetSize() <= 0;
        }

        // Returns true when the dictionary has no active entries.
        bool empty() const {
            return Empty();
        }

        // Looks up a dictionary value by key through the entries array.
        bool TryGet(const TKey& wantedKey, TValue* outValue) const {
            if (!entries || !outValue || count <= 0) {
                return false;
            }

            const Entry* data = entries->GetData();
            int limit = std::min(count, entries->getCapacity());

            for (int i = 0; i < limit; ++i) {
                const Entry& entry = data[i];

                if (entry.hashCode >= 0 && entry.key == wantedKey) {
                    *outValue = entry.value;
                    return true;
                }
            }

            return false;
        }

        // Looks up a dictionary value by key through the entries array.
        bool tryGet(const TKey& wantedKey, TValue* outValue) const {
            return TryGet(wantedKey, outValue);
        }

        // Returns a dictionary value by key or a default value.
        TValue Get(const TKey& key) const {
            TValue value{};
            TryGet(key, &value);
            return value;
        }

        // Returns true when the dictionary contains a key.
        bool ContainsKey(const TKey& key) const {
            TValue value{};
            return TryGet(key, &value);
        }

        // Returns true when the dictionary contains a value.
        bool ContainsValue(const TValue& value) const {
            if (!entries || count <= 0) {
                return false;
            }

            const Entry* data = entries->GetData();
            int limit = std::min(count, entries->getCapacity());

            for (int i = 0; i < limit; ++i) {
                const Entry& entry = data[i];

                if (entry.hashCode >= 0 && entry.value == value) {
                    return true;
                }
            }

            return false;
        }

        // Copies active dictionary entries into a native vector.
        std::vector<std::pair<TKey, TValue>> ToVector() const {
            std::vector<std::pair<TKey, TValue>> output;

            if (!entries || count <= 0) {
                return output;
            }

            const Entry* data = entries->GetData();
            int limit = std::min(count, entries->getCapacity());
            output.reserve(static_cast<size_t>(GetSize()));

            for (int i = 0; i < limit; ++i) {
                const Entry& entry = data[i];

                if (entry.hashCode >= 0) {
                    output.emplace_back(entry.key, entry.value);
                }
            }

            return output;
        }

        // Copies active dictionary entries into a native vector.
        std::vector<std::pair<TKey, TValue>> toVector() const {
            return ToVector();
        }

        // Copies active dictionary entries into a native map.
        std::map<TKey, TValue> ToMap() const {
            std::map<TKey, TValue> output;

            for (const auto& entry : ToVector()) {
                output.emplace(entry.first, entry.second);
            }

            return output;
        }

        // Copies active dictionary keys into a native vector.
        std::vector<TKey> GetKeys() const {
            std::vector<TKey> output;

            for (const auto& entry : ToVector()) {
                output.push_back(entry.first);
            }

            return output;
        }

        // Copies active dictionary values into a native vector.
        std::vector<TValue> GetValues() const {
            std::vector<TValue> output;

            for (const auto& entry : ToVector()) {
                output.push_back(entry.second);
            }

            return output;
        }

        // Returns a dictionary value by key or a default value.
        TValue operator[](const TKey& key) const {
            return Get(key);
        }
    };
}

namespace DelegateStructures {
    struct DelegateBase : Il2CppDelegate {
        // Returns the managed target instance for this delegate.
        Il2CppObject* GetInstance() const {
            return target;
        }

        // Returns the managed method metadata for this delegate.
        const MethodInfo* GetMethodInfo() const {
            return method;
        }

        // Returns true when the delegate has callable method data.
        bool IsValid() const {
            return method_ptr != nullptr || method != nullptr;
        }

        // Returns true when the delegate has callable method data.
        explicit operator bool() const {
            return IsValid();
        }
    };

    struct MulticastDelegateBase : Il2CppMulticastDelegate {
        // Returns the managed delegate invocation list.
        MonoStructures::Array<DelegateBase*>* GetDelegates() const {
            return reinterpret_cast<MonoStructures::Array<DelegateBase*>*>(delegates);
        }

        // Returns true when the multicast delegate has callable method data.
        bool IsValid() const {
            return delegate.method_ptr != nullptr || delegate.method != nullptr || delegates != nullptr;
        }

        // Returns true when the multicast delegate has callable method data.
        explicit operator bool() const {
            return IsValid();
        }
    };

    template <typename Ret>
    struct Delegate : DelegateBase {};

    template <typename Ret>
    struct MulticastDelegate : MulticastDelegateBase {};
}

namespace MonoStructures {
    template <typename... Parameters>
    struct Action : DelegateStructures::MulticastDelegate<void> {};
}

namespace Unity {
    struct Rect {
        union {
            struct { float x, y, w, h; };
            float data[4];
        };

        constexpr Rect() : x(0.0f), y(0.0f), w(0.0f), h(0.0f) {}
        constexpr Rect(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}

        // Returns a writable pointer to the rectangle data.
        float* GetPtr() {
            return data;
        }

        // Returns a read-only pointer to the rectangle data.
        const float* GetPtr() const {
            return data;
        }

        // Returns true when two rectangles have equal components.
        bool operator==(const Rect& other) const {
            return x == other.x && y == other.y && w == other.w && h == other.h;
        }

        // Returns true when two rectangles have different components.
        bool operator!=(const Rect& other) const {
            return !(*this == other);
        }
    };

    struct Ray {
        Vector3 m_Origin{};
        Vector3 m_Direction{};
    };

    struct RaycastHit {
        Vector3 point{};
        Vector3 normal{};
        uint32_t faceID = 0;
        float distance = 0.0f;
        Vector2 UV{};
#if UNITY_VER > 174
        int m_Collider = 0;
#else
        void* m_Collider = nullptr;
#endif

        // Returns the collider handle stored by Unity.
        void* GetCollider() const {
#if UNITY_VER > 174
            return reinterpret_cast<void*>(static_cast<intptr_t>(m_Collider));
#else
            return m_Collider;
#endif
        }
    };

    struct RaycastHit2D {
        Vector2 centroid{};
        Vector2 point{};
        Vector2 normal{};
        float distance = 0.0f;
        float fraction = 0.0f;
#if UNITY_VER > 174
        int m_Collider = 0;
#else
        void* m_Collider = nullptr;
#endif

        // Returns the collider handle stored by Unity.
        void* GetCollider() const {
#if UNITY_VER > 174
            return reinterpret_cast<void*>(static_cast<intptr_t>(m_Collider));
#else
            return m_Collider;
#endif
        }
    };

    struct FrustumPlanes {
        float left = 0.0f;
        float right = 0.0f;
        float bottom = 0.0f;
        float top = 0.0f;
        float zNear = 0.0f;
        float zFar = 0.0f;
    };

    struct Matrix3x3 {
        float m_Data[9]{};

        Matrix3x3() = default;

        Matrix3x3(
            float m00,
            float m01,
            float m02,
            float m10,
            float m11,
            float m12,
            float m20,
            float m21,
            float m22
        ) {
            Get(0, 0) = m00;
            Get(0, 1) = m01;
            Get(0, 2) = m02;
            Get(1, 0) = m10;
            Get(1, 1) = m11;
            Get(1, 2) = m12;
            Get(2, 0) = m20;
            Get(2, 1) = m21;
            Get(2, 2) = m22;
        }

        // Returns a writable matrix value by row and column.
        float& Get(int row, int column) {
            return m_Data[row + column * 3];
        }

        // Returns a read-only matrix value by row and column.
        const float& Get(int row, int column) const {
            return m_Data[row + column * 3];
        }

        // Returns a writable pointer to matrix data.
        float* GetPtr() {
            return m_Data;
        }

        // Returns a read-only pointer to matrix data.
        const float* GetPtr() const {
            return m_Data;
        }

        // Returns a writable matrix value by raw index.
        float& operator[](int index) {
            return m_Data[index];
        }

        // Returns a read-only matrix value by raw index.
        const float& operator[](int index) const {
            return m_Data[index];
        }

        // Sets this matrix to identity.
        Matrix3x3& SetIdentity() {
            SetZero();
            Get(0, 0) = 1.0f;
            Get(1, 1) = 1.0f;
            Get(2, 2) = 1.0f;
            return *this;
        }

        // Sets this matrix to zero.
        Matrix3x3& SetZero() {
            std::memset(m_Data, 0, sizeof(m_Data));
            return *this;
        }
    };

    struct Matrix4x4 {
        enum class InitIdentity {
            kIdentity
        };

        float m_Data[16]{};

        Matrix4x4() = default;

        explicit Matrix4x4(InitIdentity) {
            SetIdentity();
        }

        explicit Matrix4x4(const float data[16]) {
            std::memcpy(m_Data, data, sizeof(m_Data));
        }

        // Returns a writable matrix value by row and column.
        float& Get(int row, int column) {
            return m_Data[row + column * 4];
        }

        // Returns a read-only matrix value by row and column.
        const float& Get(int row, int column) const {
            return m_Data[row + column * 4];
        }

        // Returns a writable pointer to matrix data.
        float* GetPtr() {
            return m_Data;
        }

        // Returns a read-only pointer to matrix data.
        const float* GetPtr() const {
            return m_Data;
        }

        // Returns a writable matrix value by raw index.
        float& operator[](int index) {
            return m_Data[index];
        }

        // Returns a read-only matrix value by raw index.
        const float& operator[](int index) const {
            return m_Data[index];
        }

        // Sets this matrix to identity.
        Matrix4x4& SetIdentity() {
            SetZero();
            Get(0, 0) = 1.0f;
            Get(1, 1) = 1.0f;
            Get(2, 2) = 1.0f;
            Get(3, 3) = 1.0f;
            return *this;
        }

        // Sets this matrix to zero.
        Matrix4x4& SetZero() {
            std::memset(m_Data, 0, sizeof(m_Data));
            return *this;
        }

        // Returns the translation column as a Vector3.
        Vector3 GetPosition() const {
            return {m_Data[12], m_Data[13], m_Data[14]};
        }
    };
}

namespace UnityEngineStructures {
    struct Object : Il2CppObject {
        intptr_t m_CachedPtr = 0;

        // Returns true when the managed object and native Unity pointer are valid.
        bool IsValid() const {
            return m_CachedPtr != 0;
        }

        // Returns true when the managed object and native Unity pointer are valid.
        bool Alive() const {
            return IsValid();
        }

        // Returns true when both Unity object wrappers point to the same native object.
        bool Same(const Object* object) const {
            if (!object) {
                return false;
            }

            return m_CachedPtr == object->m_CachedPtr;
        }
    };

    struct MonoBehaviour : Object {
#if UNITY_VER >= 222
        void* m_CancellationTokenSource = nullptr;
#endif
    };

    template <typename T>
    // Returns true when a Unity object wrapper points to a native Unity object.
    bool IsUnityObjectAlive(T object) {
        return object && reinterpret_cast<Object*>(object)->Alive();
    }

    template <typename T1, typename T2>
    // Returns true when two Unity object wrappers point to the same native object.
    bool IsSameUnityObject(T1 first, T2 second) {
        if (!first || !second) {
            return false;
        }

        return reinterpret_cast<Object*>(first)->Same(reinterpret_cast<Object*>(second));
    }

    template <typename... Parameters>
    struct UnityAction : DelegateStructures::MulticastDelegate<void> {};

    struct ArgumentCache : Il2CppObject {
        Object* m_ObjectArgument = nullptr;
        MonoStructures::String* m_ObjectArgumentAssemblyTypeName = nullptr;
        int m_IntArgument = 0;
        float m_FloatArgument = 0.0f;
        MonoStructures::String* m_StringArgument = nullptr;
        bool m_BoolArgument = false;
    };

    enum class PersistentListenerMode : int {
        EventDefined = 0,
        Void = 1,
        Object = 2,
        Int = 3,
        Float = 4,
        String = 5,
        Bool = 6
    };

    enum class UnityEventCallState : int {
        Off = 0,
        EditorAndRuntime = 1,
        RuntimeOnly = 2
    };

    struct PersistentCall : Il2CppObject {
        Object* m_Target = nullptr;
        MonoStructures::String* m_TargetAssemblyTypeName = nullptr;
        MonoStructures::String* m_MethodName = nullptr;
        PersistentListenerMode m_Mode = PersistentListenerMode::EventDefined;
        ArgumentCache* m_Arguments = nullptr;
        UnityEventCallState m_CallState = UnityEventCallState::RuntimeOnly;

        // Returns true when the persistent call has a target type and method name.
        bool IsValid() const {
            return !MonoStructures::String::IsNullOrEmpty(m_TargetAssemblyTypeName) &&
                   !MonoStructures::String::IsNullOrEmpty(m_MethodName);
        }
    };

    struct PersistentCallGroup : Il2CppObject {
        MonoStructures::List<PersistentCall*>* m_Calls = nullptr;
    };

    struct InvokableCallBase : Il2CppObject {
        UnityAction<>* action = nullptr;
    };

    template <typename... Parameters>
    struct InvokableCall : Il2CppObject {
        UnityAction<Parameters...>* action = nullptr;
    };

    struct InvokableCallList : Il2CppObject {
        MonoStructures::List<InvokableCallBase*>* m_PersistentCalls = nullptr;
        MonoStructures::List<InvokableCallBase*>* m_RuntimeCalls = nullptr;
        MonoStructures::List<InvokableCallBase*>* m_ExecutingCalls = nullptr;
        bool m_NeedsUpdate = false;
    };

    struct UnityEventBase : Il2CppObject {
        InvokableCallList* m_Calls = nullptr;
        PersistentCallGroup* m_PersistentCalls = nullptr;
        bool m_CallsDirty = true;
    };

    template <typename... Parameters>
    struct UnityEvent : UnityEventBase {
        MonoStructures::Array<Il2CppObject*>* m_InvokeArray = nullptr;
    };
}
