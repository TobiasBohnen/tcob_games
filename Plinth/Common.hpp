// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

// IWYU pragma: always_keep

#include <tcob/tcob.hpp>

using namespace tcob;
using namespace tcob::literals;
using namespace tcob::ui;
using namespace std::chrono_literals;

inline constexpr i32    TEXTURE_BPP {3};
inline constexpr size_i WALL_SIZE {64, 64};
inline constexpr isize  MAX_TRANSPARENT_WALLS {8};
inline constexpr f64    FOV {90};
inline constexpr f64    REFERENCE_HEIGHT {360.0};
inline constexpr f64    WEAPON_BOB_MULTIPLIER {2.0};

constexpr u32 LIGHT_LAYER_WORLD {1u << 0};
constexpr u32 LIGHT_LAYER_SPELL {1u << 1};
constexpr u32 LIGHT_LAYERS_ALL {~0u};

inline constexpr i32 NUM_FACINGS {16};

class texture_cache;
class level;
class player;
struct dynamic_light;
class voxel_object;
class sprite_object;

////////////////////////////////////////////////////////////

enum class quality_level : u8 {
    Low,
    Medium,
    High,
    Ultra,
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
};

constexpr auto IsMagenta(u8 const* tex, i32 offset) -> bool { return tex[offset + 0] == 0x98 && tex[offset + 1] == 0x00 && tex[offset + 2] == 0x88; }
constexpr auto PixelIndex(size_i screenSize, isize x, isize y) -> isize { return x + (y * screenSize.Width); }

////////////////////////////////////////////////////////////
