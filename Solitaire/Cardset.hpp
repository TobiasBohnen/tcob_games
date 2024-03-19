// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Cards.hpp"
#include "Common.hpp" // IWYU pragma: keep

namespace solitaire {

struct fonts {
    gfx::font* NormalFont {nullptr};
    gfx::font* LargeFont {nullptr};
};

class cardset {
public:
    cardset(std::string const& matName, assets::group& resGrp);

    void create(assets::group& resGrp);
    auto load() const -> bool;

private:
    void draw_card(gfx::canvas& canvas, fonts const& fonts, suit s, rank r, rect_f const& rect);
    void draw_marker(gfx::canvas& canvas, fonts const& fonts, rank r, rect_f const& rect);
    void draw_back(gfx::canvas& canvas, rect_f const& rect);

    void draw_suit(gfx::canvas& canvas, suit s, point_f pos, f32 size);
    void draw_shape(gfx::canvas& canvas, rect_f const& bounds, color fill, color stroke);

    void set_suit_color(gfx::canvas& canvas, suit s);

    auto get_pad(size_f const& size) -> size_f;
    auto pad_rect(rect_f const& rect) -> rect_f;

    assets::manual_asset_ptr<gfx::texture> _texture;
};

}
