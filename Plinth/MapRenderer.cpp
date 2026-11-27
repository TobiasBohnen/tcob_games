// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "MapRenderer.hpp"

#include <algorithm>
#include <cmath>
#include <span>
#include <utility>

#include "Common.hpp"
#include "Level.hpp"
#include "LevelObject.hpp"
#include "Map.hpp"
#include "Player.hpp"
#include "Walls.hpp"

static auto get_color(map const& map, point_i cell) -> color
{
    auto const state {map.get_visibility(cell)};
    if (state == tile_visibility::Hidden) { return colors::Transparent; }
    color c {overloaded_visit(
        map.get_cell(cell),
        [](floor_cell const&) -> color { return colors::Silver; },
        [](door_wall const&) -> color { return colors::Blue; },
        [](push_wall const&) -> color { return colors::DimGray; },
        [](normal_wall const&) -> color { return colors::DimGray; },
        [](obstacle const&) -> color { return colors::CadetBlue; },
        [](diagonal_wall const&) -> color { return colors::DimGray; })};

    if (state == tile_visibility::Visible) { return c; }

    return {
        static_cast<u8>((c.R * 70) / 100),
        static_cast<u8>((c.G * 70) / 100),
        static_cast<u8>((c.B * 70) / 100)};
}

void map_renderer::draw(level const& lvl, player const& player, std::span<u32> screen, size_i screenSize) const
{
    f32 const cellSize {screenSize.Height / static_cast<f32>(MAP_HEIGHT)};

    point_i const origin {static_cast<i32>((static_cast<f32>(screenSize.Width) - (cellSize * MAP_WIDTH)) * 0.5f),
                          static_cast<i32>((static_cast<f32>(screenSize.Height) - (cellSize * MAP_HEIGHT)) * 0.5f)};

    for (i32 mapY {0}; mapY < MAP_HEIGHT; ++mapY) {
        i32 const yStart {std::clamp(origin.Y + static_cast<i32>(mapY * cellSize), 0, screenSize.Height)};
        i32 const yEnd {std::clamp(origin.Y + static_cast<i32>((mapY + 1) * cellSize), 0, screenSize.Height)};
        if (yStart >= yEnd) { continue; }

        for (i32 mapX {0}; mapX < MAP_WIDTH; ++mapX) {
            i32 const xStart {std::clamp(origin.X + static_cast<i32>(mapX * cellSize), 0, screenSize.Width)};
            i32 const xEnd {std::clamp(origin.X + static_cast<i32>((mapX + 1) * cellSize), 0, screenSize.Width)};
            if (xStart >= xEnd) { continue; }

            color const c {get_color(lvl.map(), point_i {mapX, mapY})};
            if (c.A > 0) {
                u32 const color {c.to_abgr()};
                for (i32 y {yStart}; y < yEnd; ++y) {
                    u32* rowPtr {&screen[xStart + (y * screenSize.Width)]};
                    std::fill_n(rowPtr, xEnd - xStart, color);
                }
            } else {
                for (i32 y {yStart}; y < yEnd; ++y) {
                    for (i32 x {xStart}; x < xEnd; ++x) {
                        u32& p {screen[x + (y * screenSize.Width)]};
                        p = 0xFF000000u | ((p >> 1) & 0x007F7F7Fu);
                    }
                }
            }
        }
    }

    auto const plot {[&](point_i pt, u32 color) {
        if (pt.X >= 0 && pt.X < screenSize.Width && pt.Y >= 0 && pt.Y < screenSize.Height) {
            i32 const idx {pt.X + (pt.Y * screenSize.Width)};
            screen[idx] = color;
        }
    }};

    // player
    point_i const playerPos {origin + point_i {player.Position * cellSize}};
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
    for (auto const& o : lvl.objects()) {
        auto const& obj {base_of(o)};
        if (!obj.is_seen()) { continue; }

        point_i const objPos {origin + point_i {obj.position().xy() * cellSize}};
        i32 const     radius {static_cast<i32>(cellSize / 2)};
        bresenham_circle(objPos, radius, [&](point_i pt) { plot(pt, obj.map_marker().to_abgr()); });
    }
}
