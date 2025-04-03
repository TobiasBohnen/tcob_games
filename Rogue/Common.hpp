// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

// IWYU pragma: always_keep

#include <tcob/tcob.hpp>

namespace Rogue {

using namespace tcob;
using namespace std::chrono_literals;
using namespace tcob::literals;

constinit inline size_i TermSize {40, 40};

////////////////////////////////////////////////////////////

struct tile {
    std::string Floor;
    color       ForegroundColor {colors::White};
    color       BackgroundColor {colors::Black};
    bool        Seen {false};
    bool        InSight {false};
    bool        Passable {false};
};

////////////////////////////////////////////////////////////

struct object {
    std::string Symbol;
    color       Color {colors::White};
    point_i     Position {};
};
struct monster {
    std::string Symbol;
    color       Color {colors::White};
    point_i     Position {};
};
struct player {
    color   Color {colors::White};
    point_i Position {};
};

////////////////////////////////////////////////////////////

auto line_of_sight(point_i start, point_i end, grid<tile> const& tiles) -> bool;

auto term_to_grid(point_i pos, point_i center) -> point_i;
auto grid_to_term(point_i pos, point_i center) -> point_i;

auto get_target(point_i pos, direction dir) -> point_i;

}
