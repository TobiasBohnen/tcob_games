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

constexpr static i32 HAZE_LAYERS {5};
constexpr static f64 STAR_DENSITY {0.004};
constexpr static f64 LINE_PIXELS {1.2}; // width of a crack or a grid line on screen
constexpr static f64 TEXTURE_SIZE {64.0};

namespace {

// one screen row of a ground: what the styles need to know about the ray that hits the floor there
struct ground_ray {
    ground_settings const& Style;
    shade_view const&      View;
    f64                    T {};       // ray distance to the floor of the cell in this row, so the pattern follows slopes
    f64                    LowestZ {}; // lowest point of the cell
    f64                    Slope {};   // of the ray, positive below the horizon
    point_d                RayDir;
    point_d                Pos;        // where the ray meets the floor

    // a pattern sampled on a plane, on the same texel grid as the floor textures, drifting with time
    auto pattern(point_d at, f64 scale, f64 dirX, f64 dirY, f64 offset) const -> f64
    {
        point_d const texel {std::floor(at.X * TEXTURE_SIZE) / TEXTURE_SIZE, std::floor(at.Y * TEXTURE_SIZE) / TEXTURE_SIZE};
        f64 const     x {(texel.X * scale) + (View.Time * Style.Speed * dirX) + offset};
        f64 const     y {(texel.Y * scale) + (View.Time * Style.Speed * dirY) + offset};
        return std::clamp(static_cast<f64>((*View.Noise)({static_cast<f32>(x), static_cast<f32>(y)})), 0.0, 1.0);
    }

    // the farther away, the more of the haze
    auto fade(color_d const& c, f64 dist) const -> color_d { return c.lerp(Style.Haze, 1.0 - std::exp(-dist / Style.Fade)); }

    // ray distance to the horizontal plane at height z, empty if the plane is not in front of the eye
    auto reach(f64 z) const -> std::optional<f64>
    {
        f64 const drop {View.EyeZ - z};
        if (drop <= 0.0 || Slope <= 1e-6) { return std::nullopt; }
        return drop / Slope;
    }

    // the view direction
    auto direction() const -> point3_d { return point3_d::Lift(RayDir, -Slope).normalized(); }

    // lines keep a pixel's width on screen
    auto line_width() const -> f64 { return std::max(0.015, LINE_PIXELS * T / View.ProjPlaneDist); }
};

// two layers of drifting noise with bright veins
auto WaterColor(ground_ray const& r) -> color_d
{
    auto const& style {r.Style};

    f64 const coarse {r.pattern(r.Pos, style.Scale, 1.0, 0.7, 0.0)};
    f64 const fine {r.pattern(r.Pos, style.Scale * 2.3, -1.3, 1.1, 7.3)};
    f64 const mixed {(0.6 * coarse) + (0.4 * fine)};
    f64 const vein {std::pow(1.0 - std::abs((2.0 * mixed) - 1.0), 4.0)};

    return r.fade((style.Deep * (0.6 + (0.8 * mixed))) + (style.Light * (vein * 0.5)), r.T);
}

// soft layers of mist from the floor down, each drifting its own way. The top layer is the floor itself, the others are level
// planes under the lowest point of the cell; below the last one it is the dark of Deep
auto HazeColor(ground_ray const& r) -> color_d
{
    auto const& style {r.Style};

    color_d color {style.Deep};
    for (i32 i {HAZE_LAYERS - 1}; i >= 0; --i) {
        std::optional<f64> const layerT {i == 0 ? std::optional {r.T} : r.reach(r.LowestZ - (static_cast<f64>(i) * style.Depth))};
        if (!layerT) { continue; }

        point_d const where {r.View.Position + (r.RayDir * *layerT)};
        f64 const     sign {i % 2 == 0 ? 1.0 : -1.0};
        f64 const     n {r.pattern(where, style.Scale * 0.3, sign, 0.6 * sign, 11.7 * i)};
        f64 const     shade {1.0 - (0.65 * static_cast<f64>(i) / (HAZE_LAYERS - 1))};
        f64 const     alpha {0.85 * SmoothStep(0.25, 0.75, n)};

        color = color.lerp(r.fade(style.Haze.lerp(style.Light, n) * shade, *layerT), alpha);
    }
    return color;
}

auto IceColor(ground_ray const& r) -> color_d
{
    constexpr static f64 THICKNESS {0.6}; // how deep the cracks below the surface lie

    auto const& style {r.Style};
    f64 const   lineWidth {r.line_width()};

    // a thin line where the noise crosses its middle value; the noise slope is about 1.5 per feature
    auto const crack {[&](point_d at, f64 scale, f64 offset, f64 blur) -> f64 {
        f64 const n {r.pattern(at, scale, 0.0, 0.0, offset)};
        return 1.0 - SmoothStep(0.0, lineWidth * blur, std::abs(n - 0.5) / (1.5 * scale));
    }};

    f64 const cloud {r.pattern(r.Pos, style.Scale * 0.35, 0.0, 0.0, 3.1)};
    f64 const frost {SmoothStep(0.55, 0.8, r.pattern(r.Pos, style.Scale * 0.9, 0.0, 0.0, 9.3))};

    // cracks deeper in the ice: the same pattern where the ray is THICKNESS lower, so they shift against the surface as the view moves
    point_d const below {r.Pos + (r.RayDir * (THICKNESS / std::max(r.Slope, 0.05)))};
    f64 const     deepCracks {std::max(crack(below, style.Scale * 0.5, 5.7, 2.5), 0.6 * crack(below, style.Scale * 1.3, 8.9, 2.5))};
    f64 const     surfaceCracks {std::max(crack(r.Pos, style.Scale * 0.5, 1.3, 1.0), 0.5 * crack(r.Pos, style.Scale * 1.3, 4.1, 1.0))};

    color_d color {style.Deep.lerp(style.Light, 0.2 * cloud)};
    color = color.lerp(style.Light, 0.45 * deepCracks * (1.0 - frost));
    color = color.lerp(style.Light, 0.55 * frost);
    color = color.lerp(style.Light, 0.9 * surfaceCracks);
    return r.fade(color, r.T);
}

// lines every 1/Scale, every fourth one stronger
auto GridColor(ground_ray const& r) -> color_d
{
    auto const& style {r.Style};

    f64 const u {(r.Pos.X * style.Scale) + (r.View.Time * style.Speed)};
    f64 const v {r.Pos.Y * style.Scale};
    f64 const lineWidth {r.line_width()};

    // world distance to the nearest line
    auto const line {[&](f64 coord) {
        f64 const  rounded {std::round(coord)};
        f64 const  d {std::abs(coord - rounded) / style.Scale};
        bool const major {(static_cast<i32>(rounded) & 3) == 0};
        f64 const  sharp {1.0 - SmoothStep(0.0, lineWidth * (major ? 1.6 : 1.0), d)};
        f64 const  glow {0.25 * std::exp(-d / 0.08)};
        return (sharp + glow) * (major ? 1.0 : 0.45);
    }};

    f64 const lines {std::min(std::max(line(u), line(v)), 1.0)};
    return r.fade(style.Deep.lerp(style.Light, lines), r.T);
}

// a starfield: cells of two screen pixels in (azimuth, sin elevation)
auto VoidColor(ground_ray const& r) -> color_d
{
    auto const& style {r.Style};

    f64 const dirZ {r.direction().Z};
    color_d   color {style.Haze.lerp(style.Deep, std::clamp(-dirZ, 0.0, 1.0))};

    f64 const cellsPerUnit {r.View.ProjPlaneDist / 2.0};
    i32 const gx {static_cast<i32>(std::floor((std::atan2(r.RayDir.Y, r.RayDir.X) + std::numbers::pi) * cellsPerUnit))};
    i32 const gy {static_cast<i32>(std::floor(dirZ * cellsPerUnit))};

    if (HashUnit(gx, gy, 0, 41) < STAR_DENSITY) {
        f64 const phase {HashUnit(gx, gy, 0, 42) * TAU};
        f64 const pulse {0.5 + (0.5 * std::sin((r.View.Time * style.Speed * 20.0) + phase))};
        color += style.Light * ((0.4 + (0.6 * HashUnit(gx, gy, 0, 43))) * (1.0 - (0.5 * pulse)));
    }
    return color;
}

}

////////////////////////////////////////////////////////////

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

auto ground::pixel(usize id, shade_view const& view, f64 t, f64 lowestZ, i32 y, point_d const& rayDir) const -> color_d
{
    auto const& style {at(id)};

    ground_ray const ray {.Style   = style,
                          .View    = view,
                          .T       = t,
                          .LowestZ = lowestZ,
                          .Slope   = (static_cast<f64>(y) - view.CenterY) / view.ProjPlaneDist,
                          .RayDir  = rayDir,
                          .Pos     = view.Position + (rayDir * t)};

    switch (style.Style) {
    case ground_style::Water: return WaterColor(ray);
    case ground_style::Haze:  return HazeColor(ray);
    case ground_style::Ice:   return IceColor(ray);
    case ground_style::Grid:  return GridColor(ray);
    case ground_style::Void:  return VoidColor(ray);
    }
    return style.Haze;
}
