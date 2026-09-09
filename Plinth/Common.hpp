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
inline constexpr f64    WEAPON_REFERENCE_HEIGHT {360.0};
inline constexpr f64    WEAPON_BOB_MULTIPLIER {2.0};

inline constexpr i32 NUM_FACINGS {16};
i32 const            VOXEL_SIZE {256};

class texture_cache;
class level;
class player;

////////////////////////////////////////////////////////////

struct vec3_d {
    f64 X {0.0};
    f64 Y {0.0};
    f64 Z {0.0};

    auto dot(vec3_d const& b) const -> f64;
    auto normalized() const -> vec3_d;

    friend auto operator+(vec3_d const& a, vec3_d const& b) -> vec3_d { return {.X = a.X + b.X, .Y = a.Y + b.Y, .Z = a.Z + b.Z}; }
    friend auto operator-(vec3_d const& a, vec3_d const& b) -> vec3_d { return {.X = a.X - b.X, .Y = a.Y - b.Y, .Z = a.Z - b.Z}; }
    friend auto operator*(vec3_d const& a, f64 s) -> vec3_d { return {.X = a.X * s, .Y = a.Y * s, .Z = a.Z * s}; }
};

struct vec3_i {
    i32 X {0};
    i32 Y {0};
    i32 Z {0};
};

////////////////////////////////////////////////////////////
