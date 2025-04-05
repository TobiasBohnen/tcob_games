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

constinit inline size_i TermMapSize {60, 40};
constinit inline size_i TermSize {80, 45};

class level;

////////////////////////////////////////////////////////////

struct object {
    string  Symbol;
    color   Color {colors::White};
    point_i Position {};
};
struct monster {
    string  Symbol;
    color   Color {colors::White};
    point_i Position {};
};

////////////////////////////////////////////////////////////
enum class tile_type {
    Floor,
    Wall
};

struct tile_type_traits {
    static auto passable(tile_type type) -> bool
    {
        switch (type) {
        case tile_type::Floor: return true;
        case tile_type::Wall: return false;
        }
        return false;
    }
};

struct tile {
    tile_type Type;
    string    Symbol;
    color     ForegroundColor {colors::White};
    color     BackgroundColor {colors::Black};
    bool      Seen {false};
    bool      InSight {false};

    std::vector<std::shared_ptr<object>> Objects;
    std::shared_ptr<monster>             Monster;
};

////////////////////////////////////////////////////////////

enum class action {
    None,

    MoveLeft,
    MoveRight,
    MoveUp,
    MoveDown,

    MoveLeftUp,
    MoveRightUp,
    MoveLeftDown,
    MoveRightDown,

    LookMode,

    Execute
};

using action_queue = std::queue<action>;

using animation_func = std::function<bool()>;
constinit inline milliseconds AnimationDelay {50};

////////////////////////////////////////////////////////////

auto is_line_of_sight(point_i start, point_i end, grid<tile> const& tiles) -> bool;

auto term_to_grid(point_i pos, point_i center) -> point_i;
auto grid_to_term(point_i pos, point_i center) -> point_i;

auto get_target(point_i pos, direction dir) -> point_i;

}
