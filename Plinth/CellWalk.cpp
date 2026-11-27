// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "CellWalk.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "Common.hpp"

cell_walk::cell_walk(point_d origin, point_d dir)
    : _cell {WorldToCell(origin)}
    , _step {dir.X < 0 ? -1 : 1, dir.Y < 0 ? -1 : 1}
    , _delta {(dir.X == 0) ? INFINITE_DIST : std::abs(1 / dir.X), (dir.Y == 0) ? INFINITE_DIST : std::abs(1 / dir.Y)}
    , _sideDist {(dir.X < 0) ? (origin.X - _cell.X) * _delta.X : (_cell.X + 1.0 - origin.X) * _delta.X, (dir.Y < 0) ? (origin.Y - _cell.Y) * _delta.Y : (_cell.Y + 1.0 - origin.Y) * _delta.Y}
    , _side {_sideDist.X < _sideDist.Y}
{
}

auto cell_walk::cell() const -> point_i { return _cell; }
auto cell_walk::entry() const -> f64 { return _entry; }
auto cell_walk::exit() const -> f64 { return std::min(_sideDist.X, _sideDist.Y); }
auto cell_walk::side() const -> bool { return _side; }

auto cell_walk::dir_to_previous() const -> point_i
{
    if (_first) { return point_i::Zero; }
    return _side ? point_i {0, -_step.Y} : point_i {-_step.X, 0};
}

void cell_walk::advance()
{
    if (_sideDist.X < _sideDist.Y) {
        _sideDist.X += _delta.X;
        _cell.X += _step.X;
        _side = false;
    } else {
        _sideDist.Y += _delta.Y;
        _cell.Y += _step.Y;
        _side = true;
    }

    _entry = !_side ? _sideDist.X - _delta.X : _sideDist.Y - _delta.Y;
    _first = false;
}
