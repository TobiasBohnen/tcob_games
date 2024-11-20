// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

////////////////////////////////////////////////////////////

enum class element_type : u8 {
    None,
    Liquid,
    Powder,
    Solid,
    Gas
};

////////////////////////////////////////////////////////////
enum class comp_op {
    Equals,
    LessThan,
    LessThanOrEqual,
    GreaterThan,
    GreaterThanOrEqual
};

struct temp_rule {
    f32     Temperature {};
    comp_op Op {};

    u16 TransformTo {};
};

struct neighbor_rule {
    u16 Neighbor {};

    u16 NeighborTransformTo {};
    u16 TransformTo {};
};

////////////////////////////////////////////////////////////

struct element_def final {
    u16         ID;
    std::string Name;

    std::vector<color> Colors {};

    f32 Temperature {20};
    i8  Gravity {};

    f32 ThermalConductivity {0.80f};
    f32 Density {}; // g/cm3
    u8  Dispersion {1};

    std::vector<std::variant<temp_rule, neighbor_rule>> Rules;

    element_type Type {};
};

////////////////////////////////////////////////////////////

class element_grid final {
public:
    element_grid();

    ////////////////////////////////////////////////////////////

    void element(point_i i, element_def const& element, bool useTemp);

    auto swap(point_i i0, point_i i1) -> bool;

    void temperature(point_i i, f32 val);

    ////////////////////////////////////////////////////////////

    auto id(point_i i) const -> u16;

    auto type(point_i i) const -> element_type;

    auto gravity(point_i i) const -> i8;

    auto thermal_conductivity(point_i i) const -> f32;

    auto density(point_i i) const -> f32;

    auto dispersion(point_i i) const -> u8;

    auto touched(point_i i) const -> bool;

    auto temperature(point_i i) const -> f32;

    auto color(point_i i) const -> tcob::color;
    auto colors() const -> tcob::color const*;

    ////////////////////////////////////////////////////////////

    void reset_moved();
    void clear();

    void load(io::istream& stream);
    void save(io::ostream& stream) const;

    auto contains(point_i p) const -> bool;
    auto size() const -> size_i;
    auto empty(point_i i) const -> bool;

private:
    struct cell final {
        u16          ID {0};
        element_type Type {element_type::None};
        i8           Gravity {0};
        f32          ThermalConductivity {0.8f};
        f32          Density {0};
        u8           Dispersion {0};
        bool         Touched {false};
    };

    template <typename T>
    using grid = static_grid<T, GRID_SIZE.Width, GRID_SIZE.Height>;

    grid<cell>        _grid;
    grid<f32>         _gridTemperature;
    grid<tcob::color> _gridColors;

    rng _rand;
};

////////////////////////////////////////////////////////////

class element_system final {
public:
    element_system(std::vector<element_def> const& elements);

    auto info_name(point_i i) const -> std::string;
    auto info_heat(point_i i) const -> f32;

    void update();
    void draw_elements(gfx::texture& tex) const;
    void draw_heatmap(gfx::texture& tex) const;

    void spawn(point_i i, i32 t);
    void clear();
    auto rand(i32 min, i32 max) -> i32;

    void load(io::istream& stream);
    void save(io::ostream& stream) const;

private:
    void update_temperature();

    void update_grid();
    void process_rules(point_i i, element_def const& element);
    void process_gravity(point_i i, element_type elementType);

    auto higher_density(point_i i, f32 t) const -> bool;
    auto lower_density(point_i i, f32 t) const -> bool;
    auto id_to_element(u16 t) const -> element_def const*;

    void run_parallel(auto&& func);

    element_grid _grid;

    rng _rand;

    std::unordered_map<u16, element_def> _elements;
};

inline void element_system::run_parallel(auto&& func)
{
    i32 const gridSize {GRID_SIZE.Width};
    i32 const quarterGridSize {gridSize / 4};
    i32 const eighthGridSize {gridSize / 8};

    locate_service<task_manager>().run_parallel(
        [&](par_task const& ctx) {
            isize const idx {ctx.Thread};
            i32 const   xStart {eighthGridSize * static_cast<i32>(idx)}, xEnd {eighthGridSize * static_cast<i32>(idx + 1)};
            i32 const   yStart {idx % 2 == 0 ? 0 : quarterGridSize}, yEnd {idx % 2 == 0 ? quarterGridSize : gridSize};

            for (i32 y {yEnd - 1}; y >= yStart; --y) {
                if (y % 2 == 0) {
                    for (i32 x {xStart}; x < xEnd; ++x) { func({x, y}); }
                } else {
                    for (i32 x {xEnd - 1}; x >= xStart; --x) { func({x, y}); }
                }
            }
        },
        8);

    locate_service<task_manager>().run_parallel(
        [&](par_task const& ctx) {
            isize const idx {ctx.Thread};
            i32 const   xStart {eighthGridSize * static_cast<i32>(idx)}, xEnd {eighthGridSize * static_cast<i32>(idx + 1)};
            i32 const   yStart {idx % 2 != 0 ? 0 : quarterGridSize}, yEnd {idx % 2 != 0 ? quarterGridSize : gridSize};

            for (i32 y {yEnd - 1}; y >= yStart; --y) {
                if (y % 2 == 0) {
                    for (i32 x {xStart}; x < xEnd; ++x) { func({x, y}); }
                } else {
                    for (i32 x {xEnd - 1}; x >= xStart; --x) { func({x, y}); }
                }
            }
        },
        8);
}
