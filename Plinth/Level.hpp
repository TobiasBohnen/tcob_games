// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

#include <optional>
#include <span>
#include <utility>
#include <vector>

#include "Ground.hpp"
#include "LevelObject.hpp"
#include "Lighting.hpp"
#include "Map.hpp"
#include "Motes.hpp"
#include "Sky.hpp"

enum class block_kind : u8 {
    Wall,    // wall, door, obstacle or the edge of the map
    Surface, // floor, ceiling or riser in the way
    Object   // another solid object
};

struct blocker {
    block_kind         Kind {block_kind::Wall};
    std::optional<uid> Object {}; // the object in the way, for block_kind::Object
};

struct collision_event {
    uid     Id {}; // the object that was stopped
    blocker By {};
    point_d At {}; // where it stopped
};

struct occupancy_event {
    std::optional<uid> Object {}; // empty for the player
    point_i            Cell {};
    point_d            Position {};
};

class level {
public:
    signal<std::vector<collision_event>, level> Collisions;
    signal<std::vector<occupancy_event>, level> Occupancies;

    void update(milliseconds deltaTime, i32 lightLosSamples, point_d playerPosition);

    void invalidate();

    auto floor_z_at(point_d pos) const -> f64; // absolute height of the floor under a position

    auto can_move_to(point_d from, point_d to, f64 radius, f64 z, bool flying, uid ignore) const -> bool;

    template <typename T, typename... Args>
    auto spawn(object_desc const& desc, Args&&... args) -> uid;

    auto find(uid id) -> level_object*;

    auto objects() const -> std::span<level_object const>;
    auto objects() -> std::span<level_object>;

    auto time() const -> milliseconds;

    auto lighting() -> ::lighting&;
    auto lighting() const -> ::lighting const&;

    auto map() -> ::map&;
    auto map() const -> ::map const&;

    auto motes_at(point_i pos) const -> mote_settings const&;
    auto motes() -> ::motes&;

    auto ground_at(point_i pos) const -> ground_settings const&;
    auto ground() -> ::ground&;

    auto sky() -> ::sky&;
    auto sky() const -> ::sky const&;

private:
    void set_floor_height(level_object& o) const;

    auto find_blocker(point_d from, point_d to, f64 radius, f64 z, bool flying, uid ignore) const -> std::optional<blocker>;

    ::map      _map;
    ::motes    _motes;
    ::ground   _ground;
    ::sky      _sky;
    ::lighting _lighting;

    std::vector<level_object> _objects;
    uid                       _nextId {0};

    milliseconds _time {};
};

template <typename T, typename... Args>
inline auto level::spawn(object_desc const& desc, Args&&... args) -> uid
{
    uid const id {_nextId++};
    auto&     created {_objects.emplace_back(std::in_place_type<T>, id, desc, std::forward<Args>(args)...)};
    set_floor_height(created);
    return id;
}
