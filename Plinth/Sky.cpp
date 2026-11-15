// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Sky.hpp"

#include <algorithm>
#include <cmath>

#include "Common.hpp"

void sky::set(sky_settings const& settings) { _settings = settings; }

void sky::update(milliseconds time)
{
    auto const& sky {_settings};

    f64 const t {static_cast<f64>(time.count()) / 1000.0};

    auto const direction {[](radian_d azimuth, radian_d elevation) -> point3_d {
        return {.X = elevation.cos() * azimuth.cos(), .Y = elevation.cos() * azimuth.sin(), .Z = elevation.sin()};
    }};

    f64 const days {sky.TimeOfDay + (sky.DayLength > 0.0 ? t / sky.DayLength : 0.0)};
    f64 const timeOfDay {days - std::floor(days)};

    // the sun rises at +X, peaks at noon and sets at -X
    f64 const sunAngle {TAU * (timeOfDay - 0.25)};
    f64 const sx {std::cos(sunAngle)};
    f64 const sy {sky.SunTilt * std::sin(sunAngle)};
    f64 const sz {std::sin(sunAngle)};
    f64 const sunInv {1.0 / std::sqrt((sx * sx) + (sy * sy) + (sz * sz))};
    _frame.Sun = {.X = sx * sunInv, .Y = sy * sunInv, .Z = sz * sunInv};

    f64 const u {std::clamp((_frame.Sun.Z + 0.12) / 0.37, 0.0, 1.0)};
    _frame.Day           = u * u * (3.0 - (2.0 * u));
    _frame.Night         = 1.0 - _frame.Day;
    _frame.Dusk          = std::clamp(1.0 - (std::abs(_frame.Sun.Z) / 0.3), 0.0, 1.0);
    _frame.CosSun        = sky.SunSize.cos();
    _frame.Time          = t;
    _frame.Stars         = sky.StarDensity > 0.0 && _frame.Night > 0.02;
    _frame.StarDensity   = sky.StarDensity;
    _frame.StarSize      = sky.StarSize;
    _frame.StarTwinkle   = sky.StarTwinkle;
    _frame.Zenith        = sky.NightZenith.lerp(sky.DayZenith, _frame.Day);
    _frame.Horizon       = sky.NightHorizon.lerp(sky.DayHorizon, _frame.Day);
    _frame.SunColor      = sky.SunColor;
    _frame.DuskColor     = sky.DuskColor;
    _frame.CloudCoverage = sky.CloudCoverage;
    _frame.CloudScale    = sky.CloudScale;
    _frame.CloudSoftness = sky.CloudSoftness;
    _frame.CloudWind     = sky.CloudWind;
    _frame.CloudColor    = sky.CloudColor;

    _frame.Bodies.clear();
    for (auto const& body : sky.Bodies) {
        auto const dir {direction(body.Azimuth + (body.OrbitSpeed * t), body.Elevation)};
        _frame.Bodies.push_back({.Dir        = dir,
                                 .CosR       = body.Radius.cos(),
                                 .SinR       = body.Radius.sin(),
                                 .SunDot     = dir.dot(_frame.Sun),
                                 .Ambient    = body.Ambient,
                                 .Lit        = body.Lit,
                                 .Color      = body.Color,
                                 .Noise      = body.Noise,
                                 .NoiseScale = body.NoiseScale,
                                 .NoiseSeed  = body.NoiseSeed,
                                 .NoiseColor = body.NoiseColor});
    }
}

auto sky::frame() const -> sky_frame const& { return _frame; }
