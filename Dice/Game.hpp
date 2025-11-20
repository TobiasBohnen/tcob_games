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
    base_game(gfx::window& window, assets::group const& grp);

    signal<collision_event const> Collision;
    signal<slot* const>           SlotDieChanged;

    void run(string const& file);

    auto add_shape() -> gfx::rect_shape*;
    auto remove_shape(gfx::shape* shape) -> bool;

    auto add_sprite() -> sprite*;
    void remove_sprite(sprite* sprite);

    auto add_die(std::span<die_face const> faces) -> die*;

    auto add_slot(point_f pos, slot_face face) -> slot*;
    auto get_slots() -> slots*;

    void roll();

    auto bounds() const -> rect_f const&;
    void set_background_tex(asset_ptr<gfx::texture> const& tex);

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

    gfx::window& _window;

    engine _engine;

    gfx::shape_batch           _diceBatch;
    gfx::shape_batch           _spriteBatch;
    std::unique_ptr<game_form> _form0;

    gfx::rect_shape*                     _background;
    asset_owner_ptr<gfx::material>       _backgroundMaterial;
    std::vector<std::unique_ptr<sprite>> _sprites;

    slots _slots;
    dice  _dice;

    shared_state _sharedState;
};
