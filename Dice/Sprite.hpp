// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

////////////////////////////////////////////////////////////

struct texture {
    u32      ID {0};
    size_f   Size;
    string   Region;
    grid<u8> Alpha;
};

////////////////////////////////////////////////////////////

struct sprite_def {
    std::optional<u32> TexID;
    point_f            Position;
    bool               IsCollidable {true};
    bool               IsWrappable {true};

    static auto constexpr Members()
    {
        return std::tuple {member<&sprite_def::TexID, std::nullopt> {"texture"},
                           member<&sprite_def::Position, point_f {0, 0}> {"position"},
                           member<&sprite_def::IsCollidable, true> {"collidable"},
                           member<&sprite_def::IsWrappable, true> {"wrappable"}};
    }
};

////////////////////////////////////////////////////////////

class sprite {
public:
    struct init {
        sprite_def       Def;
        texture*         Texture {};
        scripting::table Owner;
    };

    explicit sprite(init init);

    rect_f           Bounds;
    gfx::rect_shape* Shape {nullptr};
    gfx::rect_shape* WrapCopy {nullptr};

    auto is_collidable() const -> bool;
    auto is_wrappable() const -> bool;
    auto owner() const -> scripting::table const&;
    auto get_texture() const -> texture*;

    void set_bounds(point_f pos, size_f size);
    void set_texture(texture* tex);

private:
    init _init;
};