// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

////////////////////////////////////////////////////////////

class dice_painter {
public:
    explicit dice_painter(size_i texGrid);

    auto material() -> asset_owner_ptr<gfx::material>;

    void make_die(std::span<die_face const> faces);

private:
    gfx::canvas _canvas;

    size_i                         _texGrid;
    point_f                        _texPointer {2, 2};
    asset_ptr<gfx::texture>        _tex;
    asset_owner_ptr<gfx::material> _material;
};

////////////////////////////////////////////////////////////

class slot_painter {
public:
    explicit slot_painter(size_i texGrid, asset_ptr<gfx::font_family> font);

    auto material() -> asset_owner_ptr<gfx::material>;

    void make_slot(slot_face face);

private:
    gfx::canvas _canvas;

    size_i                         _texGrid;
    point_f                        _texPointer {2, 2};
    asset_ptr<gfx::texture>        _tex;
    asset_owner_ptr<gfx::material> _material;
    asset_ptr<gfx::font_family>    _font;
};