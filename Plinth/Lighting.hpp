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

    auto lights() const -> std::span<dynamic_light const>;
    auto lights_at(point_i map) const -> std::span<cell_light const>;

    auto glow_at(point_d pos) const -> color_d;    // the light that emissive ground spreads, see rebuild_glow
    auto blocks_light(point_i cell) const -> bool; // a flat lookup of what map::is_blocking_light said at the last rebuild
    void invalidate();

private:
    void rebuild_blocking(level const& lvl);

    std::vector<u8>                      _blocked; // per cell: whether it blocks light, as of the last rebuild
    std::vector<color_d>                 _glow;    // per cell: the light of the emissive ground around
    std::vector<dynamic_light>           _lights;
    std::vector<std::vector<cell_light>> _cellLights;
    usize                                _losSamples {0};
    bool                                 _dirty {true};

    task_manager& _taskManager;
};
