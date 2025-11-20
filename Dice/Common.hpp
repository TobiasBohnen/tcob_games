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

constexpr size_f DICE_SIZE {62, 62};
constexpr f32    DICE_OFFSET {72.f};

////////////////////////////////////////////////////////////

struct sprite {
    gfx::rect_shape* Shape {nullptr};
    gfx::rect_shape* WrapCopy {nullptr};

    u32  TexID {0};
    bool IsCollisionEnabled {false};

    scripting::table Owner;
};

struct texture {
    size_i   Size;
    string   Region;
    grid<u8> Alpha;
};

struct collision_event {
    sprite* A;
    sprite* B;
};

struct shared_assets {
    std::unordered_map<u32, texture>     Textures;
    asset_owner_ptr<gfx::material>       SpriteMaterial;
    std::vector<std::unique_ptr<sprite>> Sprites;

    gfx::rect_shape*               Background;
    asset_owner_ptr<gfx::material> BackgroundMaterial;

    rect_f DiceArea {0.00f, 0.00f, 1.00f, 0.25f};

    rng Rng;
};
