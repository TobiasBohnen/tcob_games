// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Field.hpp"

#include "Hand.hpp"

field::field(gfx::window& window, asset_ptr<gfx::material> const& material, asset_ptr<gfx::font> const& font)
    : gfx::entity {update_mode::Both}
    , _window {window}
    , _testText {font}
    , _slots {_batch, material}
    , _dice {_batch, _window, _slots}
{
    std::vector<die_face> faces;
    std::array const      colors {color_type::Red, color_type::Green, color_type::Yellow, color_type::Cyan, color_type::Blue};
    for (u8 i {1}; i <= 6; ++i) {
        for (auto const col : colors) {
            faces.push_back({.Value = i, .Color = col});
        }
    }
    for (i32 i {0}; i < 15; ++i) {
        _dice.add_die(_rng, faces, material);
    }

    _testText.Bounds = {{DICE_OFFSET * 6, 10}, {400, 400}};
}

void field::on_update(milliseconds deltaTime)
{
    _batch.update(deltaTime);
    _testText.update(deltaTime);
}

void field::on_fixed_update(milliseconds deltaTime)
{
    _dice.update(deltaTime);
}

void field::on_draw_to(gfx::render_target& target)
{
    _batch.draw_to(target);
    _testText.draw_to(target);
}

auto field::can_draw() const -> bool
{
    return true;
}

void field::on_key_down(input::keyboard::event const& ev)
{
    switch (ev.ScanCode) {
    case input::scan_code::C:
        _dice.roll();
        reset_shapes();
        break;
    default:
        break;
    }
}

void field::on_mouse_button_down(input::mouse::button_event const& ev)
{
    switch (ev.Button) {
    case input::mouse::button::Left: {
        _dice.hover_die(ev.Position);
    } break;
    case input::mouse::button::Right: break;
    default:                          break;
    }
}

void field::on_mouse_button_up(input::mouse::button_event const& ev)
{
    switch (ev.Button) {
    case input::mouse::button::Left: {
        auto [v, c] {_dice.drop_die()};
        reset_shapes();
        _testText.Text = std::format("{}:{}", to_string(v), to_string(c));
    } break;
    case input::mouse::button::Right: break;
    default:                          break;
    }
}

void field::on_mouse_motion(input::mouse::motion_event const& ev)
{
    reset_shapes();

    _dice.hover_slot(ev.Position);
    if (ev.Mouse->is_button_down(input::mouse::button::Left)) {
        _dice.drag(ev.Position, _window.bounds());
    } else {
        _dice.hover_die(ev.Position);
    }
}

void field::reset_shapes()
{
    _dice.reset();
    _slots.reset();
}
