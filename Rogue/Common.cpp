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

////////////////////////////////////////////////////////////

}