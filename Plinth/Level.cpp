// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Level.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <optional>
#include <span>
#include <type_traits>
#include <variant>
#include <vector>

#include "Common.hpp"
#include "LevelObject.hpp"
#include "Lighting.hpp"
#include "Sky.hpp"
#include "Walls.hpp"

level::level()
{
    _motes.resize(16);
}

void level::update(milliseconds deltaTime, i32 lightLosSamples)
{
    _time += deltaTime;
    _sky.update(_time);
    prepare_slopes();

    f64 const dt {deltaTime.count() / 1000};
    for (auto& cell : _map) {
        std::visit([dt](auto&& w) {
            if constexpr (requires { w.update(dt); }) {
                w.update(dt);
            }
        },
                   cell);
    }

    for (auto& obj : _objects) {
        auto const newPos {obj.position() + obj.direction() * dt};
        if (newPos == obj.position()) { continue; }

        if (obj.is_solid()) {
            if (can_move_to(obj.position(), newPos, obj.radius(), obj.id())) {
                obj.set_position(newPos);
            }
        } else {
            obj.set_position(newPos);
        }
    }

    std::erase_if(_objects, [](level_object const& go) { return go.dead(); });

    _lighting.update(*this, static_cast<usize>(lightLosSamples));
}

auto level::can_move_to(point_d from, point_d to, f64 radius, uid ignore) const -> bool
{
    constexpr static f64 MAX_CLIMB {0.25};

    // every floor under the footprint at `pos` has to be within climbing range of the floor at `ref`
    auto const footprint_reachable {[&](point_d pos, point_d ref) -> bool {
        point_i const refCell {WorldToCell(ref)};
        f64 const     height {HeightAt(slopes_at(refCell).Floor, refCell, ref)};

        for (i32 ty {static_cast<i32>(std::floor(pos.Y - radius))}; ty <= static_cast<i32>(std::floor(pos.Y + radius)); ++ty) {
            for (i32 tx {static_cast<i32>(std::floor(pos.X - radius))}; tx <= static_cast<i32>(std::floor(pos.X + radius)); ++tx) {
                point_i const cell {tx, ty};
                point_d const closest {std::clamp(pos.X, static_cast<f64>(tx), tx + 1.0), std::clamp(pos.Y, static_cast<f64>(ty), ty + 1.0)};
                point_d const d {pos - closest};
                if (d.length_squared() >= radius * radius) { continue; }

                if (std::abs(HeightAt(slopes_at(cell).Floor, cell, closest) - height) > MAX_CLIMB) { return false; }
            }
        }
        return true;
    }};

    // only allow moves that can be undone, otherwise the player can walk into a spot the same rule won't let them leave
    return footprint_reachable(to, from) && footprint_reachable(from, to) && is_wall_clear(to, radius) && hit_test(to, radius, ignore) == nullptr;
}

void level::toggle_wall(point_i map)
{
    std::visit(
        [](auto&& w) {
            if constexpr (requires { w.toggle(); }) {
                w.toggle();
            }
        },
        _map[map]);
}

auto level::get_visibility(point_i map) const -> tile_visibility
{
    if (!map_t::Size.contains(map)) { return tile_visibility::Hidden; }
    return _seen[map];
}

void level::mark_visible(point_i map)
{
    if (!map_t::Size.contains(map)) { return; }
    std::atomic_ref<tile_visibility> {_seen[map]}.store(tile_visibility::Visible, std::memory_order_relaxed);
}

void level::reveal_map()
{
    _seen.fill(tile_visibility::Explored);
}

void level::fade_vision()
{
    for (auto& s : _seen) {
        if (s == tile_visibility::Visible) {
            s = tile_visibility::Explored;
        }
    }
}

auto level::get_cell(point_i map) const -> cell const&
{
    return _map[map];
}

void level::set_cell(point_i map, cell const& c)
{
    _map[map]    = c;
    _slopesDirty = true;
}

auto level::surface_at(point_i map) const -> cell_surface
{
    if (!map_t::Size.contains(map)) { return {}; }

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
                      get_cell(map));
}

auto level::spawn(level_object_desc desc) -> uid
{
    uid const id {_nextId++};
    _objects.emplace_back(id, desc);
    return id;
}

auto level::find(uid id) -> level_object*
{
    auto const it {std::ranges::lower_bound(_objects, id, {}, &level_object::id)};
    return (it != _objects.end() && it->id() == id) ? &*it : nullptr;
}

auto level::objects() const -> std::span<level_object const>
{
    return _objects;
}

auto level::objects() -> std::span<level_object>
{
    return _objects;
}

auto level::time() const -> milliseconds
{
    return _time;
}

auto level::slopes_at(point_i map) const -> slopes
{
    if (!map_t::Size.contains(map)) { return {}; }
    return _cellSlopes[PixelIndex(MAP_WIDTH, map.X, map.Y)];
}

auto level::lighting() -> light_map& { return _lighting; }
auto level::lighting() const -> light_map const& { return _lighting; }

auto level::sky() -> sky_model& { return _sky; }
auto level::sky() const -> sky_model const& { return _sky; }

void level::prepare_slopes()
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

auto level::motes_at(point_i map) const -> motes const&
{
    u8 const motesIdx {surface_at(map).Motes};
    return _motes[motesIdx < _motes.size() ? motesIdx : 0];
}

void level::set_motes(u8 motesIdx, motes const& motes)
{
    if (motesIdx >= _motes.size()) { _motes.resize(static_cast<usize>(motesIdx) + 1); }
    _motes[motesIdx] = motes;
}

auto level::is_wall_clear(point_d pos, f64 radius) const -> bool
{
    i32 const minCellX {static_cast<i32>(std::floor(pos.X - radius))};
    i32 const maxCellX {static_cast<i32>(std::floor(pos.X + radius))};
    i32 const minCellY {static_cast<i32>(std::floor(pos.Y - radius))};
    i32 const maxCellY {static_cast<i32>(std::floor(pos.Y + radius))};

    for (i32 ty {minCellY}; ty <= maxCellY; ++ty) {
        for (i32 tx {minCellX}; tx <= maxCellX; ++tx) {
            if (tx < 0 || tx >= map_t::Size.Width || ty < 0 || ty >= map_t::Size.Height) { return false; }

            auto const closest {closest_point_on_wall({tx, ty}, pos)};
            if (!closest) { continue; }

            point_d const d {pos - *closest};
            if (d.length_squared() < radius * radius) { return false; }
        }
    }

    return true;
}

auto level::is_blocking_light(point_i map) const -> bool
{
    if (!map_t::Size.contains(map)) { return {}; }

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
                      get_cell(map));
}

auto level::hit_test(point_d pos, f64 radius, uid ignore) const -> level_object const*
{
    level_object const* found {nullptr};
    for (auto const& go : _objects) {
        if (go.id() != ignore && go.overlaps(pos, radius)) {
            found = &go;
            break;
        }
    }
    return found;
}

auto level::closest_point_on_wall(point_i map, point_d pos) const -> std::optional<point_d>
{
    rect_d const clampRect {static_cast<f64>(map.X), static_cast<f64>(map.Y), 1.0, 1.0};

    return overloaded_visit(
        get_cell(map),
        [](floor_cell const&) -> std::optional<point_d> { return std::nullopt; },
        [&](normal_wall const&) -> std::optional<point_d> {
            return point_d {std::clamp(pos.X, clampRect.left(), clampRect.right()),
                            std::clamp(pos.Y, clampRect.top(), clampRect.bottom())};
        },
        [&](obstacle const& w) -> std::optional<point_d> {
            std::optional<point_d> closest {};
            f64                    minDistSq {std::numeric_limits<f64>::infinity()};

            for (auto const& def : w.Shapes) {
                rect_d r {def.LocalBounds};
                r.move_by(map);

                point_d candidate {};

                if (def.IsRound) {
                    point_d const center {r.center()};
                    auto const [a, b] {r.local_center()};
                    if (a <= 0.0 || b <= 0.0) {
                        candidate = center;
                    } else {
                        point_d const uv {(pos.X - center.X) / a, (pos.Y - center.Y) / b};
                        f64 const     len {uv.length()};
                        if (len == 0.0) {
                            candidate = center;
                        } else {
                            point_d const onUnitCircle {uv.X / len, uv.Y / len};
                            candidate = point_d {center.X + (onUnitCircle.X * a), center.Y + (onUnitCircle.Y * b)};
                        }
                    }
                } else {
                    candidate = point_d {std::clamp(pos.X, r.left(), r.right()), std::clamp(pos.Y, r.top(), r.bottom())};
                }

                point_d const diff {pos - candidate};
                f64 const     distSq {diff.length_squared()};
                if (distSq < minDistSq) {
                    minDistSq = distSq;
                    closest   = candidate;
                }
            }

            return closest;
        },
        [&](diagonal_wall const& w) -> std::optional<point_d> {
            f64 const     cX {static_cast<f64>(map.X)};
            f64 const     cY {static_cast<f64>(map.Y)};
            bool const    nwSe {w.Orientation == diagonal_wall::orientation::NorthWestToSouthEast};
            point_d const a {cX, nwSe ? cY : cY + 1.0};
            point_d const b {cX + 1.0, nwSe ? cY + 1.0 : cY};
            point_d const ab {b.X - a.X, b.Y - a.Y};
            f64 const     t {std::clamp(((pos.X - a.X) * ab.X + (pos.Y - a.Y) * ab.Y) / (ab.X * ab.X + ab.Y * ab.Y), 0.0, 1.0)};
            return point_d {a.X + (t * ab.X), a.Y + (t * ab.Y)};
        },
        [&](auto const& w) -> std::optional<point_d> {
            if (w.State != wall_state::Open) {
                return point_d {std::clamp(pos.X, clampRect.left(), clampRect.right()),
                                std::clamp(pos.Y, clampRect.top(), clampRect.bottom())};
            }
            return std::nullopt;
        });
}
