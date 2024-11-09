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
    _drawOrder.reserve(_grid.Elements.size());
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

void element_system::update()
{
    _grid.Moved.fill(0);
    update_temperature();
    update_grid();
}

void element_system::draw_image(gfx::image& img) const
{
    auto const getColor {[&](point_i i, element_def const& el) {
        rng       rand {i.X * 73856093 ^ i.Y * 19349663 ^ 12345u};
        i32 const idx {rand(0, static_cast<i32>(el.Colors.size() - 1))};
        return el.Colors[idx];
    }};

    auto const size {img.get_info().Size};
    for (i32 x {0}; x < size.Width; ++x) {
        for (i32 y {0}; y < size.Height; ++y) {
            point_i const pos {x, y};
            img.set_pixel(pos, getColor(pos, *id_to_element(_grid.Elements[pos])));
        }
    }
}

void element_system::draw_heatmap(gfx::image& img) const
{
    static auto colors {gfx::color_gradient {{0, colors::Blue}, {0.5f, colors::White}, {1, colors::Red}}.get_colors()};
    auto const  size {img.get_info().Size};
    for (i32 x {0}; x < size.Width; ++x) {
        for (i32 y {0}; y < size.Height; ++y) {
            point_i const pos {x, y};

            f32 temp {_grid.temperature(pos)};
            temp = std::clamp((temp + 200) / 400, 0.f, 1.f); //-200 to 1200
            img.set_pixel(pos, colors[static_cast<u8>(temp * 255)]);
        }
    }
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
                _grid.Elements[pos]    = t;
                _grid.Temperature[pos] = element->Temperature;
            }
        }
        break;
    case element_type::Liquid:
        for (i32 x {0}; x < 50; ++x) {
            point_i const pos {static_cast<i32>((i.X + rand(-5, 5))), static_cast<i32>((i.Y + rand(-5, 5)))};

            if (!_grid.contains(pos)) { continue; }
            _grid.Elements[pos]    = t;
            _grid.Temperature[pos] = element->Temperature;
        }
        break;
    case element_type::Powder:
        for (i32 x {0}; x < 10; ++x) {
            point_i const pos {static_cast<i32>((i.X + rand(-5, 5))), static_cast<i32>((i.Y + rand(-5, 5)))};

            if (!_grid.contains(pos)) { continue; }
            _grid.Elements[pos]    = t;
            _grid.Temperature[pos] = element->Temperature;
        }
        break;
    case element_type::Gas:
        for (i32 x {0}; x < 100; ++x) {
            point_i const pos {static_cast<i32>((i.X + rand(-5, 5))), static_cast<i32>((i.Y + rand(-5, 5)))};

            if (!_grid.contains(pos)) { continue; }
            _grid.Elements[pos]    = t;
            _grid.Temperature[pos] = element->Temperature;
        }
        break;
    }
}

void element_system::update_temperature()
{
    f32 const  alpha {0.25f};
    auto const size {_grid.Temperature.get_extent()};

    grid<f32> tempGrid {size};

    // Main interior loop without bounds checks
    for (i32 x {1}; x < size.Width - 1; ++x) {
        for (i32 y {1}; y < size.Height - 1; ++y) {
            point_i const pos {x, y};
            f32           avgTemp {0};
            for (auto const& neighbor : NEIGHBORS) {
                avgTemp += _grid.Temperature[pos + neighbor];
            }
            avgTemp /= NEIGHBORS.size();
            tempGrid[pos] = _grid.Temperature[pos] + alpha * (avgTemp - _grid.Temperature[pos]);
        }
    }

    // Edge processing for top and bottom rows
    for (i32 x {0}; x < size.Width; ++x) {
        // Top row
        point_i const topPos {x, 0};
        f32 const     topAvgTemp {_grid.Temperature[topPos + point_i {0, 1}]}; // Only down neighbor
        tempGrid[topPos] = _grid.Temperature[topPos] + alpha * (topAvgTemp - _grid.Temperature[topPos]);

        // Bottom row
        point_i const bottomPos {x, size.Height - 1};
        f32 const     bottomAvgTemp {_grid.Temperature[bottomPos + point_i {0, -1}]}; // Only up neighbor
        tempGrid[bottomPos] = _grid.Temperature[bottomPos] + alpha * (bottomAvgTemp - _grid.Temperature[bottomPos]);
    }

    // Edge processing for left and right columns
    for (i32 y {1}; y < size.Height - 1; ++y) {
        // Left column
        point_i const leftPos {0, y};
        f32 const     leftAvgTemp {_grid.Temperature[leftPos + point_i {1, 0}]}; // Only right neighbor
        tempGrid[leftPos] = _grid.Temperature[leftPos] + alpha * (leftAvgTemp - _grid.Temperature[leftPos]);

        // Right column
        point_i const rightPos {size.Width - 1, y};
        f32 const     rightAvgTemp {_grid.Temperature[rightPos + point_i {-1, 0}]}; // Only left neighbor
        tempGrid[rightPos] = _grid.Temperature[rightPos] + alpha * (rightAvgTemp - _grid.Temperature[rightPos]);
    }

    _grid.Temperature = tempGrid;
}

void element_system::update_grid()
{
    //_shuffle(_drawOrder);
    for (auto const& pos : _drawOrder) {
        if (_grid.Moved[pos] == 1) { continue; }

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
                        if (temp == transition.Temperature) { _grid.id(i, transition.Target); }
                        break;
                    case math_op::LessThan:
                        if (temp < transition.Temperature) { _grid.id(i, transition.Target); }
                        break;
                    case math_op::LessThanOrEqual:
                        if (temp <= transition.Temperature) { _grid.id(i, transition.Target); }
                        break;
                    case math_op::GreaterThan:
                        if (temp > transition.Temperature) { _grid.id(i, transition.Target); }
                        break;
                    case math_op::GreaterThanOrEqual:
                        if (temp >= transition.Temperature) { _grid.id(i, transition.Target); }
                        break;
                    }
                },
                [&](neighbor_transition const& transition) {
                    for (auto const& neighbor : NEIGHBORS) {
                        point_i const np {i + neighbor};
                        if (_grid.contains(np)) {
                            i32 const eid {_grid.Elements[np]};
                            if (eid == transition.Neighbor) {
                                _grid.id(np, transition.Target);
                                _grid.id(i, 0);
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
            swap(i, down); // Swap positions with the less dense liquid below
            return;
        }

        // Attempt to move down diagonally to the right or left if possible
        bool const downRightCheck {canPassThrough(downRight)};
        bool const downLeftCheck {canPassThrough(downLeft)};

        // If both down-right and down-left are valid moves, pick one randomly
        if (downRightCheck && downLeftCheck) {
            if (rand(0, 1) == 0) {
                swap(i, downRight); // Move down-right
            } else {
                swap(i, downLeft);  // Move down-left
            }
            return;
        }

        // If only down-right is valid, move there
        if (downRightCheck) {
            swap(i, downRight);
            return;
        }

        // If only down-left is valid, move there
        if (downLeftCheck) {
            swap(i, downLeft);
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
            swap(i, down);
            return;
        }

        // Attempt to move diagonally down-right or down-left
        i32 const disp {dispersion(i)};

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
                    swap(i, dr);
                } else {
                    swap(i, dl);
                }
                return;
            }

            // If only down-right is less dense, move there
            if (downRightCheck) {
                swap(i, dr);
                return;
            }

            // If only down-left is less dense, move there
            if (downLeftCheck) {
                swap(i, dl);
                return;
            }
        }

        // Try to move horizontally to the left or right if both are empty
        if (_grid.empty(right) && _grid.empty(left)) {
            if (rand(0, 1) == 1) {
                swap(i, right); // Move right
            } else {
                swap(i, left);  // Move left
            }
            return;
        }

        // If only right is empty, move there
        if (_grid.empty(right)) {
            swap(i, right);
            return;
        }

        // If only left is empty, move there
        if (_grid.empty(left)) {
            swap(i, left);
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
            swap(i, up);
            return;
        }

        // Attempt to move diagonally up-right or up-left
        bool const upRightCheck {canPassThrough(upRight)};
        bool const upLeftCheck {canPassThrough(upLeft)};

        // Try up-right or up-left if both are less dense
        if (upRightCheck && upLeftCheck) {
            if (rand(0, 1) == 0) {
                swap(i, upRight);
            } else {
                swap(i, upLeft);
            }
            return;
        }

        // If only up-right is less dense, move there
        if (upRightCheck) {
            swap(i, upRight);
            return;
        }

        // If only up-left is less dense, move there
        if (upLeftCheck) {
            swap(i, upLeft);
            return;
        }

        // Try to move horizontally to the left or right if both are empty
        if (_grid.empty(right) && _grid.empty(left)) {
            if (rand(0, 1) == 1) {
                swap(i, right); // Move right
            } else {
                swap(i, left);  // Move left
            }
            return;
        }

        // If only right is empty, move there
        if (_grid.empty(right)) {
            swap(i, right);
            return;
        }

        // If only left is empty, move there
        if (_grid.empty(left)) {
            swap(i, left);
            return;
        }

        return;
    }
}

void element_system::swap(point_i i0, point_i i1)
{
    if (!_grid.contains(i0) || !_grid.contains(i1)) { return; }

    auto const id0 {_grid.id(i0)};
    auto const id1 {_grid.id(i1)};

    if (id0 == id1) { return; }
    if (_grid.Moved[i0] == 1 && id0 != EMPTY_ELEMENT) { return; }
    if (_grid.Moved[i1] == 1 && id1 != EMPTY_ELEMENT) { return; }

    auto const set {[&](point_i i, i32 val) {
        _grid.Elements[i] = val;
        _grid.Moved[i]    = val == EMPTY_ELEMENT ? 0 : 1;
    }};

    set(i0, id1);
    set(i1, id0);
}

auto element_system::name(point_i i) const -> std::string
{
    return id_to_element(_grid.id(i))->Name;
}

auto element_system::higher_density(point_i i, f32 t) const -> bool
{
    return density(i) > t;
}

auto element_system::lower_density(point_i i, f32 t) const -> bool
{
    return density(i) < t;
}

auto element_system::density(point_i i) const -> f32
{
    return id_to_element(_grid.id(i))->Density;
}

auto element_system::dispersion(point_i i) const -> i32
{
    return id_to_element(_grid.id(i))->Dispersion;
}

auto element_system::is_type(point_i i, element_type t) const -> bool
{
    return type(i) == t;
}

auto element_system::type(point_i i) const -> element_type
{
    return id_to_element(_grid.id(i))->Type;
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
    : Elements {size, 0}
    , Moved {size, 0}
    , Temperature {size, 20}
    , _size {size}
{
}

auto element_grid::contains(point_i p) const -> bool
{
    return _size.contains(p);
}

auto element_grid::empty(point_i i) const -> bool
{
    return id(i) == EMPTY_ELEMENT;
}

auto element_grid::id(point_i i) const -> i32
{
    if (!contains(i)) { return EMPTY_ELEMENT; }
    return Elements[i];
}

void element_grid::id(point_i i, i32 val)
{
    if (!contains(i)) { return; }

    Elements[i] = val;
    Moved[i]    = val == EMPTY_ELEMENT ? 0 : 1;
}

auto element_grid::temperature(point_i i) const -> f32
{
    if (!contains(i)) { return 0; }
    return Temperature[i];
}

void element_grid::temperature(point_i i, f32 val)
{
    if (!contains(i)) { return; }
    Temperature[i] = val;
}
