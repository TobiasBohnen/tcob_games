// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "SpriteManager.hpp"

sprite_manager::sprite_manager(event_bus& events)
    : _background {&_spriteBatch.create_shape<gfx::rect_shape>()}
    , _foreground {&_spriteBatch.create_shape<gfx::rect_shape>()}
    , _events {events}
{
    _spriteMaterial->first_pass().Texture = _spriteTexture;
    _spriteTexture->resize(SPRITE_TEXTURE_SIZE, 1, gfx::texture::format::RGBA8);
    Sprites.Changed.connect([&](auto const&) { _updateSprites = true; });

    _background->Bounds                       = {point_f::Zero, VIRTUAL_SCREEN_SIZE};
    _backgroundMaterial->first_pass().Texture = _backgroundTexture;
    _background->Material                     = _backgroundMaterial;
    _backgroundTexture->resize(size_i {VIRTUAL_SCREEN_SIZE}, 1, gfx::texture::format::RGBA8);
    _backgroundTexture->regions()["default"] = gfx::texture_region {.UVRect = {0, 0, 1, 1}, .Level = 0};
    Background.Changed.connect([&](auto const&) { _updateBackground = true; });

    _foreground->Bounds                       = {point_f::Zero, VIRTUAL_SCREEN_SIZE};
    _foregroundMaterial->first_pass().Texture = _foregroundTexture;
    _foreground->Material                     = _foregroundMaterial;
    _foregroundTexture->resize(size_i {VIRTUAL_SCREEN_SIZE}, 1, gfx::texture::format::RGBA8);
    _foregroundTexture->regions()["default"] = gfx::texture_region {.UVRect = {0, 0, 1, 1}, .Level = 0};
    Foreground.Changed.connect([&](auto const&) { _updateForeground = true; });
}

auto sprite_manager::add(sprite::init const& init) -> sprite*
{
    auto  ptr {std::make_unique<sprite>(init)};
    auto* retValue {ptr.get()};
    _sprites.push_back(std::move(ptr));
    retValue->Shape           = add_shape();
    retValue->Shape->Material = _spriteMaterial;
    if (init.Texture) {
        retValue->set_texture(init.Texture);
        retValue->set_bounds(init.Def.Position, init.Texture->Size);
    }
    return retValue;
}

void sprite_manager::remove(sprite* sprite)
{
    remove_shape(sprite->Shape);
    if (sprite->WrapCopy) { remove_shape(sprite->WrapCopy); }
    std::erase_if(_sprites, [&sprite](auto const& spr) { return spr.get() == sprite; });
}

void sprite_manager::clear()
{
    _sprites.clear();
}

void sprite_manager::sort_by_y()
{
    _spriteBatch.sort_by_y_position();
    _spriteBatch.send_to_back(*_background);
    _spriteBatch.bring_to_front(*_foreground);
}

void sprite_manager::define_texture_region(string const& region, rect_i const& uv)
{
    _spriteTexture->regions()[region] =
        {.UVRect = {static_cast<f32>(uv.left()) / static_cast<f32>(SPRITE_TEXTURE_SIZE.Width),
                    static_cast<f32>(uv.top()) / static_cast<f32>(SPRITE_TEXTURE_SIZE.Height),
                    static_cast<f32>(uv.width()) / static_cast<f32>(SPRITE_TEXTURE_SIZE.Width),
                    static_cast<f32>(uv.height()) / static_cast<f32>(SPRITE_TEXTURE_SIZE.Height)},
         .Level  = 0};
}

void sprite_manager::wrap_and_update()
{
    wrap();
    _spriteBatch.update(0ms);
}

void sprite_manager::wrap_and_collide()
{
    wrap();
    collide();
    _spriteBatch.send_to_back(*_background);
    _spriteBatch.bring_to_front(*_foreground);
}

void sprite_manager::update(milliseconds deltaTime)
{
    if (_updateSprites) {
        _updateSprites = false;
        _spriteTexture->update_data(Sprites, 0);
    }
    if (_updateBackground) {
        _updateBackground = false;
        _backgroundTexture->update_data(Background, 0);
    }
    if (_updateForeground) {
        _updateForeground = false;
        _foregroundTexture->update_data(Foreground, 0);
    }
    _spriteBatch.update(deltaTime);
}

void sprite_manager::draw_to(gfx::render_target& target)
{
    transform xform;
    _spriteBatch.draw_to(target, xform);
}

auto sprite_manager::add_shape() -> gfx::rect_shape*
{
    return &_spriteBatch.create_shape<gfx::rect_shape>();
}

auto sprite_manager::remove_shape(gfx::shape* shape) -> bool
{
    return _spriteBatch.remove_shape(*shape);
}

void sprite_manager::wrap()
{
    auto const fieldSize {_background->Bounds->Size};

    for (auto& s : _sprites) {
        if (!s) { continue; }
        if (!s->Shape || !s->is_wrappable()) {
            if (s->WrapCopy) {
                _spriteBatch.remove_shape(*s->WrapCopy);
                s->WrapCopy = nullptr;
            }
            continue;
        }

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

void sprite_manager::collide()
{
    std::vector<collision_event> events;

    auto const collide {[&events](gfx::rect_shape* a, gfx::rect_shape* b, sprite* sA, sprite* sB) {
        rect_f const inter {a->aabb(transform::Identity).as_intersection_with(b->aabb(transform::Identity))};
        if (inter == rect_f::Zero) { return; }

        auto const* texA {sA->get_texture()};
        assert(texA->Alpha.size() == size_i {a->Bounds->Size});

        auto const* texB {sB->get_texture()};
        assert(texB->Alpha.size() == size_i {b->Bounds->Size});

        auto const [leftA, topA] {a->Bounds->Position};
        auto const [widthA, heightA] {a->Bounds->Size};

        auto const [leftB, topB] {b->Bounds->Position};
        auto const [widthB, heightB] {b->Bounds->Size};

        i32 const bottomI {static_cast<i32>(inter.bottom())};
        i32 const rightI {static_cast<i32>(inter.right())};

        for (i32 y {static_cast<i32>(inter.top())}; y < bottomI; ++y) {
            for (i32 x {static_cast<i32>(inter.left())}; x < rightI; ++x) {
                i32 const ax {static_cast<i32>(x - leftA)};
                i32 const ay {static_cast<i32>(y - topA)};
                i32 const bx {static_cast<i32>(x - leftB)};
                i32 const by {static_cast<i32>(y - topB)};

                if (ax < 0 || ax >= static_cast<i32>(widthA)
                    || ay < 0 || ay >= static_cast<i32>(heightA)
                    || bx < 0 || bx >= static_cast<i32>(widthB)
                    || by < 0 || by >= static_cast<i32>(heightB)) {
                    continue;
                }

                if (texA->Alpha[ax, ay] > 250 && texB->Alpha[bx, by] > 250) {
                    events.push_back({.A = sA, .B = sB});
                    return;
                }
            }
        }
    }};

    usize const spriteCount {_sprites.size()};
    for (usize i {0}; i < spriteCount; ++i) {
        auto const& spriteA {_sprites[i]};
        if (!spriteA->is_collidable()) { continue; }

        std::array<gfx::rect_shape*, 2> shapesA {spriteA->Shape, spriteA->WrapCopy};
        i32                             countA {spriteA->WrapCopy ? 2 : 1};

        for (usize j {i + 1}; j < spriteCount; ++j) {
            auto const& spriteB {_sprites[j]};
            if (!spriteB->is_collidable()) { continue; }

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
        _events.SpriteCollision(event);
    }
}
