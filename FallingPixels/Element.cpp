// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Element.hpp"

element_system::element_system(size_i size)
    : _grid {size, element_type::Empty}
    , _gridMoved {size, 0}
{
    _shufflePoints.reserve(_grid.size());
    for (i32 x {0}; x < size.Width; ++x) {
        for (i32 y {0}; y < size.Height; ++y) {
            _shufflePoints.emplace_back(x, y);
        }
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

    if (auto elementType {get(pos)}; elementType != element_type::Empty) {
        auto const& element {get_element(elementType)};

        if (element.Gravity > 0) {
            point_i const down {pos + point_i {0, 1}};
            point_i       downRight {pos + point_i {1, 1}};
            point_i       downLeft {pos + point_i {-1, 1}};
            point_i const left {pos + point_i {-1, 0}};
            point_i const right {pos + point_i {1, 0}};

            // down
            if (empty(down)) {
                swap(pos, down);
                return;
            }

            if (element.State == aggregate_state::Solid) {
                if (state(down) == aggregate_state::Liquid && element.Density > density(down)) {
                    swap(pos, down);
                    return;
                }

                // down left/right
                bool const downRightCheck {empty(downRight) || (state(downRight) == aggregate_state::Liquid && element.Density > density(downRight))};
                bool const downLeftCheck {empty(downLeft) || (state(downLeft) == aggregate_state::Liquid && element.Density > density(downLeft))};
                if (downRightCheck && downLeftCheck) {
                    if (rand(0, 1) == 0) {
                        swap(pos, downRight);
                    } else {
                        swap(pos, downLeft);
                    }
                    return;
                }
                if (downRightCheck) {
                    swap(pos, downRight);
                    return;
                }
                if (downLeftCheck) {
                    swap(pos, downLeft);
                    return;
                }
                return;
            }

            if (element.State == aggregate_state::Liquid) {
                // down
                if (element.Density > density(down)) {
                    swap(pos, down);
                    return;
                }

                // left/right
                if (empty(right) && empty(left)) {
                    if (rand(0, 1) == 1) {
                        swap(pos, right);
                    } else {
                        swap(pos, left);
                    }
                    return;
                }

                if (empty(right)) {
                    swap(pos, right);
                    return;
                }
                if (empty(left)) {
                    swap(pos, left);
                    return;
                }

                // down left/right
                for (i32 i {0}; i < 3; ++i) {
                    if (element.Density > density(downRight) && element.Density > density(downLeft)) {
                        if (rand(0, 1) == 0) {
                            swap(pos, downRight);
                        } else {
                            swap(pos, downLeft);
                        }
                        return;
                    }
                    if (element.Density > density(downRight)) {
                        swap(pos, downRight);
                        return;
                    }
                    if (element.Density > density(downLeft)) {
                        swap(pos, downLeft);
                        return;
                    }

                    downRight += point_i {1, 0};
                    downLeft += point_i {-1, 0};
                }
                return;
            }

            return;
        }

        element.Update(*this, x, y);
    }
}

void element_system::update_image(gfx::image& img)
{
    if (_dirtyPixel.empty()) { return; }

    for (auto const& pos : _dirtyPixel) {
        img.set_pixel(pos, get_element(_grid[pos]).Color);
    }

    _dirtyPixel.clear();
}

void element_system::force_set(point_i i, element_type t)
{
    if (!_grid.contains(i)) { return; }
    _grid[i] = t;
    _dirtyPixel.insert(i);
}

auto element_system::get(point_i i) -> element_type
{
    if (!_grid.contains(i)) { return element_type::Empty; }
    return _grid[i];
}

void element_system::swap(point_i i0, point_i i1)
{
    if (!_gridMoved.contains(i0)) { return; }
    if (!_gridMoved.contains(i1)) { return; }

    auto const t0 {get(i0)};
    auto const t1 {get(i1)};

    if (t0 == t1) { return; }
    if (_gridMoved[i0] == 1 && t0 != element_type::Empty) { return; }
    if (_gridMoved[i1] == 1 && t1 != element_type::Empty) { return; }

    auto const set {[&](point_i i, element_type t) {
        _grid[i] = t;
        _dirtyPixel.insert(i);
        if (t != element_type::Empty) { _gridMoved[i] = 1; }
    }};

    set(i0, t1);
    set(i1, t0);
}

auto element_system::empty(point_i i) -> bool
{
    return get(i) == element_type::Empty;
}

auto element_system::density(point_i i) -> i32
{
    return get_element(get(i)).Density;
}

auto element_system::state(point_i i) -> aggregate_state
{
    return get_element(get(i)).State;
}

auto element_system::rand(i32 min, i32 max) -> i32
{
    return _rand(min, max);
}

auto element_system::get_element(element_type t) -> element const&
{
    switch (t) {
    case element_type::Sand: {
        static element e {
            .Color   = colors::SandyBrown,
            .Gravity = 1,
            .Density = 11,
            .State   = aggregate_state::Solid,
            .Update  = [](element_system& system, i32 x, i32 y) {},
        };
        return e;
    } break;
    case element_type::Sawdust: {
        static element e {
            .Color   = color::FromString("#998970"),
            .Gravity = 1,
            .Density = 9,
            .State   = aggregate_state::Solid,
            .Update  = [](element_system& system, i32 x, i32 y) {},
        };
        return e;
    } break;
    case element_type::Water: {
        static element e {
            .Color   = colors::Blue,
            .Gravity = 1,
            .Density = 10,
            .State   = aggregate_state::Liquid,
            .Update  = [](element_system& system, i32 x, i32 y) {},
        };
        return e;
    } break;
    case element_type::Wall: {
        static element e {
            .Color   = colors::White,
            .Gravity = 0,
            .Density = 100,
            .State   = aggregate_state::Solid,
            .Update  = [](element_system& system, i32 x, i32 y) {},
        };
        return e;
    } break;
    case element_type::Empty:
    default: {
        static element e {
            .Color   = colors::Transparent,
            .Gravity = 0,
            .Density = 0,
            .State   = aggregate_state::None,
        };
        return e;
    } break;
    }
}
