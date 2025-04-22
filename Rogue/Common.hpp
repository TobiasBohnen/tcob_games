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

class master_control;

class tile;
class dungeon;
class base_layout;

class object;
class item;
class inv_item;

class monster;
class player;
struct profile;

////////////////////////////////////////////////////////////

constinit inline size_i       TERM_MAP_SIZE {50, 35};
constinit inline size_i       TERM_SIZE {80, 45};
constinit inline milliseconds ANIMATION_DELAY {150};

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
    MFDModeChange,

    PickUp,
    Execute
};

////////////////////////////////////////////////////////////

enum class mfd_mode {
    Character,
    Inventory,
    Magic,
    Monsters
};
constexpr i32 MFD_COUNT {4};

////////////////////////////////////////////////////////////

enum class mode {
    Move,
    Look,
    Interact,
};

////////////////////////////////////////////////////////////

using log_message  = std::pair<string, i32>;
using color_pair   = std::pair<color, color>;
using action_queue = std::queue<action>;
using inventory    = std::vector<std::pair<std::shared_ptr<item>, i32>>;

////////////////////////////////////////////////////////////

constexpr color COLOR_EARTH {colors::Brown};
constexpr color COLOR_WIND {colors::LightBlue};
constexpr color COLOR_FIRE {colors::FireBrick};
constexpr color COLOR_WATER {colors::Blue};
constexpr color COLOR_LIFE {colors::Green};
constexpr color COLOR_ENERGY {colors::WhiteSmoke};

constexpr i32 XP_SCALE {50};
constexpr i32 VIT_SCALE {10};
constexpr i32 INT_SCALE {15};

}
