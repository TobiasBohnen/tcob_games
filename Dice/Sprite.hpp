// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

struct texture {
    size_f   Size;
    string   Region;
    grid<u8> Alpha;
};

struct sprite {
    gfx::rect_shape* Shape {nullptr};
    gfx::rect_shape* WrapCopy {nullptr};

    u32      TexID {0};
    texture* Texture {nullptr};
    bool     IsCollidable {true};
    bool     IsWrappable {true};

    scripting::table Owner;
};