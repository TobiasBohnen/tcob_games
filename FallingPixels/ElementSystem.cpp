// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ElementSystem.hpp"

constexpr i32                           EMPTY_ELEMENT {0};
static constexpr std::array<point_i, 4> NEIGHBORS {{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};

element_system::element_system(std::vector<element_def> const& elements)
    : _grid {}
{
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
    _grid.clear();
}

void element_system::update_temperature()
{
    run_parallel([&](point_i pos) {
        f32 const alpha {_grid.thermal_conductivity(pos)};
        f32       avgTemp {0};
        for (auto const& neighbor : NEIGHBORS) {
            avgTemp += _grid.temperature(pos + neighbor);
        }
        avgTemp /= NEIGHBORS.size();
        f32 const currentTemp {_grid.temperature(pos)};
        _grid.temperature(pos, currentTemp + alpha * (avgTemp - currentTemp));
    });
}

void element_system::update_grid()
{
    run_parallel([&](point_i pos) {
        if (_grid.touched(pos)) { return; }
        if (auto elementID {_grid.id(pos)}; elementID != EMPTY_ELEMENT) {

            auto const* element {id_to_element(elementID)};
            if (!element) { return; }

            // rules
            if (!element->Rules.empty()) { process_rules(pos, *element); }

            // gravity
            if (element->Gravity != 0) { process_gravity(pos, *element); }
        }
    });
}

void element_system::process_rules(point_i i, element_def const& element)
{
    for (auto const& rule : element.Rules) {
        if (_grid.touched(i)) { return; }

        std::visit(
            overloaded {
                [&](temp_rule const& r) {
                    f32 const temp(_grid.temperature(i));
                    if (comp(r.Op, temp, r.Temperature)) {
                        _grid.element(i, _elements[r.TransformTo], false);
                    }
                },
                [&](neighbor_rule const& r) {
                    for (auto const& neighbor : NEIGHBORS) {
                        point_i const np {i + neighbor};
                        if (!_grid.contains(np) || _grid.touched(np)) { continue; }

                        i32 const eid {_grid.id(np)};
                        if (eid == r.Neighbor) {
                            _grid.element(np, _elements[r.NeighborTransformTo], true);
                            _grid.element(i, _elements[r.TransformTo], true);
                            return;
                        }
                    }
                }},
            rule);
    }
}

void element_system::process_gravity(point_i i, element_def const& element)
{
    if (element.Type != element_type::Liquid && element.Type != element_type::Gas && element.Type != element_type::Powder) {
        return;
    }

    if (_grid.touched(i)) { return; }

    point_i const down {i + point_i {0, element.Gravity}};
    point_i const downRight {i + point_i {1, element.Gravity}};
    point_i const downLeft {i + point_i {-1, element.Gravity}};

    point_i const right {i + point_i {1, 0}};
    point_i const left {i + point_i {-1, 0}};

    auto const canPassThrough {[&](point_i pos) {
        if (_grid.empty(pos)) { return true; }

        auto const type {_grid.type(pos)};
        if (element.Type == element_type::Liquid || element.Type == element_type::Gas) {
            return type != element_type::Solid && lower_density(pos, element.Density);
        }
        if (element.Type == element_type::Powder) {
            return type != element_type::Solid && type != element_type::Powder && lower_density(pos, element.Density);
        }

        return false;
    }};

    // Try to move directly down if the cell below is less dense
    if (canPassThrough(down)) {
        if (_grid.swap(i, down)) { return; }
    }

    // Attempt to move diagonally down-right or down-left
    u8 const disp {_grid.dispersion(i)};

    bool rightFree {true};
    bool leftFree {true};
    for (u8 j {0}; j < disp; ++j) {
        rightFree = rightFree && canPassThrough(right + point_i {j, 0});
        leftFree  = leftFree && canPassThrough(left + point_i {-j, 0});

        point_i const dr {downRight + point_i {j, 0}};
        bool const    downRightCheck {rightFree && canPassThrough(dr)};
        point_i const dl {downLeft + point_i {-j, 0}};
        bool const    downLeftCheck {leftFree && canPassThrough(dl)};

        // Try down-right or down-left if both are less dense
        if (downRightCheck && downLeftCheck) {
            if (rand(0, 1) == 0) {
                if (_grid.swap(i, dr)) { return; }
                if (_grid.swap(i, dl)) { return; }
            } else {
                if (_grid.swap(i, dl)) { return; }
                if (_grid.swap(i, dr)) { return; }
            }
        }

        // If only down-right is less dense, move there
        if (downRightCheck) {
            if (_grid.swap(i, dr)) { return; }
        }

        // If only down-left is less dense, move there
        if (downLeftCheck) {
            if (_grid.swap(i, dl)) { return; }
        }
    }

    if (element.Type != element_type::Powder) {
        // Try to move horizontally to the left or right if both are empty
        if (_grid.empty(right) && _grid.empty(left)) {
            if (rand(0, 1) == 0) {
                if (_grid.swap(i, right)) { return; }
                if (_grid.swap(i, left)) { return; }
            } else {
                if (_grid.swap(i, left)) { return; }
                if (_grid.swap(i, right)) { return; }
            }
        }

        // If only right is empty, move there
        if (_grid.empty(right)) {
            if (_grid.swap(i, right)) { return; }
        }

        // If only left is empty, move there
        if (_grid.empty(left)) {
            if (_grid.swap(i, left)) { return; }
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

auto element_system::rand(i32 min, i32 max) -> i32
{
    return _rand(min, max);
}

void element_system::load(io::istream& stream)
{
    _grid.load(stream);
}

void element_system::save(io::ostream& stream) const
{
    _grid.save(stream);
}

auto element_system::id_to_element(u16 t) const -> element_def const*
{
    auto it {_elements.find(t)};
    if (it == _elements.end()) { return nullptr; }
    return &it->second;
}

////////////////////////////////////////////////////////////

element_grid::element_grid()
{
    clear();
}

void element_grid::clear()
{
    _gridElements.fill(0);
    _gridTypes.fill(element_type::None);
    _gridThermalConductivity.fill(0.8f);
    _gridDensity.fill(0);
    _gridDispersion.fill(0);
    _gridColor.fill(colors::Black);
    _gridTouched.fill(0);
    _gridTemperature.fill(20);
}

void element_grid::load(io::istream& stream)
{
    auto const size {_gridElements.size()};
    for (usize i {0}; i < size; ++i) {
        _gridElements[i]            = stream.read<decltype(_gridElements)::type>();
        _gridTypes[i]               = stream.read<decltype(_gridTypes)::type>();
        _gridThermalConductivity[i] = stream.read<decltype(_gridThermalConductivity)::type>();
        _gridDensity[i]             = stream.read<decltype(_gridDensity)::type>();
        _gridDispersion[i]          = stream.read<decltype(_gridDispersion)::type>();
        _gridColor[i]               = stream.read<decltype(_gridColor)::type>();
        _gridTouched[i]             = stream.read<decltype(_gridTouched)::type>();
        _gridTemperature[i]         = stream.read<decltype(_gridTemperature)::type>();
    }
}

void element_grid::save(io::ostream& stream) const
{
    auto const size {_gridElements.size()};
    for (usize i {0}; i < size; ++i) {
        stream.write(_gridElements[i]);
        stream.write(_gridTypes[i]);
        stream.write(_gridThermalConductivity[i]);
        stream.write(_gridDensity[i]);
        stream.write(_gridDispersion[i]);
        stream.write(_gridColor[i]);
        stream.write(_gridTouched[i]);
        stream.write(_gridTemperature[i]);
    }
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

    _gridTouched[i] = 1;

    if (useTemp) {
        _gridTemperature[i] = element.BaseTemperature;
    }
}

auto element_grid::swap(point_i i0, point_i i1) -> bool
{
    if (!contains(i0) || !contains(i1)) { return false; }

    auto const id0 {id(i0)};
    auto const id1 {id(i1)};

    if (id0 == id1) { return true; }
    if (_gridTouched[i1] == 1 && id1 != EMPTY_ELEMENT) { return false; } // prevent teleportation

    std::swap(_gridElements[i0], _gridElements[i1]);
    std::swap(_gridTypes[i0], _gridTypes[i1]);
    std::swap(_gridThermalConductivity[i0], _gridThermalConductivity[i1]);
    std::swap(_gridDensity[i0], _gridDensity[i1]);
    std::swap(_gridDispersion[i0], _gridDispersion[i1]);
    std::swap(_gridColor[i0], _gridColor[i1]);

    std::swap(_gridTemperature[i0], _gridTemperature[i1]);

    _gridTouched[i0] = id1 == EMPTY_ELEMENT ? 0 : 1;
    _gridTouched[i1] = id0 == EMPTY_ELEMENT ? 0 : 1;

    return true;
}

void element_grid::temperature(point_i i, f32 val)
{
    if (!contains(i)) { return; }
    _gridTemperature[i] = val;
}

auto element_grid::id(point_i i) const -> u16
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

auto element_grid::dispersion(point_i i) const -> u8
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

auto element_grid::touched(point_i i) const -> bool
{
    if (!contains(i)) { return false; }
    return _gridTouched[i] == 1;
}

auto element_grid::temperature(point_i i) const -> f32
{
    if (!contains(i)) { return 0; }
    return _gridTemperature[i];
}

void element_grid::reset_moved()
{
    _gridTouched.fill(0);
}

auto element_grid::contains(point_i p) const -> bool
{
    return size().contains(p);
}

auto element_grid::size() const -> size_i
{
    return GRID_SIZE;
}

auto element_grid::empty(point_i i) const -> bool
{
    return id(i) == EMPTY_ELEMENT;
}
