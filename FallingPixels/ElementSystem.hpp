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

////////////////////////////////////////////////////////////
enum class math_op {
    Equals,
    LessThan,
    LessThanOrEqual,
    GreaterThan,
    GreaterThanOrEqual
};
struct temp_transition {
    f32         Temperature {};
    math_op     Op {};
    std::string Target {};
};

struct neighbor_transition {
    std::string Neighbor {};
    std::string Target {};
};

////////////////////////////////////////////////////////////

struct element_def {
    i32         ID;
    std::string Name;

    std::vector<color> Colors {};

    f32 Temperature {20};

    f32 Gravity {1.0f}; // TODO
    f32 Density {};     // g/cm3

    std::vector<std::variant<temp_transition, neighbor_transition>> Transitions;

    element_type Type {};
};

////////////////////////////////////////////////////////////

class element_system {
public:
    element_system(size_i size);

    void set_elements(std::vector<element_def> const& elements);

    void update();
    void draw_image(gfx::image& img);
    void draw_heatmap(gfx::image& img);

    void spawn(point_i i, i32 t);

    auto rand(i32 min, i32 max) -> i32;

    void swap(point_i i0, point_i i1);

    auto empty(point_i i) -> bool;

    auto temperature(point_i i) -> f32;
    void temperature(point_i i, f32 val);

    auto id(point_i i) -> i32;
    void id(point_i i, std::string const& val);
    void id(point_i i, i32 val);

    auto name(point_i i) -> std::string;
    auto density(point_i i) -> f32;
    auto type(point_i i) -> element_type;

private:
    void update_grid();
    void update_temperature();
    void process_transitions(point_i i, element_def const& element);
    void process_gravity(point_i i, element_def const& element);

    auto id_to_element(i32 t) -> element_def const*;
    auto name_to_id(std::string const& name) const -> i32;

    grid<i32> _gridElements;
    grid<f32> _gridTemperature;
    grid<u8>  _gridMoved;

    rng _rand;

    std::vector<point_i>     _shufflePoints;
    random::shuffle<point_i> _shuffle;

    std::unordered_map<i32, element_def> _elements;
};
