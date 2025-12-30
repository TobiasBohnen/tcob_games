// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <tcob/tcob.hpp>

using namespace tcob;
using namespace std::chrono_literals;

class socket;
class sockets;
class die;
struct die_face;
class dice;
class dice_game;
struct shared_assets;
class sprite;

constexpr size_f VIRTUAL_SCREEN_SIZE {240, 180};
constexpr size_f DICE_SOCKETS_REF_SIZE {1600, 900};
constexpr size_i DMD_SIZE {80, 120};
constexpr size_i COVER_SIZE {128, 128};
constexpr size_f DICE_SIZE {62, 62};
constexpr f32    DICE_OFFSET {72.f};
constexpr i32    MAX_BACKGROUNDS {5};
constexpr size_i TEXTURE_SIZE {256, 256};

constexpr std::array<color, 16> PALETTE {
    {{0, 0, 0, 255},
     {157, 157, 157, 255},
     {255, 255, 255, 255},
     {190, 38, 51, 255},
     {224, 111, 139, 255},
     {73, 60, 43, 255},
     {164, 100, 34, 255},
     {235, 137, 49, 255},
     {247, 226, 107, 255},
     {47, 72, 78, 255},
     {68, 137, 26, 255},
     {163, 206, 39, 255},
     {27, 38, 50, 255},
     {0, 87, 132, 255},
     {49, 162, 242, 255},
     {178, 220, 239, 255}}};

static std::unordered_map<string, usize> const PALETTE_MAP {
    {"Black", 0x0},
    {"Gray", 0x1},
    {"White", 0x2},
    {"Red", 0x3},
    {"Pink", 0x4},
    {"DarkBrown", 0x5},
    {"Brown", 0x6},
    {"Orange", 0x7},
    {"Yellow", 0x8},
    {"DarkGreen", 0x9},
    {"Green", 0xA},
    {"LightGreen", 0xB},
    {"BlueGray", 0xC},
    {"DarkBlue", 0xD},
    {"Blue", 0xE},
    {"LightBlue", 0xF},
};

////////////////////////////////////////////////////////////

struct game_def {
    struct dice {
        u32              Amount {0};
        std::vector<i32> Values;
        string           Color;

        static auto constexpr Members()
        {
            return std::tuple {member<&dice::Amount> {"Amount"},
                               member<&dice::Values> {"Values"},
                               member<&dice::Color> {"Color"}};
        }
    };
    struct info {
        string Genre;
        u32    Year;

        static auto constexpr Members()
        {
            return std::tuple {member<&info::Genre> {"Genre"},
                               member<&info::Year> {"Year"}};
        }
    };

    string            Name;
    u32               Number {0};
    std::vector<dice> Dice;
    string            Cover;
    info              Info;

    static auto constexpr Members()
    {
        return std::tuple {member<&game_def::Name> {"Name"},
                           member<&game_def::Number> {"Number"},
                           member<&game_def::Dice> {"Dice"},
                           member<&game_def::Cover> {"Cover"},
                           member<&game_def::Info> {"Info"}};
    }

    string                        LuaPath;
    asset_owner_ptr<gfx::texture> CoverTex;
};

struct collision_event {
    sprite* A {nullptr};
    sprite* B {nullptr};
};

struct event_bus {
    signal<> StartTurn;
    signal<> Quit;
    signal<> Restart;

    signal<collision_event const> Collision;
    signal<socket* const>         SocketDieChanged;
    signal<>                      DieMotion;
};

struct shared_state {
    rng    Rng;
    rect_f DMDBounds;

    prop<bool>       CanStart {false};
    prop<i32>        Score;
    prop<string>     SSDValue;
    prop<grid<u8>>   DMD {grid<u8> {DMD_SIZE, 0}};
    prop<gfx::image> Foreground;
    prop<string>     Background {"default"};
};
