// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Level.hpp"

#include "Common.hpp"

level::level(map_t map)
    : _map {map}
{
    Settings.CeilingTexture = 13;
    Settings.FloorTexture   = 9;
    Settings.IsSkybox       = false;
}

auto level::update(milliseconds deltaSeconds) -> bool
{
    bool retValue {false};

    f64 const dt {deltaSeconds.count() / 1000};
    for (auto& cell : _map) {
        std::visit(
            [dt, &retValue](auto&& w) {
                if constexpr (requires { w.update(dt); }) {
                    retValue = w.update(dt) || retValue;
                }
            },
            cell);
    }
    return retValue;
}

auto level::get_cell(point_i p) const -> cell const&
{
    return _map[p];
}

void level::toggle_wall(point_i p)
{
    std::visit(
        [](auto&& w) {
            if constexpr (requires { w.toggle(); }) {
                w.toggle();
            }
        },
        _map[p]);
}

auto level::is_seen(point_i cell) const -> bool
{
    if (!map_t::Size.contains(cell)) { return false; }
    return _seen[cell];
}

void level::mark_seen(point_i cell, point_d playerPos)
{
    if (!map_t::Size.contains(cell)) { return; }

    point_d const cellCenter {cell.X + 0.5, cell.Y + 0.5};
    point_d const delta {cellCenter.X - playerPos.X, cellCenter.Y - playerPos.Y};
    f64 const     dist {std::sqrt((delta.X * delta.X) + (delta.Y * delta.Y))};

    f64 const visibleRange {Settings.FogDistance * (1.0 - Settings.FogMin)};
    if (dist < visibleRange) {
        _seen[cell] = true;
    }
}
