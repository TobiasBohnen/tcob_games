// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

#include <optional>
#include <variant>
#include <vector>

////////////////////////////////////////////////////////////

enum class wall_state : u8 {
    Closed  = 0,
    Opening = 1,
    Open    = 2,
    Closing = 3
};

enum class hit_side : u8 {
    NorthSouth,
    WestEast,
    Diagonal
};

struct wall_hit {
    f64 Distance {0.0}; // ray parameter t at the intersection
    f64 SegmentT {0.0}; // 0..1 across the hit surface, for texture X coordinate

    hit_side Side {};
    i32      Texture {0};
};

struct cell_intersect {
    point_i Cell;
    point_d RayOrigin;
    point_d RayDir;
    bool    Side {false};
    f64     Distance {0};
};

////////////////////////////////////////////////////////////

// what a cell shows and does on its floor and ceiling, whatever the cell is
struct cell_surface {
    i32 FloorTexture {INVALID_INDEX};
    i32 CeilingTexture {INVALID_INDEX};
    f64 FloorReflectivity {0.0};
    f64 FloorZ {0.0};
    f64 CeilingZ {1.0};
    u8  Motes {0};
    u8  Level {0};

    f64   FloorEmissive {0.0};  // 0: lit by the lights of the level, 1: glows by itself
    f64   FloorGlowRange {0.0}; // cells: how far an emissive floor lights its surroundings; 0: not at all
    color FloorGlowColor {colors::White};
};

////////////////////////////////////////////////////////////

// Every cell type answers closest_point(): the point of its solid geometry nearest to `pos`, for the cell at `cellPos`.
// Nothing for cells without any (floors, open doors).

struct floor_cell {
    cell_surface Surface {};

    auto intersect(cell_intersect const& ci) const -> std::optional<wall_hit>;
    auto closest_point(point_i cellPos, point_d pos) const -> std::optional<point_d>;
};

struct normal_wall {
    i32 Texture {0};

    auto intersect(cell_intersect const& ci) const -> std::optional<wall_hit>;
    auto closest_point(point_i cellPos, point_d pos) const -> std::optional<point_d>;
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
    cell_surface Surface {};

    auto intersect(cell_intersect const& ci) const -> std::optional<wall_hit>;
    auto closest_point(point_i cellPos, point_d pos) const -> std::optional<point_d>;
    void update(f64 dt);
    void toggle();

    static constexpr f64 OpenSpeed {1.5};
};

struct push_wall {
    wall_state State {wall_state::Closed};
    f64        Timer {0.0}; // 0 = closed, 1 = fully open

    point_i PushDirection {0, 0};

    i32 Texture {0};
    cell_surface Surface {};

    auto intersect(cell_intersect const& ci) const -> std::optional<wall_hit>;
    auto closest_point(point_i cellPos, point_d pos) const -> std::optional<point_d>;
    void update(f64 dt);
    void toggle();

    static constexpr f64 OpenSpeed {1.5};
};

struct obstacle_shape {
    rect_d LocalBounds {rect_d::Zero};
    bool   IsRound {false};

    i32 Texture {0};
};

struct obstacle {
    std::vector<obstacle_shape> Shapes {};

    cell_surface Surface {};

    auto intersect(cell_intersect const& ci) const -> std::optional<wall_hit>;
    auto closest_point(point_i cellPos, point_d pos) const -> std::optional<point_d>;
};

struct diagonal_wall {
    enum class orientation : u8 {
        NorthWestToSouthEast,
        SouthWestToNorthEast
    };

    orientation Orientation {orientation::NorthWestToSouthEast};

    i32 Texture {0};
    cell_surface Surface {};

    auto intersect(cell_intersect const& ci) const -> std::optional<wall_hit>;
    auto closest_point(point_i cellPos, point_d pos) const -> std::optional<point_d>;
};

using cell = std::variant<floor_cell, normal_wall, door_wall, push_wall, obstacle, diagonal_wall>;

inline constexpr i32 MAP_WIDTH {64};
inline constexpr i32 MAP_HEIGHT {64};
using map_grid = static_grid<cell, MAP_WIDTH, MAP_HEIGHT>;

////////////////////////////////////////////////////////////
