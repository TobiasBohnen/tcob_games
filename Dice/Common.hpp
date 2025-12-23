// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <tcob/tcob.hpp>

using namespace tcob;
using namespace std::chrono_literals;

class slot;
class slots;
class die;
struct die_face;
class dice;
class base_game;
struct shared_assets;
class sprite;

constexpr size_f VIRTUAL_SCREEN_SIZE {320, 240};
constexpr size_f DICE_SLOTS_REF_SIZE {1600, 900};
constexpr size_i DMD_SIZE {80, 120};
constexpr size_i COVER_SIZE {128, 128};
constexpr size_f DICE_SIZE {62, 62};
constexpr f32    DICE_OFFSET {72.f};
constexpr u32    ANY_DIE_VALUE {0};
constexpr color  ANY_DIE_COLOR {colors::Transparent};

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

////////////////////////////////////////////////////////////

struct game_def {
    string                        Name;
    u32                           Number {0};
    string                        Genre;
    asset_owner_ptr<gfx::texture> Cover;

    string LuaPath;
};

enum game_status : u8 {
    Running   = 0,
    TurnEnded = 1,
    GameOver  = 2
};

struct collision_event {
    sprite* A {nullptr};
    sprite* B {nullptr};
};

struct event_bus {
    signal<>                      StartTurn;
    signal<collision_event const> Collision;
    signal<slot* const>           SlotDieChanged;
    signal<>                      DieMotion;
};

struct shared_state {
    rng    Rng;
    rect_f DMDBounds;

    prop<bool>     CanStart {false};
    prop<i32>      Score;
    prop<string>   SSDValue;
    prop<grid<u8>> DMD {grid<u8> {DMD_SIZE, 0}};
    prop<string>   Background {"default"};
};
