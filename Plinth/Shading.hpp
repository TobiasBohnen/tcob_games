// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

#include <algorithm>

////////////////////////////////////////////////////////////

// what the procedural backdrops (sky, ground) need to know about the camera and the screen
struct shade_view {
    procgen::value_noise const* Noise {nullptr};

    f64     ProjPlaneDist {1.0};
    i32     ScreenHeight {0};
    f64     CenterY {0.0}; // the row of the horizon, with the view bob
    point_d Position {};
    f64     EyeZ {0.0};
    f64     Time {0.0};    // seconds
};

////////////////////////////////////////////////////////////

inline auto ApplyTint(u8 r, u8 g, u8 b, color_d tint) -> u32
{
    u8 const tr {static_cast<u8>(std::min(r * tint.R, 255.0))};
    u8 const tg {static_cast<u8>(std::min(g * tint.G, 255.0))};
    u8 const tb {static_cast<u8>(std::min(b * tint.B, 255.0))};
    return PackABGR(tr, tg, tb);
}

inline auto HashUnit(i32 gx, i32 gy, u32 index, u32 salt) -> f64
{
    u32 h {(static_cast<u32>(gx) * 73856093u) ^ (static_cast<u32>(gy) * 19349663u) ^ (index * 83492791u) ^ (salt * 2654435761u)};
    h ^= h >> 16;
    h *= 0x7feb352du;
    h ^= h >> 15;
    h *= 0x846ca68bu;
    h ^= h >> 16;
    return static_cast<f64>(h) / 4294967296.0;
}

inline auto BlendPixel(u32 a, u32 b, f64 t) -> u32
{
    u32 const w {static_cast<u32>(std::clamp(t, 0.0, 1.0) * 256.0)};
    u32 const inv {256 - w};
    u32 const rb {((((a & 0x00FF00FFu) * inv) + ((b & 0x00FF00FFu) * w)) >> 8) & 0x00FF00FFu};
    u32 const g {((((a & 0x0000FF00u) * inv) + ((b & 0x0000FF00u) * w)) >> 8) & 0x0000FF00u};
    return 0xFF000000u | rb | g;
}

inline auto SmoothStep(f64 e0, f64 e1, f64 x) -> f64
{
    f64 const u {std::clamp((x - e0) / (e1 - e0), 0.0, 1.0)};
    return u * u * (3.0 - (2.0 * u));
}
