// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

#include <vector>

#include "Shading.hpp"

struct sky_body_frame {
    point3_d Dir;
    f64      CosR {};
    f64      SinR {};
    f64      SunDot {};
    f64      Ambient {};
    bool     Lit {};
    color_d  Color;
    f64      Noise {};
    f64      NoiseScale {};
    f64      NoiseSeed {};
    color_d  NoiseColor {};
};

struct sky_frame {
    point3_d Sun;
    f64      Day {};
    f64      Night {};
    f64      Dusk {};
    f64      CosSun {};
    f64      Time {};
    bool     Stars {};
    f64      StarDensity {};
    f64      StarSize {};
    f64      StarTwinkle {};
    f64      CloudCoverage {};
    f64      CloudScale {};
    f64      CloudSoftness {};
    point_d  CloudWind {};
    color_d  CloudColor {};
    color_d  Zenith;
    color_d  Horizon;
    color_d  SunColor;
    color_d  DuskColor;

    std::vector<sky_body_frame> Bodies;
};

struct sky_body {
    radian_d Azimuth {0.0};    // radians at time 0
    radian_d Elevation {0.5};  // radians
    radian_d Radius {0.05};    // angular radius in radians
    radian_d OrbitSpeed {0.0}; // radians per second added to the azimuth
    f64      Ambient {0.06};   // brightness of the unlit side
    bool     Lit {true};       // false = emissive, ignores the sun
    color_d  Color {.R = 0.9, .G = 0.9, .B = 0.85};
    f64      Noise {0.0};      // 0 = plain disc, otherwise how much NoiseColor shows through
    f64      NoiseScale {4.0};
    f64      NoiseSeed {0.0};  // offsets the pattern, so two bodies don't look the same
    color_d  NoiseColor {.R = 0.7, .G = 0.7, .B = 0.65};
};

struct sky_settings {
    f64 TimeOfDay {0.5};             // 0..1, 0 = midnight, 0.5 = noon
    f64 DayLength {0.0};             // seconds per full day, 0 = frozen

    radian_d SunSize {0.04};         // angular radius in radians
    f64      SunTilt {0.3};          // sun path leans toward +Y

    f64 StarDensity {0.01};          // chance per star cell, 0 = no stars
    f64 StarSize {1.0};              // star size in screen pixels; below 1 stays one pixel but fainter
    f64 StarTwinkle {0.5};

    f64     CloudCoverage {0.0};     // 0 = no clouds, 1 = overcast
    f64     CloudScale {2.0};        // larger = smaller clouds
    f64     CloudSoftness {0.12};    // edge softness
    point_d CloudWind {0.01, 0.005}; // noise units per second
    color_d CloudColor {.R = 1.0, .G = 1.0, .B = 1.0};

    color_d DayZenith {.R = 0.25, .G = 0.5, .B = 0.95};
    color_d DayHorizon {.R = 0.65, .G = 0.8, .B = 1.0};
    color_d NightZenith {.R = 0.01, .G = 0.01, .B = 0.05};
    color_d NightHorizon {.R = 0.04, .G = 0.04, .B = 0.12};
    color_d DuskColor {.R = 1.0, .G = 0.45, .B = 0.2};
    color_d SunColor {.R = 1.0, .G = 0.95, .B = 0.8};

    std::vector<sky_body> Bodies {};
};

// The sky: sun, moons, stars and clouds for the current time of day. `update` turns the settings into the frame the renderer draws.
class sky {
public:
    void set(sky_settings const& settings);
    void update(milliseconds time);

    auto pixel(shade_view const& view, i32 skyY, point_d const& rayDir) const -> u32;

private:
    auto frame() const -> sky_frame const&;

    sky_settings _settings;
    sky_frame    _frame;
};
