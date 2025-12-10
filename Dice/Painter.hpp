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

auto get_pixel(string_view s, size_i size) -> std::vector<u8>;
