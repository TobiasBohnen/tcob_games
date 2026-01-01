// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Sprite.hpp"

#include <utility>

////////////////////////////////////////////////////////////

sprite::sprite(init init)
    : _init {std::move(init)}
{
}

auto sprite::is_collidable() const -> bool { return _init.Def.IsCollidable; }
auto sprite::is_wrappable() const -> bool { return _init.Def.IsWrappable; }
auto sprite::owner() const -> scripting::table const& { return _init.Owner; }
auto sprite::get_texture() const -> texture* { return _init.Texture; }

void sprite::set_bounds(point_f pos, size_f size)
{
    if (!Shape) { return; }

    Bounds        = {pos, size};
    Shape->Bounds = rect_f {rect_i {Bounds}};
}

void sprite::set_texture(texture* tex)
{
    if (!tex || !Shape) { return; }

    _init.Texture        = tex;
    Shape->TextureRegion = tex->Region;
    if (WrapCopy) { WrapCopy->TextureRegion = tex->Region; }

    set_bounds(Bounds.Position, tex->Size);
}