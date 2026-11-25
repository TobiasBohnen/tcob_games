// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

#include <vector>

////////////////////////////////////////////////////////////

enum class ground_style : u8 {
    Water,  // an animated surface on the cell's floor; with other colors: lava, slime, ...
    Canyon, // layers of rock ledges below the floor, seen through the gaps of the layers above
    Ice,    // a sheet with cracks
    Grid,   // glowing lines on a dark floor, every fourth one stronger
    Void    // a starfield
};

// what a cell shows instead of a floor texture: drawn from the screen row and the ray direction, like the sky
struct ground_settings {
    ground_style Style {ground_style::Water};

    color_d Deep {.R = 0.05, .G = 0.26, .B = 0.34};  // the depths
    color_d Light {.R = 0.30, .G = 0.55, .B = 0.62}; // veins of light (water), lit ledges (canyon), cracks (ice), lines (grid), stars (void)
    color_d Haze {.R = 0.10, .G = 0.20, .B = 0.26};  // what it fades to with distance

    f64 Scale {2.5};                                 // pattern features per world unit (ice: crack spacing, grid: lines)
    f64 Speed {0.12};                                // how fast the pattern drifts (grid: scrolls, void: twinkle)
    f64 Fade {14.0};                                 // ray distance over which the haze takes over
    f64 Depth {2.5};                                 // canyon: distance between the layers

    f64 Emissive {0.0};                              // 0: shaded by the lights of the level like a floor, 1: glows by itself (lava, grid, stars)
};

////////////////////////////////////////////////////////////

// the settings the cells refer to by id; id 0 is no ground
class ground {
public:
    auto at(usize id) const -> ground_settings const&;
    void set(usize id, ground_settings const& settings);

private:
    std::vector<ground_settings> _settings;
};
