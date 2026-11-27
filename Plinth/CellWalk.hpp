// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

inline constexpr f64 INFINITE_DIST {1e30};

class cell_walk {
public:
    cell_walk(point_d origin, point_d dir);

    auto cell() const -> point_i;
    auto entry() const -> f64;
    auto exit() const -> f64;
    auto side() const -> bool;

    auto dir_to_previous() const -> point_i;
    void advance();

private:
    point_i _cell;
    point_i _step;
    point_d _delta;
    point_d _sideDist;
    bool    _side;
    f64     _entry {0.0};
    bool    _first {true};
};
