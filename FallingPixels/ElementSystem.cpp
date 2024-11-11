// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ElementSystem.hpp"

constexpr i32                           EMPTY_ELEMENT {0};
static constexpr std::array<point_i, 4> NEIGHBORS {{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};

element_system::element_system(size_i size, std::vector<element_def> const& elements)
    : _grid {size}
{
    _drawOrder.reserve(size.Width * size.Height);
    for (i32 y {size.Height - 1}; y >= 0; --y) {
        if (y % 2 == 0) {
            for (i32 x {0}; x < size.Width; ++x) {
                _drawOrder.emplace_back(x, y);
            }
        } else {
            for (i32 x {size.Width - 1}; x >= 0; --x) {
                _drawOrder.emplace_back(x, y);
            }
        }
    }

    for (auto const& el : elements) {
        _elements[el.ID] = el;
    }
}

auto element_system::info_name(point_i i) const -> std::string
{
    return id_to_element(_grid.id(i))->Name;
}

auto element_system::info_heat(point_i i) const -> f32
{
    return _grid.temperature(i);
}

void element_system::update()
{
    _grid.reset_moved();
    update_temperature();
    update_grid();
}

void element_system::draw_elements(gfx::texture& tex) const
{
    tex.update_data(point_i::Zero, _grid.size(), _grid.colors(), 0);
}

void element_system::draw_heatmap(gfx::texture& tex) const
{
    auto const size {_grid.size()};
    auto       img {gfx::image::CreateEmpty(size, gfx::image::format::RGBA)};

    static auto colors {gfx::color_gradient {{0, colors::Blue}, {0.5f, colors::White}, {1, colors::Red}}.get_colors()};

    for (i32 x {0}; x < size.Width; ++x) {
        for (i32 y {0}; y < size.Height; ++y) {
            point_i const pos {x, y};
            f32           temp {_grid.temperature(pos)};
            temp = 0.5f + temp / (temp < 0 ? 400 : 1200); // -200 to 600
            temp = std::clamp(temp, 0.f, 1.f);
            img.set_pixel(pos, colors[static_cast<u8>(temp * 255)]);
        }
    }

    tex.update_data(point_i::Zero, size, img.buffer().data(), 0);
}

void element_system::spawn(point_i i, i32 t)
{
    auto const* element {id_to_element(t)};

    switch (element->Type) {
    case element_type::None:
    case element_type::Solid:
        for (i32 x {0}; x < 10; ++x) {
            for (i32 y {0}; y < 10; ++y) {
                point_i const pos {i.X - 5 + x, i.Y - 5 + y};

                if (!_grid.contains(pos)) { continue; }
                _grid.element(pos, *element, true);
            }
        }
        break;
    case element_type::Liquid:
        for (i32 x {0}; x < 50; ++x) {
            point_i const pos {static_cast<i32>((i.X + rand(-5, 5))), static_cast<i32>((i.Y + rand(-5, 5)))};

            if (!_grid.contains(pos)) { continue; }
            _grid.element(pos, *element, true);
        }
        break;
    case element_type::Powder:
        for (i32 x {0}; x < 10; ++x) {
            point_i const pos {static_cast<i32>((i.X + rand(-5, 5))), static_cast<i32>((i.Y + rand(-5, 5)))};

            if (!_grid.contains(pos)) { continue; }
            _grid.element(pos, *element, true);
        }
        break;
    case element_type::Gas:
        for (i32 x {0}; x < 100; ++x) {
            point_i const pos {static_cast<i32>((i.X + rand(-5, 5))), static_cast<i32>((i.Y + rand(-5, 5)))};

            if (!_grid.contains(pos)) { continue; }
            _grid.element(pos, *element, true);
        }
        break;
    }
}

void element_system::clear()
{
    _grid = {_grid.size()};
}

void element_system::update_temperature()
{
    auto const size {_grid.size()};

    // Main interior loop without bounds checks
    for (i32 x {1}; x < size.Width - 1; ++x) {
        for (i32 y {1}; y < size.Height - 1; ++y) {
            point_i const pos {x, y};
            f32 const     alpha {_grid.thermal_conductivity(pos)};
            f32           avgTemp {0};
            for (auto const& neighbor : NEIGHBORS) {
                avgTemp += _grid.temperature(pos + neighbor);
            }
            avgTemp /= NEIGHBORS.size();
            f32 const currentTemp {_grid.temperature(pos)};
            _grid.temperature(pos, currentTemp + alpha * (avgTemp - currentTemp));
        }
    }

    // Edge processing for top and bottom rows
    for (i32 x {0}; x < size.Width; ++x) {
        // Top row
        point_i const topPos {x, 0};
        f32 const     topAlpha {_grid.thermal_conductivity(topPos)};
        f32 const     topAvgTemp {_grid.temperature(topPos + point_i {0, 1})}; // Only down neighbor
        _grid.temperature(topPos, _grid.temperature(topPos) + topAlpha * (topAvgTemp - _grid.temperature(topPos)));

        // Bottom row
        point_i const bottomPos {x, size.Height - 1};
        f32 const     bottomAlpha {_grid.thermal_conductivity(bottomPos)};
        f32 const     bottomAvgTemp {_grid.temperature(bottomPos + point_i {0, -1})}; // Only up neighbor
        _grid.temperature(bottomPos, _grid.temperature(bottomPos) + bottomAlpha * (bottomAvgTemp - _grid.temperature(bottomPos)));
    }

    // Edge processing for left and right columns
    for (i32 y {1}; y < size.Height - 1; ++y) {
        // Left column
        point_i const leftPos {0, y};
        f32 const     leftAlpha {_grid.thermal_conductivity(leftPos)};
        f32 const     leftAvgTemp {_grid.temperature(leftPos + point_i {1, 0})}; // Only right neighbor
        _grid.temperature(leftPos, _grid.temperature(leftPos) + leftAlpha * (leftAvgTemp - _grid.temperature(leftPos)));

        // Right column
        point_i const rightPos {size.Width - 1, y};
        f32 const     rightAlpha {_grid.thermal_conductivity(rightPos)};
        f32 const     rightAvgTemp {_grid.temperature(rightPos + point_i {-1, 0})}; // Only left neighbor
        _grid.temperature(rightPos, _grid.temperature(rightPos) + rightAlpha * (rightAvgTemp - _grid.temperature(rightPos)));
    }
}

void element_system::update_grid()
{
    _shuffle(_drawOrder);
    for (auto const& pos : _drawOrder) {
        if (_grid.moved(pos) == 1) { continue; }

        if (auto elementID {_grid.id(pos)}; elementID != EMPTY_ELEMENT) {

            auto const* element {id_to_element(elementID)};
            if (!element) { continue; }

            // transitions
            process_transitions(pos, *element);
            // gravity
            if (element->Gravity > 0) { process_gravity(pos, *element); }
        }
    }
}

void element_system::process_transitions(point_i i, element_def const& element)
{
    for (auto const& transition : element.Transitions) {
        std::visit(
            overloaded {
                [&](temp_transition const& transition) {
                    f32 const temp(_grid.temperature(i));
                    switch (transition.Op) {
                    case math_op::Equals:
                        if (temp == transition.Temperature) { _grid.element(i, _elements[transition.TransformTo], false); }
                        break;
                    case math_op::LessThan:
                        if (temp < transition.Temperature) { _grid.element(i, _elements[transition.TransformTo], false); }
                        break;
                    case math_op::LessThanOrEqual:
                        if (temp <= transition.Temperature) { _grid.element(i, _elements[transition.TransformTo], false); }
                        break;
                    case math_op::GreaterThan:
                        if (temp > transition.Temperature) { _grid.element(i, _elements[transition.TransformTo], false); }
                        break;
                    case math_op::GreaterThanOrEqual:
                        if (temp >= transition.Temperature) { _grid.element(i, _elements[transition.TransformTo], false); }
                        break;
                    }
                },
                [&](neighbor_transition const& transition) {
                    for (auto const& neighbor : NEIGHBORS) {
                        point_i const np {i + neighbor};
                        if (_grid.contains(np)) {
                            i32 const eid {_grid.id(np)};
                            if (eid == transition.Neighbor) {
                                _grid.element(np, _elements[transition.NeighborTransformTo], true);
                                _grid.element(i, _elements[transition.TransformTo], true);
                                return;
                            }
                        }
                    }
                }},
            transition);
    }
}

void element_system::process_gravity(point_i i, element_def const& element)
{
    point_i const up {i + point_i {0, -1}};
    point_i const upRight {i + point_i {1, -1}};
    point_i const upLeft {i + point_i {-1, -1}};

    point_i const down {i + point_i {0, 1}};
    point_i const downRight {i + point_i {1, 1}};
    point_i const downLeft {i + point_i {-1, 1}};

    point_i const right {i + point_i {1, 0}};
    point_i const left {i + point_i {-1, 0}};

    // Powder
    if (element.Type == element_type::Powder) {
        auto const canPassThrough {[&](point_i pos) {
            return _grid.empty(pos) || (!is_type(pos, element_type::Solid) && !is_type(pos, element_type::Powder) && lower_density(pos, element.Density));
        }};

        // Attempt to move down if the cell below is a liquid or gas and less dense than the current element
        if (canPassThrough(down)) {
            _grid.swap(i, down); // Swap positions with the less dense liquid below
            return;
        }

        // Attempt to move down diagonally to the right or left if possible
        bool const downRightCheck {canPassThrough(downRight)};
        bool const downLeftCheck {canPassThrough(downLeft)};

        // If both down-right and down-left are valid moves, pick one randomly
        if (downRightCheck && downLeftCheck) {
            if (rand(0, 1) == 0) {
                _grid.swap(i, downRight); // Move down-right
            } else {
                _grid.swap(i, downLeft);  // Move down-left
            }
            return;
        }

        // If only down-right is valid, move there
        if (downRightCheck) {
            _grid.swap(i, downRight);
            return;
        }

        // If only down-left is valid, move there
        if (downLeftCheck) {
            _grid.swap(i, downLeft);
            return;
        }

        return; // No valid moves found, remain in the current position
    }

    // Liquid
    if (element.Type == element_type::Liquid) {
        auto const canPassThrough {[&](point_i pos) {
            return _grid.empty(pos) || (!is_type(pos, element_type::Solid) && lower_density(pos, element.Density));
        }};

        // Try to move directly down if the cell below is less dense
        if (canPassThrough(down)) {
            _grid.swap(i, down);
            return;
        }

        // Attempt to move diagonally down-right or down-left
        i32 const disp {_grid.dispersion(i)};

        bool rightFree {true};
        bool leftFree {true};
        for (i32 j {0}; j < disp; ++j) {
            rightFree = rightFree && canPassThrough(right + point_i {j, 0});
            leftFree  = leftFree && canPassThrough(left + point_i {-j, 0});

            point_i const dr {downRight + point_i {j, 0}};
            bool const    downRightCheck {rightFree && canPassThrough(dr)};
            point_i const dl {downLeft + point_i {-j, 0}};
            bool const    downLeftCheck {leftFree && canPassThrough(dl)};

            // Try down-right or down-left if both are less dense
            if (downRightCheck && downLeftCheck) {
                if (rand(0, 1) == 0) {
                    _grid.swap(i, dr);
                } else {
                    _grid.swap(i, dl);
                }
                return;
            }

            // If only down-right is less dense, move there
            if (downRightCheck) {
                _grid.swap(i, dr);
                return;
            }

            // If only down-left is less dense, move there
            if (downLeftCheck) {
                _grid.swap(i, dl);
                return;
            }
        }

        // Try to move horizontally to the left or right if both are empty
        if (_grid.empty(right) && _grid.empty(left)) {
            if (rand(0, 1) == 1) {
                _grid.swap(i, right); // Move right
            } else {
                _grid.swap(i, left);  // Move left
            }
            return;
        }

        // If only right is empty, move there
        if (_grid.empty(right)) {
            _grid.swap(i, right);
            return;
        }

        // If only left is empty, move there
        if (_grid.empty(left)) {
            _grid.swap(i, left);
            return;
        }
        return;
    }

    // Gas
    if (element.Type == element_type::Gas) {
        auto const canPassThrough {[&](point_i pos) {
            return _grid.empty(pos) || (!is_type(pos, element_type::Solid) && lower_density(pos, element.Density));
        }};

        // Try to move directly up if the cell above is less dense
        if (canPassThrough(up)) {
            _grid.swap(i, up);
            return;
        }

        // Attempt to move diagonally up-right or up-left
        bool const upRightCheck {canPassThrough(upRight)};
        bool const upLeftCheck {canPassThrough(upLeft)};

        // Try up-right or up-left if both are less dense
        if (upRightCheck && upLeftCheck) {
            if (rand(0, 1) == 0) {
                _grid.swap(i, upRight);
            } else {
                _grid.swap(i, upLeft);
            }
            return;
        }

        // If only up-right is less dense, move there
        if (upRightCheck) {
            _grid.swap(i, upRight);
            return;
        }

        // If only up-left is less dense, move there
        if (upLeftCheck) {
            _grid.swap(i, upLeft);
            return;
        }

        // Try to move horizontally to the left or right if both are empty
        if (_grid.empty(right) && _grid.empty(left)) {
            if (rand(0, 1) == 1) {
                _grid.swap(i, right); // Move right
            } else {
                _grid.swap(i, left);  // Move left
            }
            return;
        }

        // If only right is empty, move there
        if (_grid.empty(right)) {
            _grid.swap(i, right);
            return;
        }

        // If only left is empty, move there
        if (_grid.empty(left)) {
            _grid.swap(i, left);
            return;
        }

        return;
    }
}

auto element_system::higher_density(point_i i, f32 t) const -> bool
{
    return _grid.density(i) > t;
}

auto element_system::lower_density(point_i i, f32 t) const -> bool
{
    return _grid.density(i) < t;
}
auto element_system::is_type(point_i i, element_type t) const -> bool
{
    return _grid.type(i) == t;
}

auto element_system::rand(i32 min, i32 max) -> i32
{
    return _rand(min, max);
}

auto element_system::id_to_element(i32 t) const -> element_def const*
{
    auto it {_elements.find(t)};
    if (it == _elements.end()) { return nullptr; }
    return &it->second;
}

////////////////////////////////////////////////////////////

element_grid::element_grid(size_i size)
    : _gridElements {size, 0}
    , _gridTypes {size, element_type::None}
    , _gridThermalConductivity {size, 0.8f}
    , _gridDensity {size, 0}
    , _gridDispersion {size, 0}
    , _gridColor {size, colors::Transparent}
    , _gridMoved {size, 0}
    , _gridTemperature {size, 20}

    , _size {size}
{
}

void element_grid::element(point_i i, element_def const& element, bool useTemp)
{
    if (!contains(i)) { return; }

    _gridElements[i]            = element.ID;
    _gridTypes[i]               = element.Type;
    _gridThermalConductivity[i] = element.ThermalConductivity;
    _gridDensity[i]             = element.Density;
    _gridDispersion[i]          = element.Dispersion;
    _gridColor[i]               = element.Colors[_rand(0, static_cast<i32>(element.Colors.size() - 1))];

    _gridMoved[i] = element.ID == EMPTY_ELEMENT ? 0 : 1;

    if (useTemp) {
        _gridTemperature[i] = element.BaseTemperature;
    }
}

void element_grid::swap(point_i i0, point_i i1)
{
    if (!contains(i0) || !contains(i1)) { return; }

    auto const id0 {id(i0)};
    auto const id1 {id(i1)};

    if (id0 == id1) { return; }
    if (_gridMoved[i0] == 1 && id0 != EMPTY_ELEMENT) { return; }
    if (_gridMoved[i1] == 1 && id1 != EMPTY_ELEMENT) { return; }

    std::swap(_gridElements[i0], _gridElements[i1]);
    std::swap(_gridTypes[i0], _gridTypes[i1]);
    std::swap(_gridThermalConductivity[i0], _gridThermalConductivity[i1]);
    std::swap(_gridDensity[i0], _gridDensity[i1]);
    std::swap(_gridDispersion[i0], _gridDispersion[i1]);
    std::swap(_gridColor[i0], _gridColor[i1]);

    std::swap(_gridTemperature[i0], _gridTemperature[i1]);

    _gridMoved[i0] = id1 == EMPTY_ELEMENT ? 0 : 1;
    _gridMoved[i1] = id0 == EMPTY_ELEMENT ? 0 : 1;
}

void element_grid::temperature(point_i i, f32 val)
{
    if (!contains(i)) { return; }
    _gridTemperature[i] = val;
}

auto element_grid::id(point_i i) const -> i32
{
    if (!contains(i)) { return EMPTY_ELEMENT; }
    return _gridElements[i];
}

auto element_grid::type(point_i i) const -> element_type
{
    if (!contains(i)) { return element_type::None; }
    return _gridTypes[i];
}

auto element_grid::thermal_conductivity(point_i i) const -> f32
{
    if (!contains(i)) { return 0; }
    return _gridThermalConductivity[i];
}

auto element_grid::density(point_i i) const -> f32
{
    if (!contains(i)) { return 0; }
    return _gridDensity[i];
}

auto element_grid::dispersion(point_i i) const -> i32
{
    if (!contains(i)) { return 0; }
    return _gridDispersion[i];
}

auto element_grid::color(point_i i) const -> tcob::color
{
    if (!contains(i)) { return colors::Transparent; }
    return _gridColor[i];
}

auto element_grid::colors() const -> tcob::color const*
{
    return _gridColor.data();
}

auto element_grid::moved(point_i i) const -> bool
{
    if (!contains(i)) { return false; }
    return _gridMoved[i];
}

auto element_grid::temperature(point_i i) const -> f32
{
    if (!contains(i)) { return 0; }
    return _gridTemperature[i];
}

void element_grid::reset_moved()
{
    _gridMoved.fill(0);
}

auto element_grid::contains(point_i p) const -> bool
{
    return _size.contains(p);
}

auto element_grid::size() const -> size_i
{
    return _size;
}

auto element_grid::empty(point_i i) const -> bool
{
    return id(i) == EMPTY_ELEMENT;
}
