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

////////////////////////////////////////////////////////////

constinit inline size_i TermMapSize {50, 40};
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

    Execute
};

using action_queue = std::queue<action>;

using animation_func = std::function<bool()>;
constinit inline milliseconds AnimationDelay {50};

////////////////////////////////////////////////////////////

auto term_to_grid(point_i pos, point_i center) -> point_i;
auto grid_to_term(point_i pos, point_i center) -> point_i;

auto get_target(point_i pos, direction dir) -> point_i;

}
