// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

////////////////////////////////////////////////////////////

enum class element_type {
    None,
    Liquid,
    Powder,
    Solid,
    Gas
};

class element_system;

struct element_def {
    i32         ID;
    std::string Name;

    color Color {};
    i32   ColorVariation {};
    i32   SpawnCount {};

    bool         DefaultGravity {};
    f32          Density {}; // g/cm3
    element_type Type {};

    lua::function<void> Update {};
};

////////////////////////////////////////////////////////////

class element_system {
    friend class element_def;

public:
    element_system(size_i size, script_element_vec const& elements);

    void update();
    void update_image(gfx::image& img);

    void spawn(point_i i, i32 t);

    auto rand(i32 min, i32 max) -> i32;

    void swap(point_i i0, point_i i1);

    void clear(point_i i);
    auto empty(point_i i) -> bool;

    auto id(point_i i) -> i32;
    auto name(point_i i) -> std::string;
    auto density(point_i i) -> f32;
    auto type(point_i i) -> element_type;

private:
    void process(i32 x, i32 y);
    void process_gravity(point_i i, element_def const& element);

    auto get_element(i32 t) -> element_def const*;

    grid<i32>                   _grid;
    grid<u8>                    _gridMoved;
    std::unordered_set<point_i> _dirtyPixel;

    rng _rand;

    std::vector<point_i>     _shufflePoints;
    random::shuffle<point_i> _shuffle;

    std::vector<element_def> _elements;
};
