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
    f32     Temperature {};
    math_op Op {};
    i32     Target {};
};

struct neighbor_transition {
    i32 Neighbor {};
    i32 Target {};
};

////////////////////////////////////////////////////////////

struct element_def final {
    i32         ID;
    std::string Name;

    std::vector<color> Colors {};

    f32 Temperature {20};

    f32 Gravity {1.0f}; // TODO
    f32 Density {};     // g/cm3
    i32 Dispersion {1};

    std::vector<std::variant<temp_transition, neighbor_transition>> Transitions;

    element_type Type {};
};

////////////////////////////////////////////////////////////
class element_grid final {
public:
    element_grid(size_i size);

    tcob::grid<i32> Elements;
    tcob::grid<u8>  Moved;

    tcob::grid<f32> Temperature;

    auto contains(point_i p) const -> bool;

    auto empty(point_i i) const -> bool;

    auto id(point_i i) const -> i32;
    void id(point_i i, i32 val);

    auto temperature(point_i i) const -> f32;
    void temperature(point_i i, f32 val);

private:
    size_i _size;
};

////////////////////////////////////////////////////////////

class element_system final {
public:
    element_system(size_i size, std::vector<element_def> const& elements);

    void update();
    void draw_image(gfx::image& img) const;
    void draw_heatmap(gfx::image& img) const;

    void spawn(point_i i, i32 t);

    auto rand(i32 min, i32 max) -> i32;

    void swap(point_i i0, point_i i1);

    auto name(point_i i) const -> std::string;

    auto higher_density(point_i i, f32 t) const -> bool;
    auto lower_density(point_i i, f32 t) const -> bool;
    auto density(point_i i) const -> f32;
    auto dispersion(point_i i) const -> i32;

    auto is_type(point_i i, element_type t) const -> bool;
    auto type(point_i i) const -> element_type;

private:
    void update_grid();
    void update_temperature();
    void process_transitions(point_i i, element_def const& element);
    void process_gravity(point_i i, element_def const& element);

    auto id_to_element(i32 t) const -> element_def const*;

    element_grid _grid;

    rng _rand;

    std::vector<point_i>     _drawOrder;
    random::shuffle<point_i> _shuffle;

    std::unordered_map<i32, element_def> _elements;
};
