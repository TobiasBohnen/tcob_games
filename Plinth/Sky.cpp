// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Sky.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

#include "Common.hpp"
#include "Shading.hpp"

void sky::set(sky_settings const& settings)
{
    _settings = settings;
    _frame.Bodies.clear(); // rebuilt by the next update, so it never lags behind the bodies of the settings
}

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

    _frame.Day     = SmoothStep(-0.12, 0.25, _frame.Sun.Z);
    _frame.Night   = 1.0 - _frame.Day;
    _frame.Dusk    = std::clamp(1.0 - (std::abs(_frame.Sun.Z) / 0.3), 0.0, 1.0);
    _frame.CosSun  = sky.SunSize.cos();
    _frame.Time    = t;
    _frame.Stars   = sky.StarDensity > 0.0 && _frame.Night > 0.02;
    _frame.Zenith  = sky.NightZenith.lerp(sky.DayZenith, _frame.Day);
    _frame.Horizon = sky.NightHorizon.lerp(sky.DayHorizon, _frame.Day);

    _frame.Bodies.clear();
    for (auto const& body : sky.Bodies) {
        auto const dir {direction(body.Azimuth + (body.OrbitSpeed * t), body.Elevation)};
        _frame.Bodies.push_back({.Dir = dir, .CosR = body.Radius.cos(), .SinR = body.Radius.sin(), .SunDot = dir.dot(_frame.Sun)});
    }
}

auto sky::pixel(shade_view const& view, i32 skyY, point_d const& rayDir) const -> u32
{
    static constexpr point3_d WorldUp {.Z = 1.0};

    // two octaves of noise: the shape octave and a detail octave at twice the frequency
    auto const noise {[&view](f64 x, f64 y) -> f64 { return (*view.Noise)({static_cast<f32>(x), static_cast<f32>(y)}); }};
    auto const detail {[&](f64 x, f64 y) -> f64 { return noise((x * 2.0) + 7.31, (y * 2.0) + 3.17); }};

    auto const& fr {_frame};
    auto const& cfg {_settings};

    // view direction; the horizon is the unbobbed screen center
    f64 const  zRow {((view.ScreenHeight / 2.0) - skyY) / view.ProjPlaneDist};
    auto const dir {point3_d::Lift(rayDir, zRow).normalized()};

    // gradient
    f64 const h {std::clamp(dir.Z, 0.0, 1.0)};
    auto      color {fr.Horizon.lerp(fr.Zenith, h)};

    // dusk glow, sun halo and sun disc
    f64 const sunDot {dir.dot(fr.Sun)};
    f64 const s {std::max(sunDot, 0.0)};
    f64 const s2 {s * s};
    f64 const s8 {s2 * s2 * s2 * s2};
    f64 const s32 {s8 * s8 * s8 * s8};

    f64 const duskGlow {fr.Dusk * s8 * (1.0 - (0.7 * h))};
    f64 const haloGlow {0.35 * s32 * (0.3 + (0.7 * fr.Day))};
    color += (cfg.DuskColor * duskGlow) + (cfg.SunColor * haloGlow);
    if (sunDot >= fr.CosSun) { color = cfg.SunColor; }

    // stars: cells of StarSize screen pixels in (azimuth, sin elevation)
    if (fr.Stars) {
        f64 const azimuth {std::atan2(rayDir.Y, rayDir.X)};
        f64 const cellsPerUnit {view.ProjPlaneDist / std::max(cfg.StarSize, 1.0)};
        f64 const size {std::min(cfg.StarSize, 1.0)};
        i32 const gx {static_cast<i32>(std::floor((azimuth + std::numbers::pi) * cellsPerUnit))};
        i32 const gy {static_cast<i32>(std::floor(dir.Z * cellsPerUnit))};
        if (HashUnit(gx, gy, 0, 21) < cfg.StarDensity) {
            f64 const speed {1.0 + (2.0 * HashUnit(gx, gy, 0, 22))};
            f64 const phase {HashUnit(gx, gy, 0, 23) * TAU};
            f64 const pulse {0.5 + (0.5 * std::sin((fr.Time * speed) + phase))};
            f64 const base {0.4 + (0.6 * HashUnit(gx, gy, 0, 24))};
            color += size * size * fr.Night * (1.0 - (cfg.StarTwinkle * pulse)) * base;
        }
    }

    // moons and planets
    for (usize i {0}; i < fr.Bodies.size(); ++i) {
        auto const& body {cfg.Bodies[i]};
        auto const& placed {fr.Bodies[i]};

        f64 const d {dir.dot(placed.Dir)};
        if (d < placed.CosR) { continue; }

        f64 const rho2 {std::max(1.0 - (d * d), 0.0) / (placed.SinR * placed.SinR)};
        f64 const nz {std::sqrt(std::max(1.0 - rho2, 0.0))};

        color_d surface {body.Color};
        if (body.Noise > 0.0) {
            // disc coordinates, u to the right and v up, from the body direction and world up
            auto const right {placed.Dir.cross(WorldUp).normalized()};
            auto const up {right.cross(placed.Dir)};
            f64 const  u {dir.dot(right) / placed.SinR};
            f64 const  v {dir.dot(up) / placed.SinR};

            // features compress toward the limb, which makes the disc look round
            f64 const warp {body.NoiseScale / (0.4 + (0.6 * nz))};
            f64 const x {(u * warp) + body.NoiseSeed};
            f64 const y {v * warp};
            f64 const n {((0.5 * noise(x, y)) + (0.25 * detail(x, y))) / 0.75};
            surface = surface.lerp(body.NoiseColor, n * body.Noise);
        }

        if (body.Lit) {
            // phase: surface normal against the direction of the sun
            f64 const nDotL {((sunDot - (d * placed.SunDot)) / placed.SinR) - (placed.SunDot * nz)};
            surface *= std::lerp(body.Ambient, 1.0, std::clamp(nDotL, 0.0, 1.0));
            color = surface.lerp(color, fr.Day);
        } else {
            color = surface;
        }
    }

    // clouds: a flat layer above the viewer, projected along the view direction
    if (cfg.CloudCoverage > 0.0 && dir.Z > 0.03) {
        f64 const edge {1.0 - cfg.CloudCoverage};
        f64 const lo {edge - cfg.CloudSoftness};
        f64 const hi {edge + cfg.CloudSoftness};

        f64 const k {cfg.CloudScale / dir.Z};
        f64 const cx {(dir.X * k) + (cfg.CloudWind.X * fr.Time)};
        f64 const cy {(dir.Y * k) + (cfg.CloudWind.Y * fr.Time)};

        // the first octave carries the shape; if the detail octave at its maximum still stays under the edge, it is clear sky
        f64 const n0 {noise(cx, cy)};
        if (((0.5 * n0) + 0.25) / 0.75 > lo) {
            f64 const density {((0.5 * n0) + (0.25 * detail(cx, cy))) / 0.75};
            f64 const alpha {SmoothStep(lo, hi, density) * SmoothStep(0.03, 0.3, dir.Z)}; // fades out toward the horizon
            if (alpha > 0.0) {
                // the side facing the sun is thinner, so a cloud is lit where it is thicker than its sun-side neighbor
                f64 const toward {noise(cx + (fr.Sun.X * 0.3), cy + (fr.Sun.Y * 0.3))};
                f64 const lit {std::clamp(0.5 + ((n0 - toward) * 3.0), 0.0, 1.0)};

                f64 const  brightness {(0.12 + (0.88 * fr.Day)) * (0.55 + (0.45 * lit))};
                auto const cloud {cfg.CloudColor.lerp(cfg.DuskColor, fr.Dusk * s2) * brightness};
                color = color.lerp(cloud, alpha);
            }
        }
    }

    return ApplyTint(255, 255, 255, color);
}
