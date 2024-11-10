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
    i32     TransformTo {};
};

struct neighbor_transition {
    i32 Neighbor {};
    i32 NeighborTransformTo {};
    i32 TransformTo {};
};

////////////////////////////////////////////////////////////

struct element_def final {
    i32         ID;
    std::string Name;

    std::vector<color> Colors {};

    f32 BaseTemperature {20};
    f32 Gravity {1.0f}; // TODO

    f32 ThermalConductivity {0.80f};
    f32 Density {};     // g/cm3
    i32 Dispersion {1}; // liquid

    std::vector<std::variant<temp_transition, neighbor_transition>> Transitions;

    element_type Type {};
};

////////////////////////////////////////////////////////////

class element_grid final {
public:
    element_grid(size_i size);

    ////////////////////////////////////////////////////////////

    void element(point_i i, element_def const& element, bool useTemp);

    void swap(point_i i0, point_i i1);

    void temperature(point_i i, f32 val);

    ////////////////////////////////////////////////////////////

    auto id(point_i i) const -> i32;

    auto type(point_i i) const -> element_type;

    auto thermal_conductivity(point_i i) const -> f32;

    auto density(point_i i) const -> f32;

    auto dispersion(point_i i) const -> i32;

    auto color(point_i i) const -> tcob::color;
    auto colors() const -> tcob::color const*;

    auto moved(point_i i) const -> bool;

    auto temperature(point_i i) const -> f32;

    ////////////////////////////////////////////////////////////

    void reset_moved();

    auto contains(point_i p) const -> bool;
    auto size() const -> size_i;
    auto empty(point_i i) const -> bool;

private:
    grid<i32>          _gridElements;
    grid<element_type> _gridTypes;
    grid<f32>          _gridThermalConductivity;
    grid<f32>          _gridDensity;
    grid<f32>          _gridDispersion;
    grid<tcob::color>  _gridColor;
    grid<u8>           _gridMoved;

    grid<f32> _gridTemperature;

    size_i _size;
    rng    _rand;
};

////////////////////////////////////////////////////////////

class element_system final {
public:
    element_system(size_i size, std::vector<element_def> const& elements);

    auto info_name(point_i i) const -> std::string;
    auto info_heat(point_i i) const -> f32;

    void update();
    void draw_image(gfx::image& img) const;
    void draw_heatmap(gfx::image& img) const;

    void spawn(point_i i, i32 t);
    void clear();
    auto rand(i32 min, i32 max) -> i32;

private:
    void update_temperature();

    void update_grid();
    void process_transitions(point_i i, element_def const& element);
    void process_gravity(point_i i, element_def const& element);

    auto is_type(point_i i, element_type t) const -> bool;
    auto higher_density(point_i i, f32 t) const -> bool;
    auto lower_density(point_i i, f32 t) const -> bool;
    auto id_to_element(i32 t) const -> element_def const*;

    element_grid _grid;

    rng _rand;

    std::vector<point_i>     _drawOrder;
    random::shuffle<point_i> _shuffle;

    std::unordered_map<i32, element_def> _elements;
};
