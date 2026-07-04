// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"
#include "Walls.hpp"

struct sprite {
    point_d  Position;
    size_d   Size {size_d::One}; // world-unit width/height; {1,1} = one full tile
    i32      Texture {-1};
    degree_f Facing {0};
    bool     Solid {true};
};

class level {
public:
    level();

    map_t                                  Map;
    static_grid<bool, mapWidth, mapHeight> Seen;
    std::vector<sprite>                    Sprites;

    f64 FogMin {0.0};
    f64 FogDistance {12.0};
    f64 AmbientLight {1.0};

    i32  FloorTexture {0};
    i32  CeilingTexture {0};
    bool IsSkybox {false};

    auto update(milliseconds deltaSeconds) -> bool;

    void toggle_wall(point_i p);

    void mark_seen(point_i cell, point_d playerPos);
};
