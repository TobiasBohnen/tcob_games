// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <span>
#include <vector>

#include "Common.hpp"

////////////////////////////////////////////////////////////

struct dynamic_light {
    point3_d Position {};
    f64      Range {8.0};
    color    Color {colors::White};
    f64      Intensity {1.0};
    u32      Layers {LIGHT_LAYER_WORLD};
};

////////////////////////////////////////////////////////////

// what the shading of a pixel needs of a light, worked out once per frame
struct light_params {
    point_d Position {};
    f64     Z {0.0};
    f64     RangeSq {0.0};
    f64     InvRangeSq {0.0};
    f64     R {0.0}; // color * intensity, in 0..1
    f64     G {0.0};
    f64     B {0.0};
    u32     Layers {LIGHT_LAYER_WORLD};
};

inline auto MakeLightParams(point_d pos, f64 z, f64 range, f64 intensity, color c, u32 layers = LIGHT_LAYER_WORLD) -> light_params
{
    f64 const scale {intensity / 255.0};
    f64 const rangeSq {range * range};
    return {.Position = pos, .Z = z, .RangeSq = rangeSq, .InvRangeSq = 1.0 / rangeSq, .R = c.R * scale, .G = c.G * scale, .B = c.B * scale, .Layers = layers};
}

////////////////////////////////////////////////////////////

// a light that reaches a cell
struct cell_light {
    u32 Light {0};                        // index into lights()
    u16 Seen {0xFFFF};                    // the 4x4 parts of the cell, row by row, that see the light: all, unless something is in the way of some of them

    auto sees(point_d pos) const -> bool; // whether the part of the cell that `pos` is in sees the light
};

////////////////////////////////////////////////////////////

class lighting {
public:
    lighting();

    void update(level const& lvl, usize losSamples);

    auto lights() const -> std::span<light_params const>;
    auto lights_at(point_i map) const -> std::span<cell_light const>;

    auto glow_at(point_d pos) const -> color_d;    // the light that emissive floors spread, see rebuild_glow
    auto blocks_light(point_i cell) const -> bool; // a flat lookup of what map::is_blocking_light said at the last rebuild
    void invalidate();

private:
    void rebuild_blocking(level const& lvl);
    void rebuild_glow(level const& lvl);

    std::vector<u8>                      _blocked; // per cell: whether it blocks light, as of the last rebuild
    std::vector<color_d>                 _glow;    // per cell: the light of the emissive floors around
    std::vector<dynamic_light>           _lights;
    std::vector<light_params>            _params;  // per light, same order
    std::vector<std::vector<cell_light>> _cellLights;
    usize                                _losSamples {0};
    bool                                 _dirty {true};

    task_manager& _taskManager;
};
