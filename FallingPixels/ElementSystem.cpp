// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ElementSystem.hpp"

constexpr i32 EMPTY_ELEMENT {0};

element_system::element_system(size_i size, script_element_vec const& elements)
    : _grid {size, 0}
    , _gridMoved {size, 0}

{
    _shufflePoints.reserve(_grid.size());
    for (i32 x {0}; x < size.Width; ++x) {
        for (i32 y {0}; y < size.Height; ++y) {
            _shufflePoints.emplace_back(x, y);
        }
    }

    for (auto [id, name, table] : elements) {
        element_def& element {_elements.emplace_back()};
        element.ID   = id;
        element.Name = name;

        element.Color          = color::FromString(table["Color"].as<std::string>());
        element.ColorVariation = table["ColorVariation"].get<i32>().value_or(0);
        element.SpawnCount     = table["SpawnCount"].get<i32>().value_or(1);

        element.DefaultGravity = table["DefaultGravity"].get<bool>().value_or(true);
        element.Density        = table["Density"].as<f32>();
        element.Type           = table["Type"].as<element_type>();

        table.try_get(element.Update, "Update");
    }
}

void element_system::update()
{
    _gridMoved.fill(0);

    _shuffle(_shufflePoints);

    for (auto const& pos : _shufflePoints) {
        process(pos.X, pos.Y);
    }
}

void element_system::process(i32 x, i32 y)
{
    point_i const pos {x, y};

    if (_gridMoved[pos] == 1) { return; }

    if (auto elementID {id(pos)}; elementID != EMPTY_ELEMENT) {
        auto const* element {get_element(elementID)};
        if (!element) { return; }

        if (element->Update.is_valid()) {
            element->Update(this, pos);
        }

        if (element->DefaultGravity) { process_gravity(pos, *element); }
    }
}

void element_system::process_gravity(point_i i, element_def const& element)
{
    point_i const down {i + point_i {0, 1}};
    point_i       downRight {i + point_i {1, 1}};
    point_i       downLeft {i + point_i {-1, 1}};
    point_i const left {i + point_i {-1, 0}};
    point_i const right {i + point_i {1, 0}};

    // down
    if (empty(down)) {
        swap(i, down);
        return;
    }

    // Powder
    if (element.Type == element_type::Powder) {

        // Attempt to move down if the cell below is a liquid and less dense than the current element
        if (type(down) == element_type::Liquid && element.Density > density(down)) {
            swap(i, down); // Swap positions with the less dense liquid below
            return;
        }

        // Attempt to move down diagonally to the right or left if possible
        bool const downRightCheck {empty(downRight) || (type(downRight) == element_type::Liquid && element.Density > density(downRight))};
        bool const downLeftCheck {empty(downLeft) || (type(downLeft) == element_type::Liquid && element.Density > density(downLeft))};

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
        if (element.Density > density(down)) {
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

        // Attempt to move diagonally down-right or down-left for three steps
        for (i32 x {0}; x < 3; ++x) {
            // Try down-right or down-left if both are less dense
            if (element.Density > density(downRight) && element.Density > density(downLeft)) {
                if (rand(0, 1) == 0) {
                    swap(i, downRight);
                } else {
                    swap(i, downLeft);
                }
                return;
            }

            // If only down-right is less dense, move there
            if (element.Density > density(downRight)) {
                swap(i, downRight);
                return;
            }

            // If only down-left is less dense, move there
            if (element.Density > density(downLeft)) {
                swap(i, downLeft);
                return;
            }

            downRight += point_i {1, 0};
            downLeft += point_i {-1, 0};
        }
        return;
    }
}

void element_system::update_image(gfx::image& img)
{
    if (_dirtyPixel.empty()) { return; }

    auto mutate {
        [&](element_def const& el) {
            if (el.ColorVariation == 0) { return el.Color; }
            u8 const r {static_cast<u8>(std::clamp(el.Color.R + _rand(-el.ColorVariation, el.ColorVariation), 0, 255))};
            u8 const g {static_cast<u8>(std::clamp(el.Color.G + _rand(-el.ColorVariation, el.ColorVariation), 0, 255))};
            u8 const b {static_cast<u8>(std::clamp(el.Color.B + _rand(-el.ColorVariation, el.ColorVariation), 0, 255))};
            return color {r, g, b};
        }};

    for (auto const& pos : _dirtyPixel) {
        img.set_pixel(pos, mutate(*get_element(_grid[pos])));
    }

    _dirtyPixel.clear();
}

void element_system::spawn(point_i i, i32 t)
{
    i32 const spread {get_element(t)->SpawnCount};
    for (i32 x {0}; x < spread; ++x) {
        point_i const pos {static_cast<i32>((i.X + rand(-4, 4))), static_cast<i32>((i.Y + rand(-4, 4)))};

        if (!_grid.contains(pos)) { continue; }
        _grid[pos] = t;
        _dirtyPixel.insert(pos);
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
        _grid[i] = t;
        _dirtyPixel.insert(i);
        if (t != EMPTY_ELEMENT) { _gridMoved[i] = 1; }
    }};

    set(i0, id1);
    set(i1, id0);
}

void element_system::clear(point_i i)
{
    if (!_grid.contains(i)) { return; }

    _grid[i] = EMPTY_ELEMENT;
    _dirtyPixel.insert(i);
    _gridMoved[i] = 1;
}

auto element_system::empty(point_i i) -> bool
{
    return id(i) == EMPTY_ELEMENT;
}

auto element_system::id(point_i i) -> i32
{
    if (!_grid.contains(i)) { return EMPTY_ELEMENT; }
    return _grid[i];
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
    for (auto const& def : _elements) {
        if (def.ID == t) {
            return &def;
        }
    }

    return nullptr;
}
