// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

////////////////////////////////////////////////////////////

struct die_face {
    u8    Value {0};
    color Color {};

    auto texture_region() const -> string
    {
        return std::format("d{}-{}", Value, Color.value());
    }

    auto operator==(die_face const& other) const -> bool = default;
};

class dice_painter {
public:
    explicit dice_painter(size_i texGrid);

    auto material() -> asset_owner_ptr<gfx::material>;

    void make_die(std::span<die_face const> faces);

private:
    gfx::canvas _canvas;

    size_i                         _texGrid;
    point_f                        _pen {2, 2};
    asset_ptr<gfx::texture>        _tex;
    asset_owner_ptr<gfx::material> _material;
};

////////////////////////////////////////////////////////////

enum class op : u8 {
    Equal    = 0,
    NotEqual = 1,
    Greater  = 2,
    Less     = 3
};

struct slot_face {
    u8    Value {0};
    color Color {colors::Transparent};
    op    Op {op::Equal};

    auto texture_region() const -> string
    {
        string op;
        switch (Op) {
        case op::Equal:    op = "="; break;
        case op::NotEqual: op = "!"; break;
        case op::Greater:  op = "+"; break;
        case op::Less:     op = "-"; break;
        }

        return std::format("d{}-{}-{}", Value, Color.value(), op);
    }

    auto operator==(slot_face const& other) const -> bool = default;
};

class slot_painter {
public:
    explicit slot_painter(size_i texGrid, asset_ptr<gfx::font_family> font);

    auto material() -> asset_owner_ptr<gfx::material>;

    void make_slot(slot_face face);

private:
    gfx::canvas _canvas;

    size_i                         _texGrid;
    point_f                        _pen {2, 2};
    asset_ptr<gfx::texture>        _tex;
    asset_owner_ptr<gfx::material> _material;
    asset_ptr<gfx::font_family>    _font;
};
