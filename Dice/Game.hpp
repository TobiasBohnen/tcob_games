// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "Die.hpp"
#include "Engine.hpp"
#include "Slot.hpp"
#include "UI.hpp"

////////////////////////////////////////////////////////////

class base_game : public gfx::entity {
public:
    base_game(assets::group const& grp, size_f realWindowSize);

    signal<collision_event const> Collision;
    signal<slot* const>           SlotDieChanged;

    void run(string const& file);

    auto add_shape() -> gfx::rect_shape*;
    auto remove_shape(gfx::shape* shape) -> bool;

    auto add_sprite() -> sprite*;
    void remove_sprite(sprite* sprite);

    auto add_die(std::span<die_face const> faces) -> die*;

    auto add_slot(slot_face face) -> slot*;
    auto get_slots() -> slots*;

    void roll();

    auto ui_bounds() const -> rect_f const&;

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
    auto get_random_die_position() -> point_f;

    void wrap_sprites();
    void collide_sprites();

    engine _engine;

    asset_owner_ptr<gfx::render_texture> _screenTexture;
    asset_owner_ptr<gfx::material>       _screenMaterial {};
    gfx::quad_renderer                   _screenRenderer {gfx::buffer_usage_hint::StaticDraw};

    gfx::shape_batch           _diceBatch;
    gfx::shape_batch           _spriteBatch;
    std::unique_ptr<game_form> _form0;

    gfx::rect_shape*                     _background;
    std::vector<std::unique_ptr<sprite>> _sprites;

    slots _slots;
    dice  _dice;

    shared_state _sharedState;
};
