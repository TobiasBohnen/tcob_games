// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

// IWYU pragma: always_keep

#include <algorithm>
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

    auto normalized() const -> point3_d
    {
        f64 const len {std::sqrt(dot(*this))};
        return len > 1e-12 ? *this * (1.0 / len) : point3_d {.X = 0.0, .Y = 0.0, .Z = 1.0};
    }

    friend auto operator+(point3_d const& a, point3_d const& b) -> point3_d { return {.X = a.X + b.X, .Y = a.Y + b.Y, .Z = a.Z + b.Z}; }
    friend auto operator-(point3_d const& a, point3_d const& b) -> point3_d { return {.X = a.X - b.X, .Y = a.Y - b.Y, .Z = a.Z - b.Z}; }
    friend auto operator*(point3_d const& a, f64 s) -> point3_d { return {.X = a.X * s, .Y = a.Y * s, .Z = a.Z * s}; }
};

struct vec3_i {
    i32 X {0};
    i32 Y {0};
    i32 Z {0};
};

using mat3_d = std::array<f64, 3 * 3>;

struct color_d {
    f64 R {0.0};
    f64 G {0.0};
    f64 B {0.0};

    auto lerp(color_d const& other, f64 w) const -> color_d { return {.R = std::lerp(R, other.R, w), .G = std::lerp(G, other.G, w), .B = std::lerp(B, other.B, w)}; }
};

constexpr auto IsMagenta(u8 const* tex, i32 offset) -> bool { return tex[offset + 0] == 0x98 && tex[offset + 1] == 0x00 && tex[offset + 2] == 0x88; }
constexpr auto PixelIndex(size_i screenSize, isize x, isize y) -> isize { return x + (y * screenSize.Width); }
constexpr auto WorldToCell(point_d const& p) -> point_i { return {static_cast<i32>(std::floor(p.X)), static_cast<i32>(std::floor(p.Y))}; }

// Performs bilinear interpolation of corner heights (TL, TR, BL, BR)
// across a unit cell given fractional local coordinates (dx, dy).
inline auto HeightAt(auto const& slope, point_i cell, point_d const& pos) -> f64
{
    f64 const dx {std::clamp(pos.X - cell.X, 0.0, 1.0)};
    f64 const dy {std::clamp(pos.Y - cell.Y, 0.0, 1.0)};
    return (slope.TL * (1.0 - dx) * (1.0 - dy)) + (slope.TR * dx * (1.0 - dy)) + (slope.BL * (1.0 - dx) * dy) + (slope.BR * dx * dy);
}

////////////////////////////////////////////////////////////
