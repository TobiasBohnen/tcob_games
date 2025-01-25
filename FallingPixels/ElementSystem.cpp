// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ElementSystem.hpp"

template <typename T>
auto comp(comp_op op, T a, T b) -> bool
{
    switch (op) {
    case comp_op::Equals: return a == b;
    case comp_op::LessThan: return a < b;
    case comp_op::LessThanOrEqual: return a <= b;
    case comp_op::GreaterThan: return a > b;
    case comp_op::GreaterThanOrEqual: return a > b;
    }

    return false;
}

static constexpr std::array<point_i, 8> NEIGHBORS {
    {{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {1, 1}, {1, -1}, {-1, 1}}};

element_system::element_system(std::vector<element_def> const& elements)
    : _grid {}
{
    for (auto const& el : elements) {
        _elements[el.Element.ID] = el;
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

    static auto colors {gfx::color_gradient {{0, colors::Blue}, {0.5f, colors::White}, {1, colors::Red}}.colors()};

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

    switch (element->Element.Type) {
    case element_type::None:
    case element_type::Solid:
        for (i32 x {0}; x < 10; ++x) {
            for (i32 y {0}; y < 10; ++y) {
                point_i const pos {i.X - 5 + x, i.Y - 5 + y};

                if (!_grid.contains(pos)) { continue; }
                _grid.set(pos, *element, true);
            }
        }
        break;
    case element_type::Liquid:
        for (i32 x {0}; x < 50; ++x) {
            point_i const pos {static_cast<i32>((i.X + rand(-5, 5))), static_cast<i32>((i.Y + rand(-5, 5)))};

            if (!_grid.contains(pos)) { continue; }
            _grid.set(pos, *element, true);
        }
        break;
    case element_type::Powder:
        for (i32 x {0}; x < 10; ++x) {
            point_i const pos {static_cast<i32>((i.X + rand(-5, 5))), static_cast<i32>((i.Y + rand(-5, 5)))};

            if (!_grid.contains(pos)) { continue; }
            _grid.set(pos, *element, true);
        }
        break;
    case element_type::Gas:
        for (i32 x {0}; x < 100; ++x) {
            point_i const pos {static_cast<i32>((i.X + rand(-5, 5))), static_cast<i32>((i.Y + rand(-5, 5)))};

            if (!_grid.contains(pos)) { continue; }
            _grid.set(pos, *element, true);
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
            if (element->Element.Gravity != 0) { process_gravity(pos, element->Element.Type); }
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
                        _grid.set(i, _elements[r.Result], false);
                    }
                },
                [&](neighbor_rule const& r) {
                    for (auto const& neighbor : NEIGHBORS) {
                        point_i const np {i + neighbor};
                        if (_grid.touched(np)) { continue; }

                        i32 const eid {_grid.id(np)};
                        if (eid == r.Element || (r.Element == ANY_ELEMENT && eid != EMPTY_ELEMENT)) { // ANY but empty
                            _grid.set(np, _elements[r.NeighborResult], true);
                            _grid.set(i, _elements[r.Result], true);
                            return;
                        }
                    }
                },
                [&](dissolve_rule const& r) {
                    for (auto const& neighbor : NEIGHBORS) {
                        point_i const np {i + neighbor};
                        if (_grid.touched(np) || !_grid.dissolvable(np)) { continue; }

                        i32 const eid {_grid.id(np)};
                        if (eid == r.Element || (r.Element == ANY_ELEMENT && eid != EMPTY_ELEMENT)) { // ANY but empty
                            _grid.set(np, _elements[EMPTY_ELEMENT], false);
                            _grid.set(i, _elements[r.Result], false);
                            return;
                        }
                    }
                }},
            rule);
    }
}

void element_system::process_gravity(point_i i, element_type elementType)
{
    if (elementType != element_type::Liquid && elementType != element_type::Gas && elementType != element_type::Powder) {
        return;
    }

    if (_grid.touched(i)) { return; }

    i8 const      gravity {_grid.gravity(i)};
    point_i const down {i + point_i {0, gravity}};
    point_i const downRight {i + point_i {1, gravity}};
    point_i const downLeft {i + point_i {-1, gravity}};

    point_i const right {i + point_i {1, 0}};
    point_i const left {i + point_i {-1, 0}};

    f32 const  density {_grid.density(i)};
    auto const canPassThrough {[&](point_i pos) {
        if (_grid.empty(pos)) { return true; }

        auto const type {_grid.type(pos)};
        if (elementType == element_type::Liquid || elementType == element_type::Gas) {
            return type != element_type::Solid && lower_density(pos, density);
        }
        if (elementType == element_type::Powder) {
            return type != element_type::Solid && type != element_type::Powder && lower_density(pos, density);
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

    if (elementType != element_type::Powder) {
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
    _grid.fill({});
    _gridTemperature.fill(20); // default ambient temp
    _gridColors.fill(tcob::colors::Black);
    _gridTouched.fill(false);
}

void element_grid::load(io::istream& stream)
{
    auto const size {_grid.size()};
    for (usize i {0}; i < size; ++i) {
        _grid[i]            = stream.read<element>();
        _gridTemperature[i] = stream.read<f32>();
        _gridColors[i]      = stream.read<tcob::color>();
    }
    _gridTouched.fill(false);
}

void element_grid::save(io::ostream& stream) const
{
    auto const size {_grid.size()};
    for (usize i {0}; i < size; ++i) {
        stream.write(_grid[i]);
        stream.write(_gridTemperature[i]);
        stream.write(_gridColors[i]);
    }
}

void element_grid::set(point_i i, element_def const& element, bool useTemp)
{
    if (!contains(i)) { return; }

    _grid[i]        = element.Element;
    _gridTouched[i] = true;

    if (useTemp) {
        _gridTemperature[i] = element.Temperature;
    }

    _gridColors[i] = element.Colors[_rand(0, static_cast<i32>(element.Colors.size() - 1))];
}

auto element_grid::swap(point_i i0, point_i i1) -> bool
{
    if (!contains(i0) || !contains(i1)) { return false; }

    auto const id0 {id(i0)};
    auto const id1 {id(i1)};

    if (id0 == id1) { return true; }
    if (_gridTouched[i1] && id1 != EMPTY_ELEMENT) { return false; } // prevent teleportation

    std::swap(_grid[i0], _grid[i1]);
    std::swap(_gridTemperature[i0], _gridTemperature[i1]);
    std::swap(_gridColors[i0], _gridColors[i1]);

    _gridTouched[i0] = id1 != EMPTY_ELEMENT;
    _gridTouched[i1] = id0 != EMPTY_ELEMENT;

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
    return _grid[i].ID;
}

auto element_grid::type(point_i i) const -> element_type
{
    if (!contains(i)) { return element_type::None; }
    return _grid[i].Type;
}

auto element_grid::gravity(point_i i) const -> i8
{
    if (!contains(i)) { return 0; }
    return _grid[i].Gravity;
}

auto element_grid::thermal_conductivity(point_i i) const -> f32
{
    if (!contains(i)) { return 0; }
    return _grid[i].ThermalConductivity;
}

auto element_grid::density(point_i i) const -> f32
{
    if (!contains(i)) { return 0; }
    return _grid[i].Density;
}

auto element_grid::dispersion(point_i i) const -> u8
{
    if (!contains(i)) { return 0; }
    return _grid[i].Dispersion;
}

auto element_grid::dissolvable(point_i i) const -> bool
{
    if (!contains(i)) { return false; }
    return _grid[i].Dissolvable;
}

auto element_grid::touched(point_i i) const -> bool
{
    if (!contains(i)) { return true; }
    return _gridTouched[i];
}

auto element_grid::temperature(point_i i) const -> f32
{
    if (!contains(i)) { return 0; }
    return _gridTemperature[i];
}

auto element_grid::color(point_i i) const -> tcob::color
{
    if (!contains(i)) { return colors::Transparent; }
    return _gridColors[i];
}

auto element_grid::colors() const -> tcob::color const*
{
    return _gridColors.data();
}

void element_grid::reset_moved()
{
    _gridTouched.fill(false);
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
