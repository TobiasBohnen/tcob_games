// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Painter.hpp"

#include <utility>

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

////////////////////////////////////////////////////////////

slot_painter::slot_painter(size_i texGrid, asset_ptr<gfx::font_family> font)
    : _texGrid {texGrid}
    , _font {std::move(font)}
{
    _tex = _canvas.get_texture(0);

    _material->first_pass().Texture = _tex;
}

auto slot_painter::material() -> asset_owner_ptr<gfx::material>
{
    return _material;
}

void slot_painter::make_slot(slot_face face)
{
    if (_tex->regions().contains(face.texture_region())) { return; }

    static auto const slotPath {*gfx::path2d::Parse("M 15 0 C 7 0 0 7 0 15 L 0 49 C 0 57 7 64 15 64 L 49 64 C 57 64 64 57 64 49 L 64 15 C 64 7 57 0 49 0 L 15 0 z M 22 7 L 42 7 C 50 7 57 14 57 22 L 57 42 C 57 50 50 57 42 57 L 22 57 C 14 57 7 50 7 42 L 7 22 C 7 14 14 7 22 7 z ")};

    auto const texSize {_texGrid * 68};
    _canvas.begin_frame(texSize, 1, 0, false);
    {
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

        _canvas.path_2d(slotPath);
        _canvas.set_fill_style(face.Color == colors::Transparent ? colors::White : face.Color);
        _canvas.fill(false);
        _canvas.set_stroke_width(2);
        _canvas.set_stroke_style(colors::Black);
        _canvas.stroke();

        if (face.Value != 0) {
            string op;
            switch (face.Op) {
            case op::Equal:    op = ""; break;
            case op::NotEqual: op = "!"; break;
            case op::Greater:  op = "+"; break;
            case op::Less:     op = "-"; break;
            }

            _canvas.set_font(_font->get_font({.IsItalic = false, .Weight = gfx::font::weight::Bold}, 32).ptr());
            _canvas.set_text_halign(gfx::horizontal_alignment::Centered);
            _canvas.set_text_valign(gfx::vertical_alignment::Middle);
            _canvas.draw_text({0, 0, 68, 68}, std::format("{} {}", face.Value, op));
        }
        _canvas.restore();
    }

    _canvas.end_frame();
    _tex->Filtering = gfx::texture::filtering::Linear;
}