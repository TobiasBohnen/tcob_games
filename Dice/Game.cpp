// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Game.hpp"

using namespace scripting;

constexpr size_f VIRTUAL_SCREEN_SIZE {800, 600};
constexpr size_f DICE_SLOTS_REF_SIZE {1600, 900};

base_game::base_game(assets::group const& grp, size_i realWindowSize)
    : gfx::entity {update_mode::Both}
    , _realWindowSize {size_f {realWindowSize}}
    , _background(&_spriteBatch.create_shape<gfx::rect_shape>())
    , _slots {_diceBatch, grp.get<gfx::font_family>("Poppins"), _realWindowSize / DICE_SLOTS_REF_SIZE}
    , _dice {_diceBatch, _realWindowSize / DICE_SLOTS_REF_SIZE}
    , _engine {*this, _sharedState}
{
    _background->Bounds = {point_f::Zero, VIRTUAL_SCREEN_SIZE};

    auto const [w, h] {_realWindowSize};
    rect_f const bgBounds {0, 0, h / 3.0f * 4.0f, h};
    rect_f const uiBounds {bgBounds.width(), 0.0f, w - bgBounds.width(), h};

    _form0 = std::make_unique<game_form>(uiBounds, grp, _sharedState);
    _form0->StartTurn.connect([&]() { _engine.start_turn(); });

    _texture->Size                  = size_i {VIRTUAL_SCREEN_SIZE};
    _texture->Filtering             = gfx::texture::filtering::Linear;
    _material->first_pass().Texture = _texture;

    gfx::quad q {};
    gfx::geometry::set_color(q, colors::White);
    gfx::geometry::set_position(q, bgBounds);
    gfx::geometry::set_texcoords(q, {.UVRect = gfx::render_texture::UVRect(), .Level = 0});
    _renderer.set_geometry(q, &_material->first_pass());
}

void base_game::on_update(milliseconds deltaTime)
{
    _dice.update(deltaTime);
    _slots.update(deltaTime);

    _spriteBatch.update(deltaTime);
    _form0->update(deltaTime);
    _diceBatch.update(deltaTime);
}

void base_game::on_fixed_update(milliseconds deltaTime)
{
    if (_engine.update(deltaTime)) {
        wrap_sprites();
        collide_sprites();
    }
}

void base_game::on_draw_to(gfx::render_target& target)
{
    _spriteBatch.draw_to(*_texture);
    _renderer.render_to_target(target);

    _form0->draw_to(target);
    _diceBatch.draw_to(target);
}

auto base_game::can_draw() const -> bool
{
    return true;
}

void base_game::on_key_down(input::keyboard::event const& ev)
{
    switch (ev.ScanCode) {
    case input::scan_code::SPACE: _engine.start_turn(); break;
    default:                      break;
    }
}

void base_game::on_mouse_button_up(input::mouse::button_event const& ev)
{
    switch (ev.Button) {
    case input::mouse::button::Left:
        if (_slots.try_insert(_dice.HoverDie)) {
            SlotDieChanged(_slots.HoverSlot);
        }
        break;
    case input::mouse::button::Right: break;
    default:                          break;
    }

    dynamic_cast<input::receiver*>(_form0.get())->on_mouse_button_up(ev);
}

void base_game::on_mouse_button_down(input::mouse::button_event const& ev)
{
    dynamic_cast<input::receiver*>(_form0.get())->on_mouse_button_down(ev);
}

void base_game::on_mouse_motion(input::mouse::motion_event const& ev)
{
    auto const mp {point_f {ev.Position}};

    bool const isButtonDown {ev.Mouse->is_button_down(input::mouse::button::Left)};

    if (!isButtonDown) {
        _dice.hover(mp);
    }

    auto const getRect {[&] -> rect_f {
        if (_dice.HoverDie) {
            rect_i const  bounds {_dice.HoverDie->bounds()};
            point_f const tl {bounds.top_left()};
            point_f const br {bounds.bottom_right()};
            return rect_f::FromLTRB(tl.X, tl.Y, br.X, br.Y);
        }

        return {mp, size_f::One};
    }};

    _slots.hover(getRect(), _dice.HoverDie, isButtonDown);
    if (isButtonDown) {
        if (auto* slot {_slots.try_remove(_dice.HoverDie)}) {
            SlotDieChanged(slot);
        }
        _dice.drag(mp, ui_bounds());
    }

    dynamic_cast<input::receiver*>(_form0.get())->on_mouse_motion(ev);
}

void base_game::run(string const& file)
{
    _engine.run(file);
    wrap_sprites();
}

void base_game::wrap_sprites()
{
    auto const fieldSize {_background->Bounds->Size};

    for (auto& s : _sprites) {
        if (!s || !s->Shape || !s->IsWrappable) { continue; }

        auto b {*s->Shape->Bounds};
        b.Position.X -= _background->Bounds->left();
        bool const needs_copy_x {(b.left() < 0.f) || (b.right() > fieldSize.Width)};
        bool const needs_copy_y {(b.top() < 0.f) || (b.bottom() > fieldSize.Height)};

        if (needs_copy_x || needs_copy_y) {
            if (!s->WrapCopy) {
                s->WrapCopy                = &_spriteBatch.create_shape<gfx::rect_shape>();
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

            s->WrapCopy->Bounds   = {{copyX + _background->Bounds->left(), copyY}, s->Shape->Bounds->Size};
            s->WrapCopy->Rotation = *s->Shape->Rotation;
        } else {
            if (s->WrapCopy) {
                _spriteBatch.remove_shape(*s->WrapCopy);
                s->WrapCopy = nullptr;
            }
        }
    }
}

void base_game::collide_sprites()
{
    std::vector<collision_event> events;

    auto const collide {[this, &events](gfx::rect_shape* a, gfx::rect_shape* b, sprite* sA, sprite* sB) {
        rect_f const inter {a->aabb().as_intersection_with(b->aabb())};
        if (inter == rect_f::Zero) { return; }

        auto const* texA {sA->Texture};
        auto const* texB {sB->Texture};

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

                // world -> local
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

                if (texA->Alpha[ax, ay] > 250 && texB->Alpha[bx, by] > 250) {
                    events.push_back({.A = sA, .B = sB});
                    return;
                }
            }
        }
    }};

    for (usize i {0}; i < _sprites.size(); ++i) {
        auto const& spriteA {_sprites[i]};
        if (!spriteA->IsCollidable) { continue; }

        std::array<gfx::rect_shape*, 2> shapesA {spriteA->Shape, spriteA->WrapCopy};
        i32                             countA {spriteA->WrapCopy ? 2 : 1};

        for (usize j {i + 1}; j < _sprites.size(); ++j) {
            auto const& spriteB {_sprites[j]};
            if (!spriteB->IsCollidable) { continue; }

            std::array<gfx::rect_shape*, 2> shapesB {spriteB->Shape, spriteB->WrapCopy};
            i32                             countB {spriteB->WrapCopy ? 2 : 1};

            for (i32 a {0}; a < countA; ++a) {
                for (i32 b {0}; b < countB; ++b) {
                    collide(shapesA[a], shapesB[b], spriteA.get(), spriteB.get());
                }
            }
        }
    }

    for (auto const& event : events) {
        Collision(event);
    }
}

auto base_game::add_shape() -> gfx::rect_shape*
{
    return &_spriteBatch.create_shape<gfx::rect_shape>();
}

auto base_game::remove_shape(gfx::shape* shape) -> bool
{
    return _spriteBatch.remove_shape(*shape);
}

auto base_game::add_sprite() -> sprite*
{
    auto  ptr {std::make_unique<sprite>()};
    auto* retValue {ptr.get()};
    _sprites.push_back(std::move(ptr));
    return retValue;
}

void base_game::remove_sprite(sprite* sprite)
{
    std::erase_if(_sprites, [&sprite](auto const& spr) { return spr.get() == sprite; });
}

auto base_game::add_slot(slot_face face) -> slot*
{
    rect_f const bounds {ui_bounds()};

    usize const index {_slots.count()};
    usize const col {index % 5};
    usize const row {index / 5};

    f32 const xStart {bounds.left() + (bounds.width() / 20.0f)};
    f32 const xStep {bounds.width() / 6.0f};

    f32 const y {bounds.top() + (bounds.height() / 2)};

    point_f const pos {xStart + (col * xStep), y + (row * (bounds.height() * 0.12f))};
    return _slots.add_slot(face, pos);
}

auto base_game::get_slots() -> slots*
{
    return &_slots;
}

auto base_game::get_random_die_position() -> point_f
{
    rect_f area {_form0->Bounds};
    area.Size -= DICE_SIZE;

    point_f pos;
    pos.X = _sharedState.Rng(area.left(), area.right());
    pos.Y = _sharedState.Rng(area.top(), area.bottom());

    return pos;
}

auto base_game::add_die(std::span<die_face const> faces) -> die*
{
    return _dice.add_die(get_random_die_position(), _sharedState.Rng, faces[0], faces);
}

void base_game::roll()
{
    _dice.roll();
}
void base_game::set_background_tex(asset_ptr<gfx::texture> const& tex)
{
    _backgroundMaterial->first_pass().Texture = tex;
    tex->regions()["default"]                 = {.UVRect = {gfx::render_texture::UVRect()},
                                                 .Level  = 0};
    _background->Material                     = _backgroundMaterial;
}

auto base_game::world_size() const -> size_f
{
    return _background->Bounds->Size;
}
auto base_game::ui_bounds() const -> rect_f const&
{
    return *_form0->Bounds;
}
