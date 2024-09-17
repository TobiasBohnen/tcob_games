// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep
#include "Tiles.hpp"

namespace stn {

/////////////////////////////////////////////////////

class player {
public:
    player(game_scene* parent);

    signal<> Turn;
    signal<> Dead;
    signal<> Insane;

    void update(milliseconds deltaTime);
    void draw(canvas& canvas, point_f offset);

    void move(direction dir, bool dirOnly); // ends turn
    auto get_position() const -> point_f;
    void set_position(point_f pos);

    auto get_direction() const -> direction;
    void set_direction(direction dir, i32 phase);

    auto get_turn() const -> i32;
    void reset_turn();

    void take_health_damage();
    void take_sanity_damage();

private:
    void end_turn();

    game_scene* _parent;

    tileset     _tiles;
    std::string _tile {};
    point_f     _position {};
    direction   _direction {};

    i32 _health {5};
    i32 _sanity {5};
    i32 _turn {58};

    std::shared_ptr<smootherstep_tween<point_f>> _move;
};

}
