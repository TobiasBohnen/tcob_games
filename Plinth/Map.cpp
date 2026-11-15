// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Map.hpp"

#include <atomic>
#include <cmath>
#include <optional>
#include <type_traits>
#include <variant>

#include "Common.hpp"
#include "Walls.hpp"

void map::update(f64 dt)
{
    prepare_slopes();

    // fade vision
    for (auto& s : _seen) {
        if (s == tile_visibility::Visible) { s = tile_visibility::Explored; }
    }

    for (auto& cell : _cells) {
        std::visit([dt](auto&& w) {
            if constexpr (requires { w.update(dt); }) {
                w.update(dt);
            }
        },
                   cell);
    }
}

void map::toggle_wall(point_i pos)
{
    std::visit(
        [](auto&& w) {
            if constexpr (requires { w.toggle(); }) {
                w.toggle();
            }
        },
        _cells[pos]);
}

auto map::get_visibility(point_i pos) const -> tile_visibility
{
    if (!map_t::Size.contains(pos)) { return tile_visibility::Hidden; }
    return _seen[pos];
}

void map::mark_visible(point_i pos)
{
    if (!map_t::Size.contains(pos)) { return; }
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
    if (!map_t::Size.contains(pos)) { return {}; }

    return std::visit([](auto&& c) -> cell_surface {
        cell_surface s {};
        if constexpr (requires { c.FloorTexture; }) { s.FloorTexture = c.FloorTexture; }
        if constexpr (requires { c.CeilingTexture; }) { s.CeilingTexture = c.CeilingTexture; }
        if constexpr (requires { c.FloorReflectivity; }) { s.FloorReflectivity = c.FloorReflectivity; }
        if constexpr (requires { c.FloorZ; }) { s.FloorZ = c.FloorZ; }
        if constexpr (requires { c.CeilingZ; }) { s.CeilingZ = c.CeilingZ; }
        if constexpr (requires { c.Level; }) { s.Level = c.Level; }
        if constexpr (requires { c.Motes; }) { s.Motes = c.Motes; }
        return s;
    },
                      get_cell(pos));
}

auto map::slopes_at(point_i pos) const -> slopes
{
    if (!map_t::Size.contains(pos)) { return {}; }
    return _cellSlopes[PixelIndex(MAP_WIDTH, pos.X, pos.Y)];
}

void map::prepare_slopes()
{
    if (!_slopesDirty) { return; }
    _slopesDirty = false;

    _cellSlopes.resize(static_cast<usize>(MAP_WIDTH) * MAP_HEIGHT);

    // height of a vertex as seen from cells of one level: the largest absolute value wins, walls and other levels don't count
    auto const vertex_height {[&](i32 vx, i32 vy, bool isFloor, u8 cellLevel) -> f64 {
        f64  extremeZ {0.0};
        bool found {false};
        for (i32 cy : {vy - 1, vy}) {
            for (i32 cx : {vx - 1, vx}) {
                point_i const c {cx, cy};
                if (!map_t::Size.contains(c) || std::holds_alternative<normal_wall>(get_cell(c))) { continue; }

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
    if (!map_t::Size.contains(pos)) { return {}; }

    return std::visit([](auto&& c) -> bool {
        using T = std::decay_t<decltype(c)>;
        if constexpr (std::is_same_v<T, floor_cell>) {
            return false;
        } else if constexpr (requires { c.State; }) {
            return c.State != wall_state::Open; // doors/push walls: open = passable
        } else {
            return true;                        // normal_wall, obstacle, diagonal_wall: always solid
        }
    },
                      get_cell(pos));
}

auto map::closest_point_on_wall(point_i cellPos, point_d pos) const -> std::optional<point_d>
{
    return std::visit([&](auto const& w) { return w.closest_point(cellPos, pos); }, get_cell(cellPos));
}
