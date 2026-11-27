// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

// IWYU pragma: always_keep

#include <array>
#include <cmath>

#include <tcob/tcob.hpp> // IWYU pragma: export

using namespace tcob;
using namespace tcob::literals;
using namespace tcob::ui;
using namespace std::chrono_literals;

inline constexpr i32    TEXTURE_BPP {3};
inline constexpr size_i WALL_SIZE {64, 64};
inline constexpr isize  MAX_TRANSPARENT_WALLS {8};
inline constexpr f64    FOV {90};

inline constexpr u32 LIGHT_LAYER_WORLD {1u << 0};
inline constexpr u32 LIGHT_LAYER_SPELL {1u << 1};
inline constexpr u32 LIGHT_LAYERS_ALL {~0u};

inline constexpr i32 NUM_FACINGS {16};

inline constexpr i32 SKY_TEXTURE {-2};

class texture_cache;
class level;
class player;
struct dynamic_light;
class voxel_object;
class sprite_object;

////////////////////////////////////////////////////////////

enum class quality_level : u8 {
    Low,
    Normal,
    High,
    Custom
};

struct quality_settings {
    size_i Resolution;
    i32    VoxelPixelBudget {};
    i32    LightLosSamples {};
    bool   VoxelAo {};
    bool   Dithering {};
    bool   ContactShadows {};
};

////////////////////////////////////////////////////////////

struct point3_d {
    f64 X {0.0};
    f64 Y {0.0};
    f64 Z {0.0};

    auto dot(point3_d const& b) const -> f64 { return (X * b.X) + (Y * b.Y) + (Z * b.Z); }

    auto cross(point3_d const& b) const -> point3_d { return {.X = (Y * b.Z) - (Z * b.Y), .Y = (Z * b.X) - (X * b.Z), .Z = (X * b.Y) - (Y * b.X)}; }

    auto length() const -> f64 { return std::sqrt(length_squared()); }
    auto length_squared() const -> f64 { return dot(*this); }

    auto normalized() const -> point3_d
    {
        f64 const len {std::sqrt(dot(*this))};
        return len > 1e-12 ? *this * (1.0 / len) : point3_d {.X = 0.0, .Y = 0.0, .Z = 1.0};
    }

    auto constexpr xy() const -> point_d { return {X, Y}; }
    static constexpr auto Lift(point_d const& p, f64 z) -> point3_d { return {.X = p.X, .Y = p.Y, .Z = z}; }

    constexpr auto with_z(f64 z) const -> point3_d { return {.X = X, .Y = Y, .Z = z}; }
    constexpr auto with_xy(point_d const& p) const -> point3_d { return {.X = p.X, .Y = p.Y, .Z = Z}; }

    friend auto operator==(point3_d const&, point3_d const&) -> bool = default;
    friend auto operator+(point3_d const& a, point3_d const& b) -> point3_d { return {.X = a.X + b.X, .Y = a.Y + b.Y, .Z = a.Z + b.Z}; }
    friend auto operator-(point3_d const& a, point3_d const& b) -> point3_d { return {.X = a.X - b.X, .Y = a.Y - b.Y, .Z = a.Z - b.Z}; }
    friend auto operator*(point3_d const& a, f64 s) -> point3_d { return {.X = a.X * s, .Y = a.Y * s, .Z = a.Z * s}; }
};

struct vec3_i {
    i32 X {0};
    i32 Y {0};
    i32 Z {0};
};

struct screen_bounds {
    i32 Left {};
    i32 Right {};
    i32 Top {};
    i32 Bottom {};
    f64 Nearest {};     // nothing of the object is closer than this
    f64 MaxWallDist {}; // farthest wall behind its columns

    auto area() const -> i32 { return (Right - Left) * (Bottom - Top); }
};

using mat3_d = std::array<f64, 3 * 3>; // row-major

// R = Rz(yaw) * Ry(pitch) * Rx(roll)
inline auto Mat3FromEuler(radian_d yaw, radian_d pitch, radian_d roll) -> mat3_d
{
    f64 const cy {yaw.cos()}, sy {yaw.sin()};
    f64 const cp {pitch.cos()}, sp {pitch.sin()};
    f64 const cr {roll.cos()}, sr {roll.sin()};
    return {cy * cp, (cy * sp * sr) - (sy * cr), (cy * sp * cr) + (sy * sr),
            sy * cp, (sy * sp * sr) + (cy * cr), (sy * sp * cr) - (cy * sr),
            -sp, cp * sr, cp * cr};
}

// a * b: b is applied first
inline auto Mat3Multiply(mat3_d const& a, mat3_d const& b) -> mat3_d
{
    mat3_d r {};
    for (usize row {0}; row < 3; ++row) {
        for (usize col {0}; col < 3; ++col) {
            r[(row * 3) + col] = (a[row * 3] * b[col]) + (a[(row * 3) + 1] * b[3 + col]) + (a[(row * 3) + 2] * b[6 + col]);
        }
    }
    return r;
}

inline auto Mat3Rotate(mat3_d const& m, point3_d const& v) -> point3_d
{
    return {.X = (m[0] * v.X) + (m[1] * v.Y) + (m[2] * v.Z),
            .Y = (m[3] * v.X) + (m[4] * v.Y) + (m[5] * v.Z),
            .Z = (m[6] * v.X) + (m[7] * v.Y) + (m[8] * v.Z)};
}

// the transpose, which is the inverse of a rotation
inline auto Mat3RotateInverse(mat3_d const& m, point3_d const& v) -> point3_d
{
    return {.X = (m[0] * v.X) + (m[3] * v.Y) + (m[6] * v.Z),
            .Y = (m[1] * v.X) + (m[4] * v.Y) + (m[7] * v.Z),
            .Z = (m[2] * v.X) + (m[5] * v.Y) + (m[8] * v.Z)};
}

struct color_d {
    f64 R {0.0};
    f64 G {0.0};
    f64 B {0.0};

    auto constexpr lerp(color_d const& other, f64 w) const -> color_d { return {.R = std::lerp(R, other.R, w), .G = std::lerp(G, other.G, w), .B = std::lerp(B, other.B, w)}; }

    auto constexpr operator*=(f64 right) -> color_d& { return *this = {.R = R * right, .G = G * right, .B = B * right}; }
    auto constexpr operator*=(color_d const& right) -> color_d& { return *this = {.R = R * right.R, .G = G * right.G, .B = B * right.B}; }
    auto constexpr operator+=(f64 right) -> color_d& { return *this = {.R = R + right, .G = G + right, .B = B + right}; }
    auto constexpr operator+=(color_d const& right) -> color_d& { return *this = {.R = R + right.R, .G = G + right.G, .B = B + right.B}; }

    friend auto constexpr operator*(color_d left, f64 right) -> color_d { return left *= right; }
    friend auto constexpr operator*(color_d left, color_d const& right) -> color_d { return left *= right; }
    friend auto constexpr operator+(color_d left, f64 right) -> color_d { return left += right; }
    friend auto constexpr operator+(color_d left, color_d const& right) -> color_d { return left += right; }
};

constexpr auto PackABGR(u8 r, u8 g, u8 b) -> u32 { return 0xFF000000u | (static_cast<u32>(b) << 16) | (static_cast<u32>(g) << 8) | static_cast<u32>(r); }
constexpr auto IsMagenta(u8 const* tex, i32 offset) -> bool { return tex[offset + 0] == 0x98 && tex[offset + 1] == 0x00 && tex[offset + 2] == 0x88; }
constexpr auto PixelIndex(isize width, isize x, isize y) -> usize { return static_cast<usize>(x + (y * width)); }
constexpr auto WorldToCell(point_d const& p) -> point_i { return {static_cast<i32>(std::floor(p.X)), static_cast<i32>(std::floor(p.Y))}; }

////////////////////////////////////////////////////////////
