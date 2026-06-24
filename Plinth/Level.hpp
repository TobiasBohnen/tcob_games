// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"
#include "Walls.hpp"

struct sprite {
    point_d  Pos;
    size_d   Size {size_d::One}; // world-unit width/height; {1,1} = one full tile
    i32      Texture {-1};
    degree_f Facing {0};
    bool     Solid {true};
};

class level {
public:
    map_t               Map;
    std::vector<sprite> Sprites;

    level();

    auto update(milliseconds deltaSeconds) -> bool;
    void toggle_wall(point_i p);
};
