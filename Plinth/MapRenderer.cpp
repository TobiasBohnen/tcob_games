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
    if (!level.Seen[map]) { return colors::Black; };

    return overloaded_visit(
        level.Map[map],
        [](empty_cell const&) -> color { return colors::Gray; },
        [](cell const&) -> color { return colors::Gray; },
        [&](door_wall const&) -> color { return colors::Blue; },
        [&](push_wall const&) -> color { return colors::White; },
        [&](normal_wall const&) -> color { return colors::White; },
        [&](box_wall const& w) -> color { return colors::White; },
        [&](diagonal_wall const& w) -> color { return colors::White; },
        [&](round_pillar const& w) -> color { return colors::White; });
}

map_renderer::map_renderer(texture_cache& cache, size_i screenSize)
    : _cache {cache}
    , _screen(screenSize.area())
    , _screenSize {screenSize}
{
}

auto map_renderer::draw(level const& level, player const& player) -> u32 const*
{
    _screen.clear();

    f32 const tileSize {_screenSize.Height / static_cast<f32>(level.Map.height())};

    for (i32 y {0}; y < _screenSize.Height; ++y) {
        for (i32 x {0}; x < _screenSize.Height; ++x) {
            point_i const mapPos {static_cast<i32>(x / tileSize), static_cast<i32>(y / tileSize)};
            i32 const     idx {(_screenSize.Width - x - 1) + (y * _screenSize.Width)};
            _screen[idx] = get_color(level, mapPos).to_abgr();
        }
    }

    point_i const playerPos {player.Position * tileSize};
    i32 const     radius {static_cast<i32>(tileSize / 4)};

    bresenham_circle(playerPos, radius, [&](point_i const& pt) {
        i32 const screenX {_screenSize.Width - pt.X - 1};
        i32 const screenY {pt.Y};
        if (screenX >= 0 && screenX < _screenSize.Width && screenY >= 0 && screenY < _screenSize.Height) {
            i32 const idx {static_cast<i32>(screenX + (screenY * _screenSize.Width))};
            _screen[idx] = colors::Green.to_abgr();
        }
    });

    return _screen.data();
}