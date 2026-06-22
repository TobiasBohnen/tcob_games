// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../_common/Common.hpp"

enum class wall_kind : u8 {
    Normal,
    Door,
    PushWall,
    HalfWall
};

enum class wall_state : u8 {
    Closed  = 0,
    Opening = 1,
    Open    = 2,
    Closing = 3
};

enum class door_orientation : u8 {
    BlocksNorthSouth, // door slab runs east-west, at y = cellY + 0.5
    BlocksEastWest    // door slab runs north-south, at x = cellX + 0.5
};

enum class hit_type : u8 {
    None,
    Normal,
    Special
};

struct wall_hit {
    hit_type Hit {hit_type::None};

    f64  Distance {0.0}; // ray parameter t at the intersection
    f64  SegmentT {0.0}; // 0..1 across the hit surface, for texture X coordinate
    i32  Texture {0};
    bool Side {false};
};

struct empty {
    auto intersect(point_i, point_d, point_d) const -> wall_hit;
};

struct normal_wall {
    i32 Texture {0};

    auto intersect(point_i, point_d, point_d) const -> wall_hit;
};

struct door_wall {
    wall_state State {wall_state::Closed};
    f64        Timer {0.0}; // 0 = closed, 1 = fully open

    door_orientation Orientation {door_orientation::BlocksNorthSouth};

    i32 Texture {0};

    auto intersect(point_i cell, point_d rayOrigin, point_d rayDir) const -> wall_hit;

    static constexpr f64 OpenSpeed {1.5};
};

struct push_wall {
    wall_state State {wall_state::Closed};
    f64        Timer {0.0}; // 0 = closed, 1 = fully open

    point_i PushDirection {0, 0};

    i32 Texture {0};

    auto intersect(point_i cell, point_d rayOrigin, point_d rayDir) const -> wall_hit;

    static constexpr f64 OpenSpeed {1.5};
};

struct half_wall {
    rect_d LocalBounds;

    i32 Texture {0};

    auto intersect(point_i cell, point_d rayOrigin, point_d rayDir) const -> wall_hit;
};

using wall = std::variant<empty, normal_wall, door_wall, push_wall, half_wall>;

inline constexpr i32 mapWidth {24};
inline constexpr i32 mapHeight {24};
using map_t = static_grid<wall, mapWidth, mapHeight>;

auto update_special_walls(map_t& walls, milliseconds deltaSeconds) -> bool;

void toggle_special_wall(wall& wall);

////////////////////////////////////////////////////////////
