// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

#include <vector>

#include "Shading.hpp"

////////////////////////////////////////////////////////////

enum class ground_style : u8 {
    Water, // an animated surface on the cell's floor; with other colors: lava, slime, ...
    Haze,  // soft drifting layers of mist below the floor, fading into the dark
    Ice,   // a sheet with cracks
    Grid,  // glowing lines on a dark floor, every fourth one stronger
    Void   // a starfield
};

// what a cell shows instead of a floor texture: drawn from the screen row and the ray direction, like the sky
struct ground_settings {
    ground_style Style {};

    color_d Deep {};         // the depths
    color_d Light {};        // veins of light (water), the brightest mist (haze), cracks (ice), lines (grid), stars (void)
    color_d Haze {};         // what it fades to with distance

    f64 Scale {0.0};         // pattern features per world unit (ice: crack spacing, grid: lines)
    f64 Speed {0.0};         // how fast the pattern drifts (grid: scrolls, void: twinkle)
    f64 Fade {0.0};          // ray distance over which the haze takes over
    f64 Depth {0.0};         // haze: distance between the layers

    f64 Emissive {0.0};      // 0: shaded by the lights of the level like a floor, 1: glows by itself (lava, grid, stars)
    f64 GlowRange {0.0};     // cells: how far an emissive ground lights its surroundings with its Light color; 0: not at all
    f64 GlowIntensity {0.0}; // how strongly, at the ground itself
};

////////////////////////////////////////////////////////////

// the settings the cells refer to by id; id 0 is no ground
class ground {
public:
    auto at(usize id) const -> ground_settings const&;
    void set(usize id, ground_settings const& settings);

    // the color of the ground with this id on screen row y; t is the ray distance to the floor of the cell in that row and
    // lowestZ the lowest point of the cell. Unlit: the caller shades it by the lights unless the ground is emissive
    auto pixel(usize id, shade_view const& view, f64 t, f64 lowestZ, i32 y, point_d const& rayDir) const -> color_d;

private:
    std::vector<ground_settings> _settings;
};
