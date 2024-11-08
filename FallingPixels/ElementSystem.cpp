// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ElementSystem.hpp"

constexpr i32 EMPTY_ELEMENT {0};

element_system::element_system(size_i size)
    : _gridElements {size, 0}
    , _gridTemperature {size, 20}
    , _gridMoved {size, 0}
{
    _shufflePoints.reserve(_gridElements.size());
    for (i32 x {0}; x < size.Width; ++x) {
        for (i32 y {0}; y < size.Height; ++y) {
            _shufflePoints.emplace_back(x, y);
        }
    }
}

void element_system::set_elements(std::vector<element_def> const& elements)
{
    for (auto const& el : elements) {
        _elements[el.ID] = el;
    }
}

void element_system::update()
{
    _gridMoved.fill(0);
    process_temperature();
    process_elements();
}

void element_system::draw_image(gfx::image& img)
{
    auto mutate {[&](point_i i, element_def const& el) {
        rng rand {i.X * 73856093 ^ i.Y * 19349663 ^ 12345u};
        if (el.Color.Variation == 0) { return el.Color.Base; }
        u8 const r {static_cast<u8>(std::clamp(el.Color.Base.R + rand(-el.Color.Variation, el.Color.Variation), 0, 255))};
        u8 const g {static_cast<u8>(std::clamp(el.Color.Base.G + rand(-el.Color.Variation, el.Color.Variation), 0, 255))};
        u8 const b {static_cast<u8>(std::clamp(el.Color.Base.B + rand(-el.Color.Variation, el.Color.Variation), 0, 255))};
        return color {r, g, b};
    }};

    auto const size {img.get_info().Size};
    for (i32 x {0}; x < size.Width; ++x) {
        for (i32 y {0}; y < size.Height; ++y) {
            point_i const pos {x, y};
            img.set_pixel(pos, mutate(pos, *get_element(_gridElements[pos])));
        }
    }
}

void element_system::draw_heatmap(gfx::image& img)
{
    static auto colors {gfx::color_gradient {{0, colors::Blue}, {0.5f, colors::White}, {1, colors::Red}}.get_colors()};
    auto const  size {img.get_info().Size};
    for (i32 x {0}; x < size.Width; ++x) {
        for (i32 y {0}; y < size.Height; ++y) {
            point_i const pos {x, y};

            f32 temp {temperature(pos)};
            temp = std::clamp((temp + 200) / 400, 0.f, 1.f); //-200 to 1200
            img.set_pixel(pos, colors[static_cast<u8>(temp * 255)]);
        }
    }
}

void element_system::spawn(point_i i, i32 t)
{
    auto const* element {get_element(t)};

    i32 spread {0};
    switch (element->Type) {
    case element_type::None:
    case element_type::Solid:
        spread = 100;
        break;
    case element_type::Liquid:
        spread = 20;
        break;
    case element_type::Powder:
        spread = 1;
        break;
    case element_type::Gas:
        spread = 1000;
        break;
    }

    for (i32 x {0}; x < spread; ++x) {
        point_i const pos {static_cast<i32>((i.X + rand(-4, 4))), static_cast<i32>((i.Y + rand(-4, 4)))};

        if (!_gridElements.contains(pos)) { continue; }
        _gridElements[pos]    = t;
        _gridTemperature[pos] = element->SpawnHeat;
    }
}

void element_system::process_temperature()
{
    static std::array<point_i, 4> neighbors {{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};

    f32 const  alpha {0.25f};
    auto const size {_gridTemperature.get_extent()};

    for (i32 x {1}; x < size.Width - 1; ++x) {
        for (i32 y {1}; y < size.Height - 1; ++y) {
            point_i const pos {x, y};
            f32           avgTemp {0};
            for (auto const& n : neighbors) {
                avgTemp += _gridTemperature[pos + n];
            }
            avgTemp /= neighbors.size();
            _gridTemperature[pos] = _gridTemperature[pos] + alpha * (avgTemp - _gridTemperature[pos]);
        }
    }
}

void element_system::process_elements()
{
    _shuffle(_shufflePoints);
    for (auto const& pos : _shufflePoints) {
        if (_gridMoved[pos] == 1) { continue; }

        if (auto elementID {id(pos)}; elementID != EMPTY_ELEMENT) {
            auto const* element {get_element(elementID)};
            if (!element) { continue; }

            if (element->Update.is_valid()) {
                if (!element->Update(pos.X, pos.Y)) {
                    continue;
                }
            }

            if (element->Gravity > 0) { process_gravity(pos, *element); }
        }
    }
}

void element_system::process_gravity(point_i i, element_def const& element)
{
    point_i const up {i + point_i {0, -1}};
    point_i       upRight {i + point_i {1, -1}};
    point_i       upLeft {i + point_i {-1, -1}};

    point_i const down {i + point_i {0, 1}};
    point_i       downRight {i + point_i {1, 1}};
    point_i       downLeft {i + point_i {-1, 1}};

    point_i const left {i + point_i {-1, 0}};
    point_i const right {i + point_i {1, 0}};

    // Powder
    if (element.Type == element_type::Powder) {
        // Attempt to move down if the cell below is a liquid or gas and less dense than the current element
        if (empty(down) || (type(down) != element_type::Solid && type(down) != element_type::Powder && element.Density > density(down))) {
            swap(i, down); // Swap positions with the less dense liquid below
            return;
        }

        // Attempt to move down diagonally to the right or left if possible
        bool const downRightCheck {empty(downRight) || (type(downRight) != element_type::Solid && type(downRight) != element_type::Powder && element.Density > density(downRight))};
        bool const downLeftCheck {empty(downLeft) || (type(downLeft) != element_type::Solid && type(downRight) != element_type::Powder && element.Density > density(downLeft))};

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
        // Try to move directly down if the cell below is less dense
        if (empty(down) || (type(down) != element_type::Solid && element.Density > density(down))) {
            swap(i, down);
            return;
        }

        // Try to move horizontally to the left or right if both are empty
        if (empty(right) && empty(left)) {
            if (rand(0, 1) == 1) {
                swap(i, right); // Move right
            } else {
                swap(i, left);  // Move left
            }
            return;
        }

        // If only right is empty, move there
        if (empty(right)) {
            swap(i, right);
            return;
        }

        // If only left is empty, move there
        if (empty(left)) {
            swap(i, left);
            return;
        }

        // Attempt to move diagonally down-right or down-left
        bool const downRightCheck {empty(downRight) || (type(downRight) != element_type::Solid && element.Density > density(downRight))};
        bool const downLeftCheck {empty(downLeft) || (type(downLeft) != element_type::Solid && element.Density > density(downLeft))};

        // Try down-right or down-left if both are less dense
        if (downRightCheck && downLeftCheck) {
            if (rand(0, 1) == 0) {
                swap(i, downRight);
            } else {
                swap(i, downLeft);
            }
            return;
        }

        // If only down-right is less dense, move there
        if (downRightCheck) {
            swap(i, downRight);
            return;
        }

        // If only down-left is less dense, move there
        if (downLeftCheck) {
            swap(i, downLeft);
            return;
        }
        return;
    }

    // Gas
    if (element.Type == element_type::Gas) {
        // Try to move directly up if the cell above is less dense
        if (empty(up) || (type(up) != element_type::Solid && element.Density > density(up))) {
            swap(i, up);
            return;
        }

        // Try to move horizontally to the left or right if both are empty
        if (empty(right) && empty(left)) {
            if (rand(0, 1) == 1) {
                swap(i, right); // Move right
            } else {
                swap(i, left);  // Move left
            }
            return;
        }

        // If only right is empty, move there
        if (empty(right)) {
            swap(i, right);
            return;
        }

        // If only left is empty, move there
        if (empty(left)) {
            swap(i, left);
            return;
        }

        // Attempt to move diagonally up-right or up-left
        bool const upRightCheck {empty(upRight) || (type(upRight) == element_type::Gas && element.Density > density(upRight))};
        bool const upLeftCheck {empty(upLeft) || (type(upLeft) == element_type::Gas && element.Density > density(upLeft))};

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
        return;
    }
}

void element_system::swap(point_i i0, point_i i1)
{
    if (!_gridMoved.contains(i0)) { return; }
    if (!_gridMoved.contains(i1)) { return; }

    auto const id0 {id(i0)};
    auto const id1 {id(i1)};

    if (id0 == id1) { return; }
    if (_gridMoved[i0] == 1 && id0 != EMPTY_ELEMENT) { return; }
    if (_gridMoved[i1] == 1 && id1 != EMPTY_ELEMENT) { return; }

    auto const set {[&](point_i i, i32 t) {
        _gridElements[i] = t;
        if (t != EMPTY_ELEMENT) { _gridMoved[i] = 1; }
    }};

    set(i0, id1);
    set(i1, id0);
}

auto element_system::empty(point_i i) -> bool
{
    return id(i) == EMPTY_ELEMENT;
}

auto element_system::id(point_i i) -> i32
{
    if (!_gridElements.contains(i)) { return EMPTY_ELEMENT; }
    return _gridElements[i];
}

void element_system::id(point_i i, i32 val)
{
    if (!_gridElements.contains(i)) { return; }

    _gridElements[i] = val;
    _gridMoved[i]    = 1;
}

auto element_system::temperature(point_i i) -> f32
{
    if (!_gridTemperature.contains(i)) { return 0; }
    return _gridTemperature[i];
}

void element_system::temperature(point_i i, f32 val)
{
    if (!_gridTemperature.contains(i)) { return; }
    _gridTemperature[i] = val;
}

auto element_system::name(point_i i) -> std::string
{
    return get_element(id(i))->Name;
}

auto element_system::density(point_i i) -> f32
{
    return get_element(id(i))->Density;
}

auto element_system::type(point_i i) -> element_type
{
    return get_element(id(i))->Type;
}

auto element_system::rand(i32 min, i32 max) -> i32
{
    return _rand(min, max);
}

auto element_system::get_element(i32 t) -> element_def const*
{
    auto it {_elements.find(t)};
    if (it == _elements.end()) { return nullptr; }
    return &it->second;
}
