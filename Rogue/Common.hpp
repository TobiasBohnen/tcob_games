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

class tile;
class level;
class master_control;
class base_layout;
class monster;
class object;
class player;
struct profile;
class inv_item;

////////////////////////////////////////////////////////////

constinit inline size_i       TermMapSize {50, 35};
constinit inline size_i       TermSize {80, 45};
constinit inline milliseconds AnimationDelay {150};

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
    Magic
};

////////////////////////////////////////////////////////////

enum class mode {
    Move,
    Look,
    Interact,
};

////////////////////////////////////////////////////////////

using log_message    = std::pair<string, i32>;
using color_pair     = std::pair<color, color>;
using action_queue   = std::queue<action>;
using animation_func = std::function<bool()>;

////////////////////////////////////////////////////////////

auto term_to_grid(point_i pos, point_i center) -> point_i;
auto grid_to_term(point_i pos, point_i center) -> point_i;

////////////////////////////////////////////////////////////

constexpr color COLOR_EARTH {colors::Brown};
constexpr color COLOR_WIND {colors::LightBlue};
constexpr color COLOR_FIRE {colors::FireBrick};
constexpr color COLOR_WATER {colors::Blue};
constexpr color COLOR_LIFE {colors::Green};
constexpr color COLOR_LIGHT {colors::WhiteSmoke};

constexpr i32 XP_SCALE {50};
constexpr i32 VIT_SCALE {10};
constexpr i32 INT_SCALE {15};

struct magic {
    i32 Earth {1};
    i32 Wind {2};
    i32 Fire {3};
    i32 Water {4};
    i32 Life {5};
    i32 Light {6};
};

struct attributes {
    i32 Strength {10};
    i32 Intelligence {20};
    i32 Vitality {5};
    i32 Agility {3};
    i32 Dexterity {9};
};

struct profile {
    string Name {"Tim"};

    i32 HP {75};
    i32 MP {25};
    i32 XP {85};
    i32 Gold {0};

    f32 VisualRange {7};

    magic Magic;

    attributes Attributes;

    auto static constexpr xp_required_for(i32 level) -> i32
    {
        return XP_SCALE * (level - 1) * level;
    }
};

}
