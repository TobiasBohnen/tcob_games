// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

#include <optional>
#include <span>
#include <vector>

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

// an object that ran into something while moving
struct collision_event {
    uid     Id {}; // the object that was stopped
    blocker By {};
    point_d At {}; // where it stopped
};

class level {
public:
    signal<collision_event, level> Collision;

    void update(milliseconds deltaTime, i32 lightLosSamples);

    auto can_move_to(point_d from, point_d to, f64 radius, std::optional<f64> flightZ, uid ignore) const -> bool;

    auto spawn(level_object_desc desc) -> uid;
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

    auto sky() -> ::sky&;
    auto sky() const -> ::sky const&;

private:
    auto find_blocker(point_d from, point_d to, f64 radius, std::optional<f64> flightZ, uid ignore) const -> std::optional<blocker>;

    ::map      _map;
    ::motes    _motes;
    ::sky      _sky;
    ::lighting _lighting;

    std::vector<level_object> _objects;
    uid                       _nextId {0};

    milliseconds _time {};
};
