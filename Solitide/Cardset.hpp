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

////////////////////////////////////////////////////////////

class cardset {
public:
    virtual ~cardset() = default;
    cardset(std::string folder);

    auto get_card_size() -> size_f;

    auto get_material() const -> assets::asset_ptr<gfx::material>;

    auto load() const -> bool;

protected:
    auto get_folder() const -> std::string;
    auto get_texture() const -> gfx::texture*;
    auto is_loaded() const -> bool;

private:
    assets::manual_asset_ptr<gfx::material> _material;
    assets::manual_asset_ptr<gfx::texture>  _texture;
    std::string                             _name;
    bool                                    _loaded;
};

auto load_cardsets() -> std::map<std::string, std::shared_ptr<cardset>>;

////////////////////////////////////////////////////////////

class default_cardset : public cardset {
public:
    default_cardset(assets::group& resGrp);

    void create(assets::group& resGrp, gfx::texture* tex);

private:
    void draw_card(gfx::canvas& canvas, fonts const& fonts, suit s, rank r, rect_f const& rect);
    void draw_marker(gfx::canvas& canvas, fonts const& fonts, rank r, rect_f const& rect);
    void draw_back(gfx::canvas& canvas, rect_f const& rect);

    void draw_suit(gfx::canvas& canvas, suit s, point_f center, f32 size);
    void draw_shape(gfx::canvas& canvas, rect_f const& bounds, color fill, color stroke);

    void set_suit_color(gfx::canvas& canvas, suit s);

    auto pad_rect(rect_f const& rect) -> rect_f;
};

}
