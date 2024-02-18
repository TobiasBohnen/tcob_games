// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Cards.hpp"
#include "Common.hpp" // IWYU pragma: keep

namespace solitaire {

class cardset {
public:
    cardset(std::string const& matName, assets::group& resGrp);

private:
    void draw_card(suit s, rank r, rect_f const& rect);
    void draw_marker(rank r, rect_f const& rect);
    void draw_back(rect_f const& rect);

    void draw_suit(suit s, point_f pos, f32 size);
    void draw_shape(rect_f const& bounds, color fill, color stroke);

    void set_suit_color(suit s);

    size_f _cardPad;
    size_f _texSize;

    std::shared_ptr<gfx::canvas>  _canvas;
    std::shared_ptr<gfx::texture> _texture;
    gfx::font*                    _normalFont;
    gfx::font*                    _largeFont;
};

}
