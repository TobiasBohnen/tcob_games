// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "MapRenderer.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

#include "Common.hpp"
#include "Level.hpp"
#include "Player.hpp"
#include "Walls.hpp"

static auto get_color(level const& level, point_i map) -> color
{
    auto const state {level.get_visibility(map)};
    if (state == level::tile_visibility::Hidden) { return colors::Black; }
    color c {overloaded_visit(
        level.get_cell(map),
        [](floor_cell const&) -> color { return colors::Silver; },
        [](door_wall const&) -> color { return colors::Blue; },
        [](push_wall const&) -> color { return colors::DimGray; },
        [](normal_wall const&) -> color { return colors::DimGray; },
        [](obstacle const&) -> color { return colors::CadetBlue; },
        [](diagonal_wall const&) -> color { return colors::DimGray; })};

    if (state == level::tile_visibility::Visible) { return c; }

    return {
        static_cast<u8>((c.R * 70) / 100),
        static_cast<u8>((c.G * 70) / 100),
        static_cast<u8>((c.B * 70) / 100)};
}

map_renderer::map_renderer(size_i screenSize)
    : _screen(screenSize.area())
    , _screenSize {screenSize}
{
}

auto map_renderer::draw(level const& level, player const& player) -> u32 const*
{
    _screen.clear();
    f32 const cellSize {_screenSize.Height / static_cast<f32>(MAP_HEIGHT)};

    for (i32 mapY {0}; mapY < MAP_HEIGHT; ++mapY) {
        i32 const yStart {std::clamp(static_cast<i32>(mapY * cellSize), 0, _screenSize.Height)};
        i32 const yEnd {std::clamp(static_cast<i32>((mapY + 1) * cellSize), 0, _screenSize.Height)};
        if (yStart >= yEnd) { continue; }

        for (i32 mapX {0}; mapX < MAP_WIDTH; ++mapX) {
            i32 const xStart {std::clamp(static_cast<i32>(mapX * cellSize), 0, _screenSize.Width)};
            i32 const xEnd {std::clamp(static_cast<i32>((mapX + 1) * cellSize), 0, _screenSize.Width)};
            if (xStart >= xEnd) { continue; }

            u32 const color {get_color(level, point_i {mapX, mapY}).to_abgr()};

            for (i32 y {yStart}; y < yEnd; ++y) {
                u32* rowPtr {&_screen[xStart + (y * _screenSize.Width)]};
                std::fill_n(rowPtr, xEnd - xStart, color);
            }
        }
    }

    auto const plot {[&](point_i pt, u32 color) {
        if (pt.X >= 0 && pt.X < _screenSize.Width && pt.Y >= 0 && pt.Y < _screenSize.Height) {
            i32 const idx {pt.X + (pt.Y * _screenSize.Width)};
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

    auto const edge_interp_x {[](point_i a, point_i b, i32 y) -> f64 {
        if (a.Y == b.Y) { return a.X; }
        f64 const t {static_cast<f64>(y - a.Y) / static_cast<f64>(b.Y - a.Y)};
        return a.X + (t * (b.X - a.X));
    }};

    for (i32 y {tip.Y}; y <= baseRight.Y; ++y) {
        bool const secondHalf {y > baseLeft.Y || tip.Y == baseLeft.Y};

        f64 const xa {edge_interp_x(tip, baseRight, y)};
        f64 const xb {secondHalf ? edge_interp_x(baseLeft, baseRight, y) : edge_interp_x(tip, baseLeft, y)};

        i32 const xStart {static_cast<i32>(std::round(std::min(xa, xb)))};
        i32 const xEnd {static_cast<i32>(std::round(std::max(xa, xb)))};

        for (i32 x {xStart}; x <= xEnd; ++x) {
            plot(point_i {x, y}, triColor);
        }
    }

    // objects
    for (auto const& obj : level.objects()) {
        if (!obj.is_seen()) { continue; }

        point_i const sprPos {obj.position() * cellSize};
        i32 const     radius {static_cast<i32>(cellSize / 2)};
        bresenham_circle(sprPos, radius, [&](point_i pt) { plot(pt, obj.map_marker().to_abgr()); });
    }

    return _screen.data();
}
