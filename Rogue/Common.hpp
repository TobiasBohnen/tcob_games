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
class renderer;

class tile;
class dungeon;
class base_layout;

class object;
class item;
class inv_item;

class actor;
class monster;
class player;
struct profile;

////////////////////////////////////////////////////////////

constinit inline size_i       TERM_MAP_SIZE {50, 35};
constinit inline size_i       TERM_SIZE {80, 45};
constinit inline milliseconds ANIMATION_DELAY {150};

////////////////////////////////////////////////////////////

enum class action : u8 {
    None,

    Left,
    Right,
    Up,
    Down,

    LeftUp,
    RightUp,
    LeftDown,
    RightDown,

    Center,

    LookMode,
    UseMode,

    MFDModeChange,
    MFDModeCharacter,
    MFDModeInventory,
    MFDModeMonsters,

    Get,
    Search,
    Execute
};

////////////////////////////////////////////////////////////

enum class mfd_mode : u8 {
    Character,
    Inventory,
    Monsters
};
constexpr i32 MFD_COUNT {3};

////////////////////////////////////////////////////////////

enum class mode : u8 {
    Move,
    Look,
    Use,
};

////////////////////////////////////////////////////////////

struct log_message {
    string Message;
};

////////////////////////////////////////////////////////////

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

constexpr color_pair SEEN_COLORS {color::Lerp(colors::Black, colors::White, 0.15f), colors::Black};

auto inline term_to_grid(point_i pos, point_i center) -> point_i
{
    return pos - point_i {TERM_MAP_SIZE.Width / 2, TERM_MAP_SIZE.Height / 2} + center;
}

auto inline grid_to_term(point_i pos, point_i center) -> point_i
{
    return pos + point_i {TERM_MAP_SIZE.Width / 2, TERM_MAP_SIZE.Height / 2} - center;
}

}
