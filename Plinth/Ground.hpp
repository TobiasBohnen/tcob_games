// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

#include <vector>

////////////////////////////////////////////////////////////

enum class ground_style : u8 {
    Water,  // an animated surface at the height of the cell's floor; with other colors: lava, slime, ...
    Canyon, // layers of rock ledges below the floor, seen through the gaps of the layers above
    Void    // a starfield
};

// what a cell shows instead of a floor texture: drawn from the screen row and the ray direction, like the sky
struct ground_settings {
    ground_style Style {ground_style::Water};

    color_d Deep {0.05, 0.26, 0.34};  // the depths
    color_d Light {0.30, 0.55, 0.62}; // veins of light (water), lit ledges (canyon), stars (void)
    color_d Haze {0.10, 0.20, 0.26};  // what it fades to with distance

    f64 Scale {2.5};                  // pattern features per world unit
    f64 Speed {0.12};                 // how fast the pattern drifts
    f64 Fade {14.0};                  // ray distance over which the haze takes over
    f64 Depth {2.5};                  // canyon: distance between the layers
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
