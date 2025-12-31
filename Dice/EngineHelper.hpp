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
    callback<>    OnCollision;
    callback<>    OnSetup;
    callback<>    OnTeardown;
    callback<i32> OnTurnUpdate;
    callback<>    OnTurnFinish;
    callback<>    OnTurnStart;
    callback<>    DrawDMD;
};

////////////////////////////////////////////////////////////

struct sprite_def {
    u32     Texture {0};
    point_f Position;
    bool    IsCollidable {true};
    bool    IsWrappable {true};

    static auto constexpr Members()
    {
        return std::tuple {member<&sprite_def::Texture, 0> {"texture"},
                           member<&sprite_def::Position, point_f {0, 0}> {"position"},
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

struct blit_settings {
    std::optional<u32>  Transparent;
    std::optional<u32>  Rotation;
    std::optional<bool> FlipH;
    std::optional<bool> FlipV;

    static auto constexpr Members()
    {
        return std::tuple {member<&blit_settings::Transparent, std::nullopt> {"transparent"},
                           member<&blit_settings::Rotation, std::nullopt> {"rotation"},
                           member<&blit_settings::FlipH, std::nullopt> {"flip_h"},
                           member<&blit_settings::FlipV, std::nullopt> {"flip_v"}};
    }
};

////////////////////////////////////////////////////////////

class dmd_proxy {
public:
    dmd_proxy(prop<grid<u8>>& dmd);

    auto size() const -> size_i;

    void clear();

    void line(point_i start, point_i end, u8 color);
    void circle(point_i center, i32 radius, u8 color, bool fill);
    void rect(rect_i const& rect, u8 color, bool fill);

    void blit(rect_i const& rect, string const& dotStr);
    void print(point_i pos, string_view text, u8 color);

    void draw_socket(socket* socket, rect_f const& dmdBounds);

private:
    prop<grid<u8>>& _dmd;
};

////////////////////////////////////////////////////////////

class tex_proxy {
public:
    tex_proxy(prop<gfx::image>& img, color clear);

    auto bounds() const -> rect_i;

    void clear();

    void line(point_i start, point_i end, u8 color);
    void circle(point_i center, i32 radius, u8 color, bool fill);
    void rect(rect_i const& rect, u8 color, bool fill);

    void blit(rect_i const& rect, string const& data, blit_settings settings);

private:
    prop<gfx::image>& _img;
    color             _clear;
};

////////////////////////////////////////////////////////////

auto decode_texture_pixels(string_view s, size_i size) -> std::vector<u8>;