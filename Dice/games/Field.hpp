// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp" // IWYU pragma: keep

#include "../objects/Die.hpp"
#include "../objects/Slot.hpp"

////////////////////////////////////////////////////////////

class field : public gfx::entity {
public:
    field(gfx::window& window, assets::group const& grp);

protected:
    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_draw_to(gfx::render_target& target) override;
    auto can_draw() const -> bool override;

    void on_key_down(input::keyboard::event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;
    void on_mouse_button_up(input::mouse::button_event const& ev) override;
    void on_mouse_motion(input::mouse::motion_event const& ev) override;

private:
    rng          _rng;
    gfx::window& _window;

    gfx::shape_batch _batch;
    gfx::text        _testText;

    slots _slots;
    slot* _hoverSlot {nullptr};
    dice  _dice;
    die*  _hoverDie {nullptr};

    gfx::background _background;
};
