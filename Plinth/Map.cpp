// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Map.hpp"

#include <atomic>
#include <cmath>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>

#include "CellWalk.hpp"
#include "Common.hpp"
#include "Walls.hpp"

template <typename T>
static auto BlocksLight(T const& c) -> bool
{
    if constexpr (std::is_same_v<T, floor_cell>) {
        return false;
    } else if constexpr (requires { c.State; }) {
        return c.State != wall_state::Open; // doors/push walls: open = passable
    } else {
        return true;                        // normal_wall, obstacle, diagonal_wall: always solid
    }
}

map::map()
{
    _cellSlopes.resize(static_cast<usize>(MAP_WIDTH) * MAP_HEIGHT);
}

void map::update(f64 dt)
{
    prepare_slopes();

    // fade vision
    for (auto& s : _seen) {
        if (s == tile_visibility::Visible) { s = tile_visibility::Explored; }
    }

    for (auto& cell : _cells) {
        std::visit([&](auto&& w) {
            if constexpr (requires { w.update(dt); }) {
                bool const blocked {BlocksLight(w)};
                w.update(dt);
                if (BlocksLight(w) != blocked) { _lightChanged = true; }
            }
        },
                   cell);
    }
}

auto map::light_changed() -> bool
{
    return std::exchange(_lightChanged, false);
}

auto map::toggle_wall(point_i pos) -> bool
{
    return std::visit(
        [this](auto&& w) {
            if constexpr (requires { w.toggle(); }) {
                w.toggle();
                _lightChanged = true;
                return true;
            } else {
                return false;
            }
        },
        _cells[pos]);
}

auto map::get_visibility(point_i pos) const -> tile_visibility
{
    if (!map_grid::Size.contains(pos)) { return tile_visibility::Hidden; }
    return _seen[pos];
}

void map::mark_visible(point_i pos)
{
    if (!map_grid::Size.contains(pos)) { return; }
    std::atomic_ref<tile_visibility> {_seen[pos]}.store(tile_visibility::Visible, std::memory_order_relaxed);
}

void map::reveal_map()
{
    _seen.fill(tile_visibility::Explored);
}

auto map::get_cell(point_i pos) const -> cell const&
{
    return _cells[pos];
}

void map::set_cell(point_i pos, cell const& c)
{
    _cells[pos]  = c;
    _slopesDirty = true;
}

auto map::surface_at(point_i pos) const -> cell_surface
{
    if (!map_grid::Size.contains(pos)) { return {}; }

    return std::visit([](auto const& c) -> cell_surface {
        if constexpr (requires { c.Surface; }) {
            return c.Surface;
        } else {
            return {};
        }
    },
                      get_cell(pos));
}

auto map::slopes_at(point_i pos) const -> slopes
{
    if (!map_grid::Size.contains(pos)) { return {}; }
    return _cellSlopes[PixelIndex(MAP_WIDTH, pos.X, pos.Y)];
}

void map::prepare_slopes()
{
    if (!_slopesDirty) { return; }
    _slopesDirty = false;

    // height of a vertex as seen from cells of one level: the largest absolute value wins, walls and other levels don't count
    auto const vertex_height {[&](i32 vx, i32 vy, bool isFloor, u8 cellLevel) -> f64 {
        f64  extremeZ {0.0};
        bool found {false};
        for (i32 cy : {vy - 1, vy}) {
            for (i32 cx : {vx - 1, vx}) {
                point_i const c {cx, cy};
                if (!map_grid::Size.contains(c) || std::holds_alternative<normal_wall>(get_cell(c))) { continue; }

                auto const surface {surface_at(c)};
                if (surface.Level != cellLevel) { continue; }

                f64 const z {isFloor ? surface.FloorZ : surface.CeilingZ};
                if (!found || std::abs(z) > std::abs(extremeZ) || (std::abs(z) == std::abs(extremeZ) && z > extremeZ)) {
                    extremeZ = z;
                    found    = true;
                }
            }
        }
        return extremeZ;
    }};

    for (i32 y {0}; y < MAP_HEIGHT; ++y) {
        for (i32 x {0}; x < MAP_WIDTH; ++x) {
            point_i const c {x, y};
            if (std::holds_alternative<normal_wall>(get_cell(c))) { continue; } // no floor, no ceiling

            u8 const lvl {surface_at(c).Level};
            _cellSlopes[PixelIndex(MAP_WIDTH, x, y)] = {.Ceiling = {.TL = vertex_height(x, y, false, lvl), .TR = vertex_height(x + 1, y, false, lvl), .BL = vertex_height(x, y + 1, false, lvl), .BR = vertex_height(x + 1, y + 1, false, lvl)},
                                                        .Floor   = {.TL = vertex_height(x, y, true, lvl), .TR = vertex_height(x + 1, y, true, lvl), .BL = vertex_height(x, y + 1, true, lvl), .BR = vertex_height(x + 1, y + 1, true, lvl)}};
        }
    }
}

auto map::is_blocking_light(point_i pos) const -> bool
{
    if (!map_grid::Size.contains(pos)) { return true; }

    return std::visit([](auto&& c) -> bool { return BlocksLight(c); }, get_cell(pos));
}

auto map::has_line_of_sight(point_d from, point_d to) const -> bool
{
    point_d const diff {to - from};
    f64 const     dist {diff.length()};
    if (dist < 1e-6) { return true; }

    auto const toCell {WorldToCell(to)};

    cell_walk walk {from, diff / dist};
    for (;;) {
        if (walk.exit() >= dist) { return true; }

        walk.advance();
        auto const cell {walk.cell()};
        if (cell == toCell) { return true; }
        if (!map_grid::Size.contains(cell)) { return true; }
        if (is_blocking_light(cell)) { return false; }
    }
}

auto map::closest_point_on_wall(point_i cellPos, point_d pos) const -> std::optional<point_d>
{
    return std::visit([&](auto const& w) { return w.closest_point(cellPos, pos); }, get_cell(cellPos));
}
