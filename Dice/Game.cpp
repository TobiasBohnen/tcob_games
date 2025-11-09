// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Game.hpp"

using namespace scripting;

base_game::base_game(gfx::window& window, assets::group const& grp)
    : gfx::entity {update_mode::Both}
    , _rng {12345}
    , _window {window}
    , _slots {_batch, grp.get<gfx::font_family>("Poppins")}
    , _dice {_batch, _window}
    , _engine {*this, _assets}
{
    auto const [w, h] {size_f {*window.Size}};

    _assets.Background         = &_batch.create_shape<gfx::rect_shape>();
    _assets.Background->Bounds = {(w - h) / 2, 0, h, h};
}

void base_game::on_update(milliseconds deltaTime)
{
    _dice.update(deltaTime);
    _slots.update(deltaTime);

    _engine.update(deltaTime);

    wrap_sprites();
    _batch.update(deltaTime);
}

void base_game::on_draw_to(gfx::render_target& target)
{
    _batch.draw_to(target);
}

auto base_game::can_draw() const -> bool
{
    return true;
}

void base_game::on_key_down(input::keyboard::event const& ev)
{
    switch (ev.ScanCode) {
    case input::scan_code::C: _dice.roll(); break;
    default:                  break;
    }
}

void base_game::on_mouse_button_up(input::mouse::button_event const& ev)
{
    switch (ev.Button) {
    case input::mouse::button::Left:  _dice.drop(_hoverSlot); break;
    case input::mouse::button::Right: break;
    default:                          break;
    }
}

void base_game::on_mouse_motion(input::mouse::motion_event const& ev)
{
    bool const isButtonDown {ev.Mouse->is_button_down(input::mouse::button::Left)};

    if (!isButtonDown) {
        _hoverDie = _dice.hover_die(ev.Position);
    }

    auto const getRect {[&] -> rect_f {
        if (_hoverDie) {
            rect_i const  bounds {_hoverDie->bounds()};
            point_f const tl {_window.camera().convert_screen_to_world(bounds.top_left())};
            point_f const br {_window.camera().convert_screen_to_world(bounds.bottom_right())};
            return rect_f::FromLTRB(tl.X, tl.Y, br.X, br.Y);
        }

        point_f const mp {_window.camera().convert_screen_to_world(ev.Position)};
        return {mp, size_f::One};
    }};

    _hoverSlot = _slots.hover_slot(getRect(), _hoverDie, isButtonDown);
    if (isButtonDown) {
        _slots.take_die(_hoverDie);
        _dice.drag(ev.Position);
    }
}

void base_game::run(string const& file)
{
    _engine.run(file);
}

void base_game::wrap_sprites()
{
    auto const fieldSize {_assets.Background->Bounds->Size};

    for (auto& s : _assets.Sprites) {
        if (!s || !s->Shape) { continue; }

        auto b {*s->Shape->Bounds};
        b.Position.X -= _assets.Background->Bounds->left();
        bool const needs_copy_x {(b.left() < 0.f) || (b.right() > fieldSize.Width)};
        bool const needs_copy_y {(b.top() < 0.f) || (b.bottom() > fieldSize.Height)};

        if (needs_copy_x || needs_copy_y) {
            if (!s->WrapCopy) {
                s->WrapCopy                = &_batch.create_shape<gfx::rect_shape>();
                s->WrapCopy->Material      = *s->Shape->Material;
                s->WrapCopy->TextureRegion = *s->Shape->TextureRegion;
            }

            f32 copyX {b.left()};
            f32 copyY {b.top()};

            if (b.left() < 0.f) {
                copyX = b.left() + fieldSize.Width;
            } else if (b.right() > fieldSize.Width) {
                copyX = b.left() - fieldSize.Width;
            }
            if (b.top() < 0.f) {
                copyY = b.top() + fieldSize.Height;
            } else if (b.bottom() > fieldSize.Height) {
                copyY = b.top() - fieldSize.Height;
            }

            s->WrapCopy->Bounds   = {{copyX + _assets.Background->Bounds->left(), copyY}, s->Shape->Bounds->Size};
            s->WrapCopy->Rotation = *s->Shape->Rotation;
        } else {
            if (s->WrapCopy) {
                _batch.remove_shape(*s->WrapCopy);
                s->WrapCopy = nullptr;
            }
        }
    }
}

auto base_game::create_shape() -> gfx::rect_shape* { return &_batch.create_shape<gfx::rect_shape>(); }
auto base_game::random(f32 min, f32 max) -> f32
{
    return _rng(min, max);
}
void base_game::roll()
{
    _dice.roll();
}
