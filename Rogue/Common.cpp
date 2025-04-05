// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Common.hpp"

namespace Rogue {

auto is_line_of_sight(point_i start, point_i end, grid<tile> const& tiles) -> bool
{
    auto [x0, y0] {start};
    auto const [x1, y1] {end};
    i32 const dx {std::abs(x1 - x0)};
    i32 const dy {std::abs(y1 - y0)};
    i32 const sx {(x0 < x1) ? 1 : -1};
    i32 const sy {(y0 < y1) ? 1 : -1};
    i32       err {dx - dy};

    for (;;) {
        if (x0 == x1 && y0 == y1) { break; }

        i32 const e2 {2 * err};
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }

        point_i const pos {x0, y0};
        if (pos != end && !tile_type_traits::passable(tiles[pos].Type)) { return false; }
    }
    return true;
}

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