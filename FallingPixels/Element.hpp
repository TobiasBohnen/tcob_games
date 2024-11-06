// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <tcob/tcob.hpp>

using namespace tcob;

////////////////////////////////////////////////////////////

enum class element_type {
    Empty,
    Sand,
    Sawdust,
    Water,
    Wall,
};
enum class aggregate_state {
    None,
    Liquid,
    Solid,
    Gas
};

////////////////////////////////////////////////////////////

class element_system {
public:
    element_system(size_i size);

    void update();

    void update_image(gfx::image& img);

    void force_set(point_i i, element_type t);

private:
    struct element {
        color           Color {};
        i32             Gravity {};
        i32             Density {};
        aggregate_state State {};

        std::function<void(element_system&, i32, i32)> Update;
    };

    void process(i32 x, i32 y);

    auto rand(i32 min, i32 max) -> i32;

    auto get(point_i i) -> element_type;
    void swap(point_i i0, point_i i1);
    auto empty(point_i i) -> bool;

    auto density(point_i i) -> i32;
    auto state(point_i i) -> aggregate_state;

    auto get_element(element_type t) -> element const&;

    grid<element_type>          _grid;
    grid<u8>                    _gridMoved;
    std::unordered_set<point_i> _dirtyPixel;

    rng _rand;

    std::vector<point_i>     _shufflePoints;
    random::shuffle<point_i> _shuffle;
};
