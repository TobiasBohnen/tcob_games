// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

#include "LevelObject.hpp"
#include "Walls.hpp"

class level {
public:
    explicit level(map_t map);

    void update(milliseconds deltaSeconds);

    auto get_cell(point_i p) const -> cell const&;

    auto is_clear(point_d pos, f64 radius, uid ignore = 0) const -> bool;
    auto is_wall_clear(point_d pos, f64 radius) const -> bool;

    auto hit_test(this auto&& self, point_d pos, f64 radius, uid ignore = 0);

    void toggle_wall(point_i p);

    auto is_seen(point_i cell) const -> bool;
    void mark_seen(point_i cell);
    void mark_all_seen();

    void show_message(string const& msg);
    auto get_message() const -> string const&;

    auto spawn(level_object_desc desc) -> uid;

    auto find(uid id) -> level_object*;

    auto objects() const -> std::span<level_object const>;
    auto objects() -> std::span<level_object>;

private:
    auto closest_point_on_wall(point_i map, point_d pos) const -> std::optional<point_d>;

    map_t                                    _map;
    static_grid<bool, MAP_WIDTH, MAP_HEIGHT> _seen;

    std::vector<level_object> _objects;
    uid                       _nextId {0};

    milliseconds _messageTimer {};
    string       _message;
};

inline auto level::hit_test(this auto&& self, point_d pos, f64 radius, uid ignore)
{
    using object_t = std::conditional_t<std::is_const_v<std::remove_reference_t<decltype(self)>>, level_object const, level_object>;

    object_t* found {nullptr};
    for (auto& go : self._objects) {
        if (go.id() != ignore && go.overlaps(pos, radius)) {
            found = &go;
            break;
        }
    }
    return found;
}
