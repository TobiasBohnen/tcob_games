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
    , _dice {_batch, material, _window}
{
    {
        std::vector<die_face> faces;
        std::array const      colors {color_type::Red, color_type::Green, color_type::Yellow, color_type::Cyan, color_type::Blue};
        for (u8 i {1}; i <= 6; ++i) {
            for (auto const col : colors) {
                faces.push_back({.Value = i, .Color = col});
            }
        }

        for (i32 i {0}; i < 15; ++i) {
            _dice.add_die({i * DICE_OFFSET, DICE_OFFSET * 2}, _rng, {.Value = static_cast<u8>(_rng(1, 6)), .Color = color_type::Green}, faces);
        }
    }

    {
        for (i32 i {0}; i < 5; ++i) {
            _slots.add_slot({i * DICE_OFFSET, 10},
                            std::array<u8, 6> {1, 2, 3, 4, 5, 6},
                            std::array {color_type::Green, color_type::Yellow, color_type::Cyan, color_type::Blue});
        }
    }

    _testText.Bounds = {{DICE_OFFSET * 6, 10}, {1000, 400}};
}

void field::on_update(milliseconds deltaTime)
{
    auto [v, c, i] {_slots.get_hand()};
    auto sum {_slots.get_sum()};
    for (auto id : i) {
        _slots.get_slot(id)->State = slot_state::PartOfHand;
    }
    _testText.Text = std::format("{}:{}-{}-<{}>", to_string(v), to_string(c), helper::join(i, ","), sum);

    _dice.update(deltaTime);
    _slots.update(deltaTime);

    _batch.update(deltaTime);
    _testText.update(deltaTime);
}

void field::on_fixed_update(milliseconds deltaTime)
{
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
        break;
    default:
        break;
    }
}

void field::on_mouse_button_down(input::mouse::button_event const& ev)
{
    switch (ev.Button) {
    case input::mouse::button::Left:  break;
    case input::mouse::button::Right: break;
    default:                          break;
    }
}

void field::on_mouse_button_up(input::mouse::button_event const& ev)
{
    switch (ev.Button) {
    case input::mouse::button::Left: {
        _dice.drop(_hoverSlot);
    } break;
    case input::mouse::button::Right: break;
    default:                          break;
    }
}

void field::on_mouse_motion(input::mouse::motion_event const& ev)
{
    bool const isButtonDown {ev.Mouse->is_button_down(input::mouse::button::Left)};

    if (!isButtonDown) {
        _hoverDie = _dice.hover_die(ev.Position);
    }

    auto getRect {[&] -> rect_f {
        if (_hoverDie) {
            rect_i const  bounds {_hoverDie->bounds()};
            point_f const tl {_window.camera().convert_screen_to_world(bounds.top_left())};
            point_f const br {_window.camera().convert_screen_to_world(bounds.bottom_right())};
            return rect_f::FromLTRB(tl.X, tl.Y, br.X, br.Y);
        }

        point_f const mp {_window.camera().convert_screen_to_world(ev.Position)};
        return {mp, size_f::One};
    }};

    _hoverSlot = _slots.hover_slot(getRect());
    if (_hoverSlot) {
        if (!_hoverDie) {
            _hoverSlot->State = slot_state::Hovered;
        } else if (isButtonDown) {
            _hoverSlot->State = _hoverSlot->can_drop(_hoverDie->current_face())
                ? slot_state::Accept
                : slot_state::Reject;
        }
    }
    if (isButtonDown) {
        _slots.take_die(_hoverDie);
        _dice.drag(ev.Position);
    }
}
