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

    void clear_grid(grid<tile>& grid, rng& rng) const;

    void draw_horizontal_tunnel(grid<tile>& grid, i32 x0, i32 x1, i32 y, tile const& floor) const;
    void draw_vertical_tunnel(grid<tile>& grid, i32 x, i32 y0, i32 y1, tile const& floor) const;
    void draw_room(grid<tile>& grid, rect_i const& rect, tile const& floor) const;
    void draw_hallway(grid<tile>& grid, rect_i const& room1, rect_i const& room2, tile const& floor, bool b) const;
};

////////////////////////////////////////////////////////////

class tunneling : public base_layout {
public:
    tunneling(i32 maxRooms, i32 minSize, i32 maxSize);

    auto generate(u64 seed, size_i size) -> grid<tile> override;

private:
    i32 _maxRooms;
    i32 _minSize;
    i32 _maxSize;
};

////////////////////////////////////////////////////////////

class bsp_tree : public base_layout {
public:
    bsp_tree(i32 minSize, i32 maxSize);

    auto generate(u64 seed, size_i size) -> grid<tile> override;

private:
    i32 _minSize;
    i32 _maxSize;
};

////////////////////////////////////////////////////////////

class drunkards_walk : public base_layout {
public:
    drunkards_walk(f32 percentGoal, i32 walkIterations, f32 weightedTowardCenter, f32 weightedTowardPreviousDirection);

    auto generate(u64 seed, size_i size) -> grid<tile> override;

private:
    void walk(grid<tile>& grid, rng& rng);

    f32 _percentGoal;
    i32 _walkIterations;
    f32 _weightedTowardCenter;
    f32 _weightedTowardPreviousDirection;

    i32       _filled {0};
    f32       _filledGoal {0};
    direction _previousDirection {direction::None};
    i32       _drunkardX {0};
    i32       _drunkardY {0};
};

////////////////////////////////////////////////////////////

class cellular_automata : public base_layout { }; // TODO

////////////////////////////////////////////////////////////

class room_addition : public base_layout { }; // TODO

////////////////////////////////////////////////////////////

class city_walls : public base_layout { }; // TODO

////////////////////////////////////////////////////////////

class maze_with_rooms : public base_layout { }; // TODO

////////////////////////////////////////////////////////////

class messy_bsp_tree : public base_layout { }; // TODO

////////////////////////////////////////////////////////////

struct leaf {
    leaf(rect_i const& rect);

    rect_i Rect;

    std::shared_ptr<leaf> Child1 {nullptr};
    std::shared_ptr<leaf> Child2 {nullptr};

    auto split_leaf(rng& rng) -> bool;
    void create_rooms(base_layout* layout, grid<tile>& grid, i32 minSize, i32 maxSize, rng& rng);
    auto get_room(rng& rng) -> rect_i;

private:
    std::optional<rect_i> _room;
};
}
