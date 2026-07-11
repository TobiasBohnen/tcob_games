// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "MapRenderer.hpp"

#include "Common.hpp"
#include "Level.hpp"
#include "Player.hpp"
#include "TextureCache.hpp"
#include "Walls.hpp"

static auto get_color(level const& level, point_i map) -> color
{
    if (!level.is_seen(map)) { return colors::Black; };

    return overloaded_visit(
        level.get_cell(map),
        [](floor_cell const&) -> color { return colors::Silver; },
        [](door_wall const&) -> color { return colors::Blue; },
        [](push_wall const&) -> color { return colors::DimGray; },
        [](normal_wall const&) -> color { return colors::DimGray; },
        [](box_wall const& w) -> color { return colors::DimGray; },
        [](diagonal_wall const& w) -> color { return colors::DimGray; },
        [](round_pillar const& w) -> color { return colors::DimGray; });
}

map_renderer::map_renderer(texture_cache& cache, size_i screenSize)
    : _cache {cache}
    , _screen(screenSize.area())
    , _screenSize {screenSize}
{
}

auto map_renderer::draw(level const& level, player const& player) -> u32 const*
{
    // walls
    _screen.clear();
    f32 const cellSize {_screenSize.Height / static_cast<f32>(MAP_HEIGHT)};
    for (i32 y {0}; y < _screenSize.Height; ++y) {
        for (i32 x {0}; x < _screenSize.Height; ++x) {
            point_i const mapPos {static_cast<i32>(x / cellSize), static_cast<i32>(y / cellSize)};
            i32 const     idx {x + (y * _screenSize.Width)};
            _screen[idx] = get_color(level, mapPos).to_abgr();
        }
    }

    auto const plot {[&](point_i const& pt, u32 color) {
        if (pt.X >= 0 && pt.X < _screenSize.Width && pt.Y >= 0 && pt.Y < _screenSize.Height) {
            i32 const idx {static_cast<i32>(pt.X + (pt.Y * _screenSize.Width))};
            _screen[idx] = color;
        }
    }};

    // player
    point_i const playerPos {player.Position * cellSize};
    u32 const     triColor {colors::Red.to_abgr()};
    f64 const     triLength {cellSize};
    f64 const     triWidth {triLength * 0.5};
    point_d const dir {player.Direction};
    point_d const perp {-dir.Y, dir.X};

    point_i tip {playerPos + point_i {static_cast<i32>(dir.X * triLength), static_cast<i32>(dir.Y * triLength)}};
    point_i baseLeft {playerPos + point_i {static_cast<i32>((-dir.X * triLength * 0.3) + (perp.X * triWidth)), static_cast<i32>((-dir.Y * triLength * 0.3) + (perp.Y * triWidth))}};
    point_i baseRight {playerPos + point_i {static_cast<i32>((-dir.X * triLength * 0.3) - (perp.X * triWidth)), static_cast<i32>((-dir.Y * triLength * 0.3) - (perp.Y * triWidth))}};

    if (tip.Y > baseLeft.Y) { std::swap(tip, baseLeft); }
    if (baseLeft.Y > baseRight.Y) { std::swap(baseLeft, baseRight); }
    if (tip.Y > baseLeft.Y) { std::swap(tip, baseLeft); }

    auto const edgeInterpX {[](point_i const& a, point_i const& b, i32 y) -> f64 {
        if (a.Y == b.Y) { return a.X; }
        f64 const t {static_cast<f64>(y - a.Y) / static_cast<f64>(b.Y - a.Y)};
        return a.X + (t * (b.X - a.X));
    }};

    for (i32 y {tip.Y}; y <= baseRight.Y; ++y) {
        bool const secondHalf {y > baseLeft.Y || tip.Y == baseLeft.Y};

        f64 const xa {edgeInterpX(tip, baseRight, y)};
        f64 const xb {secondHalf ? edgeInterpX(baseLeft, baseRight, y) : edgeInterpX(tip, baseLeft, y)};

        i32 const xStart {static_cast<i32>(std::round(std::min(xa, xb)))};
        i32 const xEnd {static_cast<i32>(std::round(std::max(xa, xb)))};

        for (i32 x {xStart}; x <= xEnd; ++x) {
            plot(point_i {x, y}, triColor);
        }
    }

    // sprites
    for (auto const& spr : level.Sprites) {
        if (!level.is_seen(point_i {spr.Position})) { continue; } // TODO: sprite map visibility and color

        point_i const sprPos {spr.Position * cellSize};
        i32 const     radius {static_cast<i32>(cellSize / 2)};
        bresenham_circle(sprPos, radius, [&](point_i const& pt) { plot(pt, colors::Green.to_abgr()); });
    }

    return _screen.data();
}
