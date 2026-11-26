// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Ground.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <optional>

#include "Common.hpp"
#include "Shading.hpp"

auto ground::at(usize id) const -> ground_settings const&
{
    static ground_settings const none {};
    return id < _settings.size() ? _settings[id] : none;
}

void ground::set(usize id, ground_settings const& settings)
{
    if (id >= _settings.size()) { _settings.resize(id + 1); }
    _settings[id] = settings;
}

auto ground::pixel(usize id, shade_view const& view, f64 t, f64 lowestZ, i32 y, point_d const& rayDir) const -> u32
{
    auto const& style {at(id)};

    constexpr static i32 HAZE_LAYERS {5};
    constexpr static f64 STAR_DENSITY {0.004};
    constexpr static f64 LINE_PIXELS {1.2}; // width of a crack or a grid line on screen

    auto const noise {[&view](f64 x, f64 z) -> f64 { return (*view.Noise)({static_cast<f32>(x), static_cast<f32>(z)}); }};

    f64 const time {view.Time};

    // t is the ray distance to the floor of the cell in this row, so the pattern follows slopes
    point_d const pos {view.Position + (rayDir * t)};

    // slope of the ray, positive below the horizon
    f64 const  s {(static_cast<f64>(y) - view.CenterY) / view.ProjPlaneDist};
    auto const dir {point3_d::Lift(rayDir, -s).normalized()};

    // ray distance to the horizontal plane at height z, empty if the plane is not in front of the eye
    auto const reach {[&](f64 z) -> std::optional<f64> {
        f64 const drop {view.EyeZ - z};
        if (drop <= 0.0 || s <= 1e-6) { return std::nullopt; }
        return drop / s;
    }};

    // a pattern sampled on a plane, drifting with time
    auto const pattern {[&](point_d at, f64 scale, f64 dirX, f64 dirY, f64 offset) -> f64 {
        return std::clamp(noise((at.X * scale) + (time * style.Speed * dirX) + offset, (at.Y * scale) + (time * style.Speed * dirY) + offset), 0.0, 1.0);
    }};

    // the farther away, the more of the haze
    auto const fade {[&](color_d const& c, f64 dist) { return c.lerp(style.Haze, 1.0 - std::exp(-dist / style.Fade)); }};

    // one world unit of ray distance is about this many world units across on screen: lines keep a pixel's width
    f64 const lineWidth {std::max(0.015, LINE_PIXELS * t / view.ProjPlaneDist)};

    color_d color {style.Haze};

    switch (style.Style) {
    case ground_style::Water: {
        // two layers of drifting noise, with bright veins where the layers meet
        f64 const coarse {pattern(pos, style.Scale, 1.0, 0.7, 0.0)};
        f64 const fine {pattern(pos, style.Scale * 2.3, -1.3, 1.1, 7.3)};
        f64 const mixed {(0.6 * coarse) + (0.4 * fine)};
        f64 const vein {std::pow(1.0 - std::abs((2.0 * mixed) - 1.0), 4.0)};

        color = fade((style.Deep * (0.6 + (0.8 * mixed))) + (style.Light * (vein * 0.5)), t);
        break;
    }
    case ground_style::Haze: {
        // soft layers of mist from the floor down, each drifting its own way; below the last one it is the dark of Deep.
        // The top layer is the floor itself, the others are level planes under the lowest point of the cell
        color = style.Deep;
        for (i32 i {HAZE_LAYERS - 1}; i >= 0; --i) {
            std::optional<f64> const layerT {i == 0 ? std::optional {t} : reach(lowestZ - (static_cast<f64>(i) * style.Depth))};
            if (!layerT) { continue; }

            point_d const where {view.Position + (rayDir * *layerT)};
            f64 const     sign {i % 2 == 0 ? 1.0 : -1.0};
            f64 const     n {pattern(where, style.Scale * 0.3, sign, 0.6 * sign, 11.7 * i)};
            f64 const     shade {1.0 - (0.65 * static_cast<f64>(i) / (HAZE_LAYERS - 1))};
            f64 const     alpha {0.85 * SmoothStep(0.25, 0.75, n)};

            color = color.lerp(fade(style.Haze.lerp(style.Light, n) * shade, *layerT), alpha);
        }
        break;
    }
    case ground_style::Ice: {
        constexpr static f64 THICKNESS {0.6}; // how deep the cracks below the surface lie

        // a thin line where the noise crosses its middle value; the noise slope is about 1.5 per feature
        auto const crack {[&](point_d at, f64 scale, f64 offset, f64 blur) -> f64 {
            f64 const n {pattern(at, scale, 0.0, 0.0, offset)};
            return 1.0 - SmoothStep(0.0, lineWidth * blur, std::abs(n - 0.5) / (1.5 * scale));
        }};

        f64 const cloud {pattern(pos, style.Scale * 0.35, 0.0, 0.0, 3.1)};
        f64 const frost {SmoothStep(0.55, 0.8, pattern(pos, style.Scale * 0.9, 0.0, 0.0, 9.3))};

        // cracks deeper in the ice: the same pattern where the ray is THICKNESS lower, so they shift against the surface
        // as the view moves
        point_d const below {pos + (rayDir * (THICKNESS / std::max(s, 0.05)))};
        f64 const     deepCracks {std::max(crack(below, style.Scale * 0.5, 5.7, 2.5), 0.6 * crack(below, style.Scale * 1.3, 8.9, 2.5))};
        f64 const     surfaceCracks {std::max(crack(pos, style.Scale * 0.5, 1.3, 1.0), 0.5 * crack(pos, style.Scale * 1.3, 4.1, 1.0))};

        color = style.Deep.lerp(style.Light, 0.2 * cloud);
        color = color.lerp(style.Light, 0.45 * deepCracks * (1.0 - frost));
        color = color.lerp(style.Light, 0.55 * frost);
        color = color.lerp(style.Light, 0.9 * surfaceCracks);
        color = fade(color, t);
        break;
    }
    case ground_style::Grid: {
        f64 const u {(pos.X * style.Scale) + (time * style.Speed)};
        f64 const v {pos.Y * style.Scale};
        f64 const ru {std::round(u)};
        f64 const rv {std::round(v)};

        // world distance to the nearest line; every fourth line is a major one
        auto const line {[&](f64 delta, f64 rounded) {
            f64 const  d {std::abs(delta) / style.Scale};
            bool const major {(static_cast<i32>(rounded) & 3) == 0};
            f64 const  sharp {1.0 - SmoothStep(0.0, lineWidth * (major ? 1.6 : 1.0), d)};
            f64 const  glow {0.25 * std::exp(-d / 0.08)};
            return (sharp + glow) * (major ? 1.0 : 0.45);
        }};

        f64 const lines {std::min(std::max(line(u - ru, ru), line(v - rv, rv)), 1.0)};

        color = fade(style.Deep.lerp(style.Light, lines), t);
        break;
    }
    case ground_style::Void: {
        color = style.Haze.lerp(style.Deep, std::clamp(-dir.Z, 0.0, 1.0));

        // stars: cells of two screen pixels in (azimuth, sin elevation)
        f64 const cellsPerUnit {view.ProjPlaneDist / 2.0};
        i32 const gx {static_cast<i32>(std::floor((std::atan2(rayDir.Y, rayDir.X) + std::numbers::pi) * cellsPerUnit))};
        i32 const gy {static_cast<i32>(std::floor(dir.Z * cellsPerUnit))};
        if (HashUnit(gx, gy, 0, 41) < STAR_DENSITY) {
            f64 const phase {HashUnit(gx, gy, 0, 42) * TAU};
            f64 const pulse {0.5 + (0.5 * std::sin((time * style.Speed * 20.0) + phase))};
            color += style.Light * ((0.4 + (0.6 * HashUnit(gx, gy, 0, 43))) * (1.0 - (0.5 * pulse)));
        }
        break;
    }
    }

    return ApplyTint(255, 255, 255, color);
}
