// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Common.hpp"

namespace Rogue {

auto term_to_grid(point_i pos, point_i center) -> point_i
{
    return pos - point_i {TermMapSize.Width / 2, TermMapSize.Height / 2} + center;
}

auto grid_to_term(point_i pos, point_i center) -> point_i
{
    return pos + point_i {TermMapSize.Width / 2, TermMapSize.Height / 2} - center;
}

auto get_target(point_i pos, direction dir) -> point_i
{
    switch (dir) {
    case direction::Left: return pos + point_i {-1, 0};
    case direction::Right: return pos + point_i {1, 0};
    case direction::Up: return pos + point_i {0, -1};
    case direction::Down: return pos + point_i {0, 1};
    default: break;
    }

    return pos;
}

////////////////////////////////////////////////////////////

}