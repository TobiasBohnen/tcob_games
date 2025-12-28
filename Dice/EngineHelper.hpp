// Copyright (c) 2025 Tobias Bohnen
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

enum game_status : u8 {
    Running   = 0,
    TurnEnded = 1,
    GameOver  = 2
};

////////////////////////////////////////////////////////////

template <typename T = void>
using callback = std::optional<scripting::function<T>>;

struct callbacks {
    callback<>     OnCollision;
    callback<>     OnDieChange;
    callback<>     OnDieMotion;
    callback<>     OnSetup;
    callback<>     OnTeardown;
    callback<i32>  OnTurnUpdate;
    callback<>     OnTurnFinish;
    callback<bool> CanStartTurn;
    callback<>     OnTurnStart;
};

////////////////////////////////////////////////////////////

struct sprite_def {
    u32  Texture {0};
    bool IsCollidable {true};
    bool IsWrappable {true};

    static auto constexpr Members()
    {
        return std::tuple {member<&sprite_def::Texture> {"texture"},
                           member<&sprite_def::IsCollidable, true> {"collidable"},
                           member<&sprite_def::IsWrappable, true> {"wrappable"}};
    }
};

////////////////////////////////////////////////////////////

struct bg_def {
    string Bitmap;

    static auto constexpr Members()
    {
        return std::tuple {member<&bg_def::Bitmap> {"bitmap"}};
    }
};

////////////////////////////////////////////////////////////

struct tex_def {
    size_i              Size {size_i::Zero};
    string              Bitmap;
    std::optional<u32>  Transparent;
    std::optional<u32>  Rotation;
    std::optional<bool> FlipH;
    std::optional<bool> FlipV;

    static auto constexpr Members()
    {
        return std::tuple {member<&tex_def::Size> {"size"},
                           member<&tex_def::Bitmap> {"bitmap"},
                           member<&tex_def::Transparent, std::nullopt> {"transparent"},
                           member<&tex_def::Rotation, std::nullopt> {"rotation"},
                           member<&tex_def::FlipH, std::nullopt> {"flip_h"},
                           member<&tex_def::FlipV, std::nullopt> {"flip_v"}};
    }
};

////////////////////////////////////////////////////////////

class dmd_proxy {
public:
    dmd_proxy(prop<grid<u8>>& dmd);

    void clear();

    void line(point_i start, point_i end, u8 color);
    void circle(point_i center, i32 radius, u8 color, bool fill);
    void rect(rect_i const& rect, u8 color, bool fill);

    void blit(rect_i const& rect, string const& dotStr);
    void print(point_i pos, string_view text, u8 color);

private:
    prop<grid<u8>>& _dmd;
};

////////////////////////////////////////////////////////////

auto decode_texture_pixels(string_view s, size_i size) -> std::vector<u8>;