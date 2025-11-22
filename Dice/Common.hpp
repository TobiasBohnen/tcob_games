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
    bool           IsCollisionEnabled {false};

    scripting::table Owner;
};

struct collision_event {
    sprite* A;
    sprite* B;
};

struct shared_state {
    rng  Rng;
    bool CanStart {false};
};
