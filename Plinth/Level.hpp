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

class level {
public:
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
    ::map      _map;
    ::motes    _motes;
    ::sky      _sky;
    ::lighting _lighting;

    std::vector<level_object> _objects;
    uid                       _nextId {0};

    milliseconds _time {};
};
