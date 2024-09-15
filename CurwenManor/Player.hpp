// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep
#include "GameCanvas.hpp"
#include "Tiles.hpp"

namespace stn {

/////////////////////////////////////////////////////

class player {
public:
    player(tilemap& map, canvas& canvas, assets& assets);

    void update(milliseconds deltaTime);
    void draw(gfx::canvas& canvas);

    void move(direction dir, bool dirOnly);
    void set_direction(direction dir, i32 phase);

private:
    tilemap& _map;
    canvas&  _canvas;
    assets&  _assets;

    tileset     _tiles;
    std::string _tile {};
    point_f     _position {};
    direction   _direction {};

    i32 _health {5};
    i32 _sanity {5};

    std::shared_ptr<smootherstep_tween<point_f>> _move;
};

}
