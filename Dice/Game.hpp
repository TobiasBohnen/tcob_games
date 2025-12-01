// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "Die.hpp"
#include "Engine.hpp"
#include "Slot.hpp"
#include "Sprite.hpp"
#include "UI.hpp"

////////////////////////////////////////////////////////////

class base_game : public gfx::entity {
public:
    base_game(assets::group const& grp, size_f realWindowSize);

    void run(string const& file);

    auto add_sprite() -> sprite*;
    void remove_sprite(sprite* sprite);

    auto get_random_die_position() -> point_f;

protected:
    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_draw_to(gfx::render_target& target) override;
    auto can_draw() const -> bool override;

    void on_key_down(input::keyboard::event const& ev) override;
    void on_mouse_button_up(input::mouse::button_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;
    void on_mouse_motion(input::mouse::motion_event const& ev) override;

private:
    auto add_shape() -> gfx::rect_shape*;
    auto remove_shape(gfx::shape* shape) -> bool;

    void wrap_sprites();
    void collide_sprites();

    gfx::shape_batch           _diceBatch;
    gfx::shape_batch           _spriteBatch;
    std::unique_ptr<game_form> _form0;

    asset_owner_ptr<gfx::material> _spriteMaterial;
    asset_owner_ptr<gfx::texture>  _spriteTexture;

    asset_owner_ptr<gfx::material> _backgroundMaterial;
    asset_owner_ptr<gfx::texture>  _backgroundTexture;
    gfx::rect_shape*               _background;

    asset_owner_ptr<gfx::render_texture> _screenTexture;
    asset_owner_ptr<gfx::material>       _screenMaterial {};
    gfx::quad_renderer                   _screenRenderer {gfx::buffer_usage_hint::StaticDraw};

    shared_state _sharedState;
    event_bus    _events;
    engine       _engine;

    slots                                _slots;
    dice                                 _dice;
    std::vector<std::unique_ptr<sprite>> _sprites;
};
