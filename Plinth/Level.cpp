// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Level.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstdlib>
#include <optional>
#include <span>
#include <type_traits>
#include <variant>
#include <vector>

#include "Common.hpp"
#include "Ground.hpp"
#include "LevelObject.hpp"
#include "Lighting.hpp"
#include "Map.hpp"
#include "Motes.hpp"
#include "Player.hpp"
#include "Sky.hpp"
#include "Walls.hpp"

void level::update(milliseconds deltaTime, i32 lightLosSamples, player& player)
{
    constexpr static f64 EYE_HEIGHT {0.5}; // eye height above the floor under the player
    player.EyeZ = floor_z_at(player.Position) + EYE_HEIGHT;

    _time += deltaTime;
    _sky.update(_time);

    f64 const dt {deltaTime.count() / 1000};

    std::vector<occupancy_event> occupancies;
    occupancies.reserve(_objects.size() + 1);
    occupancies.push_back({.Cell = WorldToCell(player.Position), .Position = player.Position});
    for (auto const& obj : _objects) {
        auto const& base {base_of(obj)};
        occupancies.push_back({.Object = base.id(), .Cell = WorldToCell(base.position().xy()), .Position = base.position().xy()});
    }

    // a closing door opens again while someone is in its cell
    for (auto const& ev : occupancies) {
        if (!map_grid::Size.contains(ev.Cell)) { continue; }

        auto const* door {std::get_if<door_wall>(&_map.get_cell(ev.Cell))};
        if (door && door->State == wall_state::Closing) { _map.toggle_wall(ev.Cell); }
    }

    if (!occupancies.empty()) { Occupancies(occupancies); }

    _map.update(dt);

    std::vector<collision_event> collisions;

    for (auto& obj : _objects) {
        // only what moves by itself and collides is stepped
        std::visit([&](auto& x) {
            using kind = std::remove_cvref_t<decltype(x)>;

            set_floor_height(obj);
            x.age(deltaTime);
            if (x.dead()) { return; }

            if constexpr (std::derived_from<kind, movable> && std::derived_from<kind, solid>) {
                auto const oldPos {x.position().xy()};
                auto const newPos {oldPos + x.Direction * dt};
                if (newPos == oldPos) { return; }

                auto const hit {find_blocker(oldPos, newPos, x.Radius, x.center_z(), x.flying(), x.id())};
                if (!hit) {
                    x.Blocked = false;
                    x.set_xy(newPos);
                    set_floor_height(obj);
                    return;
                }

                // reported once, when the object runs into something; it stays put until it moves again or the game removes it
                if (!x.Blocked) {
                    x.Blocked = true;
                    collisions.push_back({.Id = x.id(), .By = *hit, .At = oldPos});
                }
            }
        },
                   obj);
    }

    // after the loop, so a handler can spawn or kill objects; whatever it kills is removed right below
    if (!collisions.empty()) { Collisions(collisions); }

    std::erase_if(_objects, [](level_object const& go) { return base_of(go).dead(); });

    _lighting.update(*this, static_cast<usize>(lightLosSamples));
}

void level::invalidate() { _lighting.invalidate(); }

auto level::floor_z_at(point_d pos) const -> f64
{
    auto const cell {WorldToCell(pos)};
    return HeightAt(_map.slopes_at(cell).Floor, cell, pos);
}

void level::set_floor_height(level_object& o) const
{
    auto& base {base_of(o)};
    base.set_floor_z(is_flying(o) ? 0.0 : floor_z_at(base.position().xy()));
}

auto level::can_move_to(point_d from, point_d to, f64 radius, f64 z, bool flying, uid ignore) const -> bool
{
    return !find_blocker(from, to, radius, z, flying, ignore);
}

auto level::find_blocker(point_d from, point_d to, f64 radius, f64 z, bool flying, uid ignore) const -> std::optional<blocker>
{
    constexpr static f64 MAX_CLIMB {0.25};

    // true if `pred` holds for every cell in the bounding box of the circle at `pos`; it gets the cell and the point of it nearest to `pos`
    auto const box_all {[&](point_d pos, auto&& pred) -> bool {
        for (i32 ty {static_cast<i32>(std::floor(pos.Y - radius))}; ty <= static_cast<i32>(std::floor(pos.Y + radius)); ++ty) {
            for (i32 tx {static_cast<i32>(std::floor(pos.X - radius))}; tx <= static_cast<i32>(std::floor(pos.X + radius)); ++tx) {
                point_d const closest {std::clamp(pos.X, static_cast<f64>(tx), tx + 1.0), std::clamp(pos.Y, static_cast<f64>(ty), ty + 1.0)};
                if (!pred(point_i {tx, ty}, closest)) { return false; }
            }
        }
        return true;
    }};

    // the same for the cells that the circle overlaps
    auto const footprint_all {[&](point_d pos, auto&& pred) -> bool {
        return box_all(pos, [&](point_i cell, point_d closest) { return (pos - closest).length_squared() >= radius * radius || pred(cell, closest); });
    }};

    // no wall geometry within `radius` of `pos`, and inside the map
    auto const is_wall_clear {[&](point_d pos) -> bool {
        return box_all(pos, [&](point_i cell, point_d /* closest */) {
            if (!map_grid::Size.contains(cell)) { return false; }

            auto const wall {_map.closest_point_on_wall(cell, pos)};
            return !wall || (pos - *wall).length_squared() >= radius * radius;
        });
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
    auto const z_clear {[&](point_d pos, f64 zz) -> bool {
        constexpr static f64 DIAG {0.7071067811865476};

        std::array<point_d, 9> const samples {{{0.0, 0.0}, {1.0, 0.0}, {-1.0, 0.0}, {0.0, 1.0}, {0.0, -1.0}, {DIAG, DIAG}, {DIAG, -DIAG}, {-DIAG, DIAG}, {-DIAG, -DIAG}}};

        return std::ranges::all_of(samples, [&](point_d const& offset) {
            point_d const p {pos + (offset * radius)};
            point_i const cell {WorldToCell(p)};
            auto const    s {_map.slopes_at(cell)};
            return HeightAt(s.Floor, cell, p) <= zz - radius && HeightAt(s.Ceiling, cell, p) >= zz + radius;
        });
    }};

    // walls first: a flyer next to a wall also fails the floor and ceiling test, which would hide what it really hit
    if (!is_wall_clear(to)) { return blocker {.Kind = block_kind::Wall}; }

    // walkers only make moves that can be undone, otherwise the player can walk into a spot the same rule won't let them leave;
    // flyers are stopped by risers, slopes and ceilings in their way
    bool const surface_ok {flying ? z_clear(to, z) : (footprint_reachable(to, from) && footprint_reachable(from, to))};
    if (!surface_ok) { return blocker {.Kind = block_kind::Surface}; }

    // the first other solid object that the move runs into; voxel objects are tested against their actual voxels at height z
    auto const other {std::ranges::find_if(_objects, [&](level_object const& go) { return base_of(go).id() != ignore && overlaps(go, from, to, radius, z); })};
    if (other != _objects.end()) { return blocker {.Kind = block_kind::Object, .Object = base_of(*other).id()}; }

    return std::nullopt;
}

auto level::find_object(uid id) -> level_object*
{
    auto const it {std::ranges::lower_bound(_objects, id, {}, [](level_object const& o) { return base_of(o).id(); })};
    return (it != _objects.end() && base_of(*it).id() == id) ? &*it : nullptr;
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

auto level::ground_at(point_i pos) const -> ground_settings const& { return _ground.at(_map.surface_at(pos).Ground); }
auto level::ground() -> ::ground& { return _ground; }

auto level::sky() -> ::sky& { return _sky; }
auto level::sky() const -> ::sky const& { return _sky; }
