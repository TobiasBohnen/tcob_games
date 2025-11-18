// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "Engine.hpp"
#include "UI.hpp"
#include "objects/Die.hpp"
#include "objects/Slot.hpp"

////////////////////////////////////////////////////////////

class base_game : public gfx::entity {
public:
    base_game(gfx::window& window, assets::group const& grp);

    signal<collision_event const> Collision;

    void run(string const& file);

    auto create_shape() -> gfx::rect_shape*;
    auto remove_shape(gfx::shape* shape) -> bool;

    void add_die(std::span<die_face const> faces);
    void release_dice(std::span<i32 const> slotIdx);

    void add_slot(point_f pos, slot_face face);
    auto get_slots() -> slots*;
    auto get_dice() -> dice*;

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
    void wrap_sprites();
    void collide_sprites();

    gfx::window& _window;

    gfx::shape_batch           _diceBatch;
    gfx::shape_batch           _spriteBatch;
    std::unique_ptr<game_form> _form0;

    slots _slots;
    slot* _hoverSlot {nullptr};
    dice  _dice;
    die*  _hoverDie {nullptr};

    engine        _engine;
    shared_assets _assets;
};
