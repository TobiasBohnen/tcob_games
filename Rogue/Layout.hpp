// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

namespace Rogue {

////////////////////////////////////////////////////////////

class base_layout {
public:
    virtual ~base_layout() = default;

    auto virtual generate(u64 seed, size_i size) -> grid<tile> = 0;

protected:
    void fill_grid(grid<tile>& grid, rng& rng, std::span<tile const> tiles) const;

    void draw_room(grid<tile>& grid, rect_i const& rect, tile const& floor);
    void draw_hallway(grid<tile>& grid, rect_i const& room1, rect_i const& room2, tile const& floor, bool b);

private:
    void draw_horizontal_tunnel(grid<tile>& grid, i32 x0, i32 x1, i32 y, tile const& floor);
    void draw_vertical_tunnel(grid<tile>& grid, i32 x, i32 y0, i32 y1, tile const& floor);
};

////////////////////////////////////////////////////////////

class turtle : public base_layout {
public:
    enum class scale_target {
        Path,
        RoomWidth,
        RoomHeight
    };

    struct pen {
        point_i   Position {0, 0};
        direction Direction {direction::Right};
        size_i    RoomSize {4, 4};
        i32       PathLength {4};

        scale_target ScaleTarget {scale_target::Path};
    };

    turtle(pen const& start, string sequence);

    auto generate(u64 seed, size_i size) -> grid<tile> override;

private:
    void interpret_string();
    void apply_scale(pen& t, i32 factor);
    void rotate_direction(pen& t, bool clockwise);

    void draw_room(pen& t);
    void draw_path(pen& t, i32 l);
    void move_forward(pen& t);

    string _sequence;

    grid<tile> _grid;
    rng        _rng;

    pen _startPen;
};

////////////////////////////////////////////////////////////

class tunneling : public base_layout {
public:
    tunneling(i32 maxRooms, i32 minRoomSize, i32 maxRoomSize);

    auto generate(u64 seed, size_i size) -> grid<tile> override;

private:
    i32 _maxRooms;
    i32 _minRoomSize;
    i32 _maxRoomSize;
};

////////////////////////////////////////////////////////////

class bsp_tree : public base_layout {
public:
    bsp_tree(i32 minRoomSize, i32 maxRoomSize);

    auto generate(u64 seed, size_i size) -> grid<tile> override;

private:
    i32 _minRoomSize;
    i32 _maxRoomSize;
};

////////////////////////////////////////////////////////////

class drunkards_walk : public base_layout {
public:
    drunkards_walk(f32 percentGoal, i32 walkIterations, f32 weightedTowardCenter, f32 weightedTowardPreviousDirection);

    auto generate(u64 seed, size_i size) -> grid<tile> override;

private:
    void walk(rng& rng);

    f32 _percentGoal;
    i32 _walkIterations;
    f32 _weightedTowardCenter;
    f32 _weightedTowardPreviousDirection;

    i32       _filled {0};
    f32       _filledGoal {0};
    direction _previousDirection {direction::None};
    i32       _drunkardX {0};
    i32       _drunkardY {0};

    tile _floor;

    grid<tile> _grid;
};

////////////////////////////////////////////////////////////

class cellular_automata : public base_layout {
public:
    cellular_automata(i32 iterations, i32 neighbors, f32 wallProbability, i32 minRoomSize, i32 maxRoomSize, bool smoothEdges, i32 smoothing);

    auto generate(u64 seed, size_i size) -> grid<tile> override;

private:
    void random_fill_map();
    void draw_caves();
    void cleanup_map();
    void create_tunnel(point_i point1, point_i point2, std::unordered_set<i32> const& currentCave);
    auto get_adjacent_walls(point_i p) -> i32;
    auto get_adjacent_walls_simple(point_i p) -> i32;
    void get_caves();
    void flood_fill(point_i pos);
    void connect_caves();
    auto check_connectivity(std::unordered_set<i32> const& cave1, std::unordered_set<i32> const& cave2) const -> bool;

    auto get_index(point_i pos) const -> i32
    {
        return pos.Y * _grid.width() + pos.X;
    }
    auto get_point(isize idx) const -> point_i
    {
        auto const width = _grid.width(); // Get the grid width.
        return {static_cast<i32>(idx % width), static_cast<i32>(idx / width)};
    }

    i32  _iterations;
    i32  _neighbors;
    f32  _wallProbability;
    i32  _minRoomSize;
    i32  _maxRoomSize;
    bool _smoothEdges;
    i32  _smoothing;

    tile _floor;
    tile _wall;

    std::vector<std::unordered_set<i32>> _caves;
    grid<tile>                           _grid;
    rng                                  _rng;
};

////////////////////////////////////////////////////////////

class city_walls : public base_layout {
public:
    city_walls(i32 minRoomSize, i32 maxRoomSize);

    auto generate(u64 seed, size_i size) -> grid<tile> override;

private:
    void draw_room(rect_i const& rect);
    void draw_doors();

    i32 _minRoomSize;
    i32 _maxRoomSize;

    tile _floor;
    tile _wall;

    std::unordered_set<rect_i> _rooms;
    grid<tile>                 _grid;
    rng                        _rng;
};

////////////////////////////////////////////////////////////

class maze_with_rooms : public base_layout {
public:
    maze_with_rooms(i32 minRoomSize, i32 maxRoomSize, i32 buildRoomAttempts, f32 connectionChance, f32 windingPercent, bool allowDeadEnds);

    auto generate(u64 seed, size_i size) -> grid<tile> override;

private:
    void grow_maze(point_i start);
    void add_rooms();
    void connect_regions();
    void create_room(rect_i const& room);
    void add_junction(point_i pos);
    void remove_dead_ends();
    auto can_carve(point_i pos, point_i dir) -> bool;
    void start_region();
    void carve(point_i pos);

    i32  _minRoomSize;
    i32  _maxRoomSize;
    i32  _buildRoomAttempts;
    f32  _connectionChance;
    f32  _windingPercent;
    bool _allowDeadEnds;

    tile _floor;
    tile _wall;

    grid<tile>               _grid;
    rng                      _rng;
    grid<std::optional<i32>> _regions;
    i32                      _currentRegion {-1};
};

////////////////////////////////////////////////////////////

class messy_bsp_tree : public base_layout {
public:
    messy_bsp_tree(i32 minRoomSize, i32 maxRoomSize, bool smoothEdges, i32 smoothing, i32 filling);

    auto generate(u64 seed, size_i size) -> grid<tile> override;

private:
    void draw_room(rect_i const& rect);
    void draw_hallway(rect_i const& room1, rect_i const& room2);
    void cleanup_map();
    auto get_adjacent_walls_simple(point_i p) -> i32;

    i32  _minRoomSize;
    i32  _maxRoomSize;
    bool _smoothEdges;
    i32  _smoothing;
    i32  _filling;

    tile _floor;
    tile _wall;

    grid<tile> _grid;
    rng        _rng;
};

////////////////////////////////////////////////////////////

struct leaf {
    using hall_func = std::function<void(rect_i, rect_i)>;
    using room_func = std::function<void(rect_i)>;

    leaf(rect_i const& rect);

    rect_i Rect;

    std::unique_ptr<leaf> Child1 {nullptr};
    std::unique_ptr<leaf> Child2 {nullptr};

    void split_leaf(rng& rng, i32 maxRoomSize);
    void create_rooms(hall_func&& hall, room_func&& room, i32 minRoomSize, i32 maxRoomSize, rng& rng);
    auto get_room(rng& rng) -> rect_i;

private:
    std::optional<rect_i> _room;
};
}
