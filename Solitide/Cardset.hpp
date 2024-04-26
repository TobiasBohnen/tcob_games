// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Cards.hpp"
#include "Common.hpp" // IWYU pragma: keep

namespace solitaire {

////////////////////////////////////////////////////////////

class cardset {
public:
    virtual ~cardset() = default;
    cardset(std::string folder);

    auto get_card_size() -> size_f;

    auto get_material() const -> assets::asset_ptr<gfx::material>;

    auto load() const -> bool;

protected:
    auto is_loaded() const -> bool;

    auto pad_rect(rect_f const& rect) -> rect_f;

    void set_suit_color(gfx::canvas& canvas, suit s);
    void draw_suit(gfx::canvas& canvas, suit s, point_f center, f32 size);

    void save_textures(assets::asset_ptr<gfx::texture> const& canvasTex, size_f texSize) const;

private:
    auto get_folder() const -> std::string;
    auto get_texture() const -> gfx::texture*;

    assets::manual_asset_ptr<gfx::material> _material;
    assets::manual_asset_ptr<gfx::texture>  _texture;
    std::string                             _name;
    bool                                    _loaded;
};

auto load_cardsets() -> std::map<std::string, std::shared_ptr<cardset>>;

////////////////////////////////////////////////////////////

class default_cardset : public cardset {
    struct fonts {
        gfx::font* NormalFont {nullptr};
        gfx::font* LargeFont {nullptr};
    };

public:
    default_cardset(assets::group& resGrp);

    void create(assets::group& resGrp);

private:
    void draw_card(gfx::canvas& canvas, fonts const& fonts, suit s, rank r, rect_f const& rect);
    void draw_marker(gfx::canvas& canvas, fonts const& fonts, rank r, rect_f const& rect);
    void draw_back(gfx::canvas& canvas, rect_f const& rect);

    void draw_shape(gfx::canvas& canvas, rect_f const& bounds, color fill, color stroke);
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class mini_cardset : public cardset {
public:
    mini_cardset(std::string folder);

    void create(assets::group& resGrp, size_f texSize);

private:
    void draw_card(gfx::canvas& canvas, gfx::font* font, suit s, rank r, rect_f const& rect);
    void draw_marker(gfx::canvas& canvas, gfx::font* font, rank r, rect_f const& rect);
    void draw_back(gfx::canvas& canvas, rect_f const& rect);

    void draw_shape(gfx::canvas& canvas, rect_f const& bounds, color fill, color stroke);
};
////////////////////////////////////////////////////////////

class mini_h_cardset : public mini_cardset {
public:
    mini_h_cardset(assets::group& resGrp);
};

////////////////////////////////////////////////////////////

class mini_v_cardset : public mini_cardset {
public:
    mini_v_cardset(assets::group& resGrp);
};

}
