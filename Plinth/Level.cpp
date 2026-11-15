// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Level.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
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
            if (can_move_to(obj.position(), newPos, obj.radius(), obj.flight_z(), obj.id())) {
                obj.set_position(newPos);
            }
        } else {
            obj.set_position(newPos);
        }
    }

    std::erase_if(_objects, [](level_object const& go) { return go.dead(); });

    _lighting.update(*this, static_cast<usize>(lightLosSamples));
}

auto level::can_move_to(point_d from, point_d to, f64 radius, std::optional<f64> flightZ, uid ignore) const -> bool
{
    constexpr static f64 MAX_CLIMB {0.25};

    // no wall geometry within `radius` of `pos`, and inside the map
    auto const is_wall_clear {[&](point_d pos) -> bool {
        i32 const minCellX {static_cast<i32>(std::floor(pos.X - radius))};
        i32 const maxCellX {static_cast<i32>(std::floor(pos.X + radius))};
        i32 const minCellY {static_cast<i32>(std::floor(pos.Y - radius))};
        i32 const maxCellY {static_cast<i32>(std::floor(pos.Y + radius))};

        for (i32 ty {minCellY}; ty <= maxCellY; ++ty) {
            for (i32 tx {minCellX}; tx <= maxCellX; ++tx) {
                if (tx < 0 || tx >= map_t::Size.Width || ty < 0 || ty >= map_t::Size.Height) { return false; }

                auto const closest {_map.closest_point_on_wall({tx, ty}, pos)};
                if (!closest) { continue; }

                if ((pos - *closest).length_squared() < radius * radius) { return false; }
            }
        }
        return true;
    }};

    // no other object overlaps the circle at `pos`; flyers don't stop each other
    auto const hit_test {[&](point_d pos) -> bool {
        return std::ranges::any_of(_objects, [&](level_object const& go) {
            if (go.id() == ignore || (flightZ && go.flight_z())) { return false; }
            return go.overlaps(pos, radius);
        });
    }};

    // true if `pred` holds for every cell the circle at `pos` overlaps; it gets the cell and the point of it nearest to `pos`
    auto const footprint_all {[&](point_d pos, auto&& pred) -> bool {
        for (i32 ty {static_cast<i32>(std::floor(pos.Y - radius))}; ty <= static_cast<i32>(std::floor(pos.Y + radius)); ++ty) {
            for (i32 tx {static_cast<i32>(std::floor(pos.X - radius))}; tx <= static_cast<i32>(std::floor(pos.X + radius)); ++tx) {
                point_i const cell {tx, ty};
                point_d const closest {std::clamp(pos.X, static_cast<f64>(tx), tx + 1.0), std::clamp(pos.Y, static_cast<f64>(ty), ty + 1.0)};
                if ((pos - closest).length_squared() >= radius * radius) { continue; }

                if (!pred(cell, closest)) { return false; }
            }
        }
        return true;
    }};

    // every floor under the footprint at `pos` has to be within climbing range of the floor at `ref`
    auto const footprint_reachable {[&](point_d pos, point_d ref) -> bool {
        point_i const refCell {WorldToCell(ref)};
        f64 const     height {HeightAt(_map.slopes_at(refCell).Floor, refCell, ref)};

        return footprint_all(pos, [&](point_i cell, point_d closest) {
            return std::abs(HeightAt(_map.slopes_at(cell).Floor, cell, closest) - height) <= MAX_CLIMB;
        });
    }};

    // a flying object is a sphere at height `z`. Its center and eight points on its rim are tested against the floor and ceiling
    // of the cell they are in: a slope under the rim touches the sphere before the center sees any rise.
    auto const z_clear {[&](point_d pos, f64 z) -> bool {
        constexpr static f64 DIAG {0.7071067811865476};

        std::array<point_d, 9> const samples {{{0.0, 0.0}, {1.0, 0.0}, {-1.0, 0.0}, {0.0, 1.0}, {0.0, -1.0}, {DIAG, DIAG}, {DIAG, -DIAG}, {-DIAG, DIAG}, {-DIAG, -DIAG}}};

        return std::ranges::all_of(samples, [&](point_d const& offset) {
            point_d const p {pos + (offset * radius)};
            point_i const cell {WorldToCell(p)};
            auto const    s {_map.slopes_at(cell)};
            return HeightAt(s.Floor, cell, p) <= z - radius && HeightAt(s.Ceiling, cell, p) >= z + radius;
        });
    }};

    // walkers only make moves that can be undone, otherwise the player can walk into a spot the same rule won't let them leave;
    // flyers are stopped by risers and ceilings in their way, walls and other objects stop everybody
    bool const level_ok {flightZ ? z_clear(to, *flightZ) : (footprint_reachable(to, from) && footprint_reachable(from, to))};
    return level_ok && is_wall_clear(to) && !hit_test(to);
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
