// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Level.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <optional>
#include <span>
#include <vector>

#include "Common.hpp"
#include "LevelObject.hpp"
#include "Lighting.hpp"
#include "Map.hpp"
#include "Motes.hpp"
#include "Sky.hpp"
#include "Walls.hpp"

void level::update(milliseconds deltaTime, i32 lightLosSamples)
{
    _time += deltaTime;
    _sky.update(_time);

    f64 const dt {deltaTime.count() / 1000};
    _map.update(dt);

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

    // nearest solid point of a cell's wall geometry, nullopt if the cell has none
    auto const closest_point_on_wall {[&](point_i cellPos, point_d pos) -> std::optional<point_d> {
        rect_d const clampRect {static_cast<f64>(cellPos.X), static_cast<f64>(cellPos.Y), 1.0, 1.0};

        return overloaded_visit(
            _map.get_cell(cellPos),
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
                    r.move_by(cellPos);

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

                    f64 const distSq {(pos - candidate).length_squared()};
                    if (distSq < minDistSq) {
                        minDistSq = distSq;
                        closest   = candidate;
                    }
                }

                return closest;
            },
            [&](diagonal_wall const& w) -> std::optional<point_d> {
                f64 const     cX {static_cast<f64>(cellPos.X)};
                f64 const     cY {static_cast<f64>(cellPos.Y)};
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
    }};

    // no wall geometry within `radius` of `pos`, and inside the map
    auto const is_wall_clear {[&](point_d pos) -> bool {
        i32 const minCellX {static_cast<i32>(std::floor(pos.X - radius))};
        i32 const maxCellX {static_cast<i32>(std::floor(pos.X + radius))};
        i32 const minCellY {static_cast<i32>(std::floor(pos.Y - radius))};
        i32 const maxCellY {static_cast<i32>(std::floor(pos.Y + radius))};

        for (i32 ty {minCellY}; ty <= maxCellY; ++ty) {
            for (i32 tx {minCellX}; tx <= maxCellX; ++tx) {
                if (tx < 0 || tx >= map_t::Size.Width || ty < 0 || ty >= map_t::Size.Height) { return false; }

                auto const closest {closest_point_on_wall({tx, ty}, pos)};
                if (!closest) { continue; }

                if ((pos - *closest).length_squared() < radius * radius) { return false; }
            }
        }
        return true;
    }};

    // no other object overlaps the circle at `pos`
    auto const hit_test {[&](point_d pos) -> bool {
        return std::ranges::any_of(_objects, [&](level_object const& go) { return go.id() != ignore && go.overlaps(pos, radius); });
    }};

    // every floor under the footprint at `pos` has to be within climbing range of the floor at `ref`
    auto const footprint_reachable {[&](point_d pos, point_d ref) -> bool {
        point_i const refCell {WorldToCell(ref)};
        f64 const     height {HeightAt(_map.slopes_at(refCell).Floor, refCell, ref)};

        for (i32 ty {static_cast<i32>(std::floor(pos.Y - radius))}; ty <= static_cast<i32>(std::floor(pos.Y + radius)); ++ty) {
            for (i32 tx {static_cast<i32>(std::floor(pos.X - radius))}; tx <= static_cast<i32>(std::floor(pos.X + radius)); ++tx) {
                point_i const cell {tx, ty};
                point_d const closest {std::clamp(pos.X, static_cast<f64>(tx), tx + 1.0), std::clamp(pos.Y, static_cast<f64>(ty), ty + 1.0)};
                if ((pos - closest).length_squared() >= radius * radius) { continue; }

                if (std::abs(HeightAt(_map.slopes_at(cell).Floor, cell, closest) - height) > MAX_CLIMB) { return false; }
            }
        }
        return true;
    }};

    // only allow moves that can be undone, otherwise the player can walk into a spot the same rule won't let them leave
    return footprint_reachable(to, from) && footprint_reachable(from, to) && is_wall_clear(to) && !hit_test(to);
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

auto level::lighting() -> ::lighting& { return _lighting; }
auto level::lighting() const -> ::lighting const& { return _lighting; }

auto level::map() const -> ::map const& { return _map; }
auto level::map() -> ::map& { return _map; }

auto level::motes_at(point_i pos) const -> mote_settings const& { return _motes.at(_map.surface_at(pos).Motes); }
auto level::motes() -> ::motes& { return _motes; }

auto level::sky() -> ::sky& { return _sky; }
auto level::sky() const -> ::sky const& { return _sky; }