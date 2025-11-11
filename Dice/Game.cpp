// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Game.hpp"

using namespace scripting;

base_game::base_game(gfx::window& window, assets::group const& grp)
    : gfx::entity {update_mode::Both}
    , _window {window}
    , _slots {_diceBatch, grp.get<gfx::font_family>("Poppins")}
    , _dice {_diceBatch, _window}
    , _engine {*this, _assets}
{
    auto const [w, h] {size_f {*window.Size}};

    _assets.Background         = &_entityBatch.create_shape<gfx::rect_shape>();
    _assets.Background->Bounds = {(w - h) / 2, 0, h, h};
}

void base_game::on_update(milliseconds deltaTime)
{
    _dice.update(deltaTime);
    _slots.update(deltaTime);

    if (_engine.update(deltaTime)) {
        wrap_sprites();
        collide_sprites();
    }

    _entityBatch.update(deltaTime);
    _diceBatch.update(deltaTime);
}

void base_game::on_draw_to(gfx::render_target& target)
{
    _entityBatch.draw_to(target);
    _diceBatch.draw_to(target);
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
                s->WrapCopy                = &_entityBatch.create_shape<gfx::rect_shape>();
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
                _entityBatch.remove_shape(*s->WrapCopy);
                s->WrapCopy = nullptr;
            }
        }
    }
}

void base_game::collide_sprites()
{
    auto const collide {[this](gfx::rect_shape* a, gfx::rect_shape* b, sprite* sA, sprite* sB) {
        rect_f const inter {a->aabb().as_intersection_with(b->aabb())};
        if (inter == rect_f::Zero) { return; }

        auto const& texA {_assets.Textures[sA->TexID]};
        auto const& texB {_assets.Textures[sB->TexID]};

        auto const& invA {a->transform().as_inverted()};
        auto const& invB {b->transform().as_inverted()};

        // cache bounds and dimensions
        f32 const aLeft {a->Bounds->left()};
        f32 const aTop {a->Bounds->top()};
        f32 const aWidth {a->Bounds->width()};
        f32 const aHeight {a->Bounds->height()};

        f32 const bLeft {b->Bounds->left()};
        f32 const bTop {b->Bounds->top()};
        f32 const bWidth {b->Bounds->width()};
        f32 const bHeight {b->Bounds->height()};

        i32 const iWidth {static_cast<i32>(inter.width())};
        i32 const iHeight {static_cast<i32>(inter.height())};

        for (i32 y {0}; y < iHeight; ++y) {
            f32 const wy {inter.top() + static_cast<f32>(y)};
            for (i32 x {0}; x < iWidth; ++x) {
                f32 const wx {inter.left() + static_cast<f32>(x)};

                // world → local
                point_f const localA {invA.transform_point({wx, wy})};
                point_f const localB {invB.transform_point({wx, wy})};

                i32 const ax {static_cast<i32>(localA.X - aLeft)};
                i32 const ay {static_cast<i32>(localA.Y - aTop)};
                i32 const bx {static_cast<i32>(localB.X - bLeft)};
                i32 const by {static_cast<i32>(localB.Y - bTop)};

                if (ax < 0 || ay < 0 || bx < 0 || by < 0
                    || ax >= static_cast<i32>(aWidth)
                    || ay >= static_cast<i32>(aHeight)
                    || bx >= static_cast<i32>(bWidth)
                    || by >= static_cast<i32>(bHeight)) {
                    continue;
                }

                if (texA.Alpha[ax, ay] > 250 && texB.Alpha[bx, by] > 250) {
                    _engine.collision(sA, sB);
                    return;
                }
            }
        }
    }};

    for (usize i {0}; i < _assets.Sprites.size(); ++i) {
        auto const& spriteA {_assets.Sprites[i]};
        if (!spriteA->IsCollisionEnabled) { continue; }

        std::vector<gfx::rect_shape*> shapesA {spriteA->Shape};
        if (spriteA->WrapCopy) { shapesA.push_back(spriteA->WrapCopy); }

        for (usize j {i + 1}; j < _assets.Sprites.size(); ++j) {
            auto const& spriteB {_assets.Sprites[j]};
            if (!spriteB->IsCollisionEnabled) { continue; }

            std::vector<gfx::rect_shape*> shapesB {spriteB->Shape};
            if (spriteB->WrapCopy) { shapesB.push_back(spriteB->WrapCopy); }

            for (auto* a : shapesA) {
                for (auto* b : shapesB) {
                    collide(a, b, spriteA.get(), spriteB.get());
                }
            }
        }
    }
}

auto base_game::create_shape() -> gfx::rect_shape*
{
    return &_entityBatch.create_shape<gfx::rect_shape>();
}

void base_game::roll()
{
    _dice.roll();
}
