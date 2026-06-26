// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

////////////////////////////////////////////////////////////

enum class wall_state : u8 {
    Closed  = 0,
    Opening = 1,
    Open    = 2,
    Closing = 3
};

struct wall_hit {
    f64 Distance {0.0}; // ray parameter t at the intersection
    f64 SegmentT {0.0}; // 0..1 across the hit surface, for texture X coordinate

    f64 Light {0.0};

    i32 Texture {0};

    bool Hit {false};
    bool Transparent {false};
    bool Shaded {false};
};

struct cell_intersect {
    point_i Cell;
    point_d RayOrigin;
    point_d RayDir;
    bool    Side {false};
    f64     Distance {0};
};

////////////////////////////////////////////////////////////

struct empty_cell {
    auto intersect(cell_intersect const& ci) const -> wall_hit;
};

struct cell {
    i32 FloorTexture {INVALID_INDEX};
    i32 CeilingTexture {INVALID_INDEX};
    f64 Light {0.0};

    auto intersect(cell_intersect const& ci) const -> wall_hit;
};

struct normal_wall {
    i32 Texture {0};

    auto intersect(cell_intersect const& ci) const -> wall_hit;
};

struct door_wall {
    enum class orientation : u8 {
        BlocksNorthSouth, // door slab runs east-west, at y = cellY + 0.5
        BlocksEastWest    // door slab runs north-south, at x = cellX + 0.5
    };

    wall_state State {wall_state::Closed};
    f64        Timer {0.0}; // 0 = closed, 1 = fully open

    orientation Orientation {orientation::BlocksNorthSouth};

    i32 Texture {0};
    i32 FrameTexture {0};
    i32 FloorTexture {INVALID_INDEX};
    i32 CeilingTexture {INVALID_INDEX};
    f64 Light {0.0};

    auto intersect(cell_intersect const& ci) const -> wall_hit;
    auto update(f64 dt) -> bool;
    void toggle();

    static constexpr f64 OpenSpeed {1.5};
};

struct push_wall {
    wall_state State {wall_state::Closed};
    f64        Timer {0.0}; // 0 = closed, 1 = fully open

    point_i PushDirection {0, 0};

    i32 Texture {0};
    i32 FloorTexture {INVALID_INDEX};
    i32 CeilingTexture {INVALID_INDEX};
    f64 Light {0.0};

    auto intersect(cell_intersect const& ci) const -> wall_hit;
    auto update(f64 dt) -> bool;
    void toggle();

    static constexpr f64 OpenSpeed {1.5};
};

struct box_wall {
    rect_d LocalBounds;

    i32 Texture {0};
    i32 FloorTexture {INVALID_INDEX};
    i32 CeilingTexture {INVALID_INDEX};
    f64 Light {0.0};

    auto intersect(cell_intersect const& ci) const -> wall_hit;
};

struct diagonal_wall {
    enum class orientation : u8 {
        NorthWestToSouthEast,
        SouthWestToNorthEast
    };

    orientation Orientation {orientation::NorthWestToSouthEast};

    i32 Texture {0};
    i32 FloorTexture {INVALID_INDEX};
    i32 CeilingTexture {INVALID_INDEX};
    f64 Light {0.0};

    auto intersect(cell_intersect const& ci) const -> wall_hit;
};

struct round_pillar {
    f64 Radius {0};

    i32 Texture {0};
    i32 FloorTexture {INVALID_INDEX};
    i32 CeilingTexture {INVALID_INDEX};
    f64 Light {0.0};

    auto intersect(cell_intersect const& ci) const -> wall_hit;
};

struct thin_wall {
    enum class orientation : u8 {
        BlocksNorthSouth,
        BlocksEastWest
    };
    orientation Orientation {orientation::BlocksNorthSouth};
    f64         Offset {0.5};

    i32 Texture {0};
    i32 FloorTexture {-1};
    i32 CeilingTexture {-1};
    f64 Light {0.0};

    auto intersect(cell_intersect const& ci) const -> wall_hit;
};

using wall = std::variant<empty_cell, cell, normal_wall, door_wall, push_wall, box_wall, diagonal_wall, round_pillar, thin_wall>;

inline constexpr i32 mapWidth {64};
inline constexpr i32 mapHeight {64};
using map_t = static_grid<wall, mapWidth, mapHeight>;

////////////////////////////////////////////////////////////
