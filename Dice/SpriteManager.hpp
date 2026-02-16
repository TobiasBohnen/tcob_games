// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "Sprite.hpp"

////////////////////////////////////////////////////////////

class sprite_manager {
public:
    struct init {
        event_bus& Events;
    };

    explicit sprite_manager(init const& init);

    prop<gfx::image> Foreground {gfx::image::CreateEmpty(size_i {VIRTUAL_SCREEN_SIZE}, gfx::image::format::RGBA)};
    prop<gfx::image> Background {gfx::image::CreateEmpty(size_i {VIRTUAL_SCREEN_SIZE}, gfx::image::format::RGBA)};
    prop<gfx::image> Sprites {gfx::image::CreateEmpty(SPRITE_TEXTURE_SIZE, gfx::image::format::RGBA)};

    auto add(sprite::init const& init) -> sprite*;
    void remove(sprite* sprite);
    void clear();

    void sort_by_y();

    void define_texture_region(string const& region, rect_i const& uv);

    void wrap_and_update();
    void wrap_and_collide();

    void update(milliseconds deltaTime);
    void draw_to(gfx::render_target& target);

private:
    auto add_shape() -> gfx::rect_shape*;
    auto remove_shape(gfx::shape* shape) -> bool;

    void wrap();
    void collide();

    gfx::shape_batch _spriteBatch;

    asset_owner_ptr<gfx::material> _backgroundMaterial;
    asset_owner_ptr<gfx::texture>  _backgroundTexture;
    gfx::rect_shape*               _background;
    bool                           _updateBackground {true};

    asset_owner_ptr<gfx::material> _foregroundMaterial;
    asset_owner_ptr<gfx::texture>  _foregroundTexture;
    gfx::rect_shape*               _foreground;
    bool                           _updateForeground {true};

    asset_owner_ptr<gfx::material> _spriteMaterial;
    asset_owner_ptr<gfx::texture>  _spriteTexture;
    bool                           _updateSprites {true};

    std::vector<std::unique_ptr<sprite>> _sprites;

    init _init;
};
