// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Painter.hpp"

dice_painter::dice_painter(size_i texGrid)
    : _texGrid {texGrid}
{
    _tex = _canvas.get_texture(0);

    _material->first_pass().Texture = _tex;
}

auto dice_painter::material() -> asset_owner_ptr<gfx::material>
{
    return _material;
}

void dice_painter::make_die(std::span<die_face const> faces)
{
    static auto const dicePath {*gfx::path2d::Parse("m 15,0 h 34 c 8,0 15,7 15,15 v 34 c 0,8 -7,15 -15,15 H 15 C 7,64 0,57 0,49 V 15 C 0,7 7,0 15,0 Z")};
    auto const        paintDie {[&](color fg, color bg) {
        _canvas.path_2d(dicePath);
        _canvas.set_fill_style(fg);
        _canvas.fill(false);
        _canvas.set_stroke_width(2);
        _canvas.set_stroke_style(bg);
        _canvas.stroke();
    }};
    auto const        paintDots {[&](color fg, auto... dots) {
        _canvas.set_fill_style(fg);
        _canvas.begin_path();
        (_canvas.circle(dots, 6), ...);
        _canvas.fill();
    }};

    auto const texSize {_texGrid * 68};
    _canvas.begin_frame(texSize, 1, 0, false);
    for (auto const& face : faces) {
        if (_tex->regions().contains(face.texture_region())) { continue; }

        _canvas.save();

        _canvas.translate(_pen);

        _tex->regions()[face.texture_region()] = {
            .UVRect = {{(_pen.X - 2) / texSize.Width,
                        -(_pen.Y - 2) / texSize.Height},
                       {68.f / texSize.Width,
                        -68.f / texSize.Height}},
            .Level  = 0};

        _pen += point_f {68, 0};
        if (_pen.X >= texSize.Width) {
            _pen.X = 2;
            _pen.Y += 68;
        }

        paintDie(face.Color, colors::Black);

        switch (face.Value) {
        case 1: paintDots(colors::Black, point_f {32, 32}); break;
        case 2: paintDots(colors::Black, point_f {16, 16}, point_f {48, 48}); break;
        case 3: paintDots(colors::Black, point_f {16, 16}, point_f {32, 32}, point_f {48, 48}); break;
        case 4: paintDots(colors::Black, point_f {16, 16}, point_f {48, 16}, point_f {16, 48}, point_f {48, 48}); break;
        case 5: paintDots(colors::Black, point_f {16, 16}, point_f {48, 16}, point_f {32, 32}, point_f {16, 48}, point_f {48, 48}); break;
        case 6: paintDots(colors::Black, point_f {16, 16}, point_f {48, 16}, point_f {16, 32}, point_f {48, 32}, point_f {16, 48}, point_f {48, 48}); break;
        }

        _canvas.restore();
    }
    _canvas.end_frame();
    _tex->Filtering = gfx::texture::filtering::Linear;
}
