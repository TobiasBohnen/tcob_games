// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

// IWYU pragma: always_keep

#include <tcob/tcob.hpp>

namespace Rogue {
////////////////////////////////////////////////////////////

using namespace tcob;
using namespace std::chrono_literals;
using namespace tcob::literals;

class level;
class master_control;
class base_layout;
class monster;
class object;
class player;
class inv_item;

////////////////////////////////////////////////////////////

constinit inline size_i TermMapSize {50, 35};
constinit inline size_i TermSize {80, 45};

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
    InteractMode,

    PickUp,
    Execute
};

using action_queue = std::queue<action>;

using animation_func = std::function<bool()>;
constinit inline milliseconds AnimationDelay {150};

////////////////////////////////////////////////////////////

using log_message = std::pair<string, i32>;
using color_pair  = std::pair<color, color>;

////////////////////////////////////////////////////////////

auto term_to_grid(point_i pos, point_i center) -> point_i;
auto grid_to_term(point_i pos, point_i center) -> point_i;

}
