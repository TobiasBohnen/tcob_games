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
class dice;
class base_game;
struct shared_assets;

constexpr size_f VIRTUAL_SCREEN_SIZE {800, 600};
constexpr size_f DICE_SLOTS_REF_SIZE {1600, 900};
constexpr size_f DICE_SIZE {62, 62};
constexpr f32    DICE_OFFSET {72.f};
constexpr i32    DMD_WIDTH {100};
constexpr i32    DMD_HEIGHT {150};

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

struct texture {
    size_f   Size;
    string   Region;
    grid<u8> Alpha;
};

struct sprite {
    gfx::rect_shape* Shape {nullptr};
    gfx::rect_shape* WrapCopy {nullptr};

    u32            TexID {0};
    texture const* Texture {nullptr};
    bool           IsCollidable {true};
    bool           IsWrappable {true};

    scripting::table Owner;
};

struct collision_event {
    sprite* A;
    sprite* B;
};

struct shared_state {
    rng            Rng;
    bool           CanStart {false};
    prop<i32>      Score;
    prop<grid<u8>> DMD {grid<u8> {{DMD_WIDTH, DMD_HEIGHT}, 0}};
    prop<rect_f>   DMDBounds;

    asset_owner_ptr<gfx::material> SpriteMaterial;
    asset_owner_ptr<gfx::texture>  SpriteTexture;

    asset_owner_ptr<gfx::material> BackgroundMaterial;
    asset_owner_ptr<gfx::texture>  BackgroundTexture;
};
