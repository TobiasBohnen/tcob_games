// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

////////////////////////////////////////////////////////////

struct blit_settings {
    std::optional<u32>                        Transparent;
    u32                                       Rotation {0};
    f32                                       Scale {1};
    bool                                      FlipH {false};
    bool                                      FlipV {false};
    std::optional<std::unordered_map<u8, u8>> Swap;

    static auto constexpr Members()
    {
        return std::tuple {
            member<&blit_settings::Transparent, std::nullopt> {"transparent"},
            member<&blit_settings::Rotation, 0> {"rotation"},
            member<&blit_settings::Scale, 1.0f> {"scale"},
            member<&blit_settings::FlipH, false> {"flip_h"},
            member<&blit_settings::FlipV, false> {"flip_v"},
            member<&blit_settings::Swap, std::nullopt> {"swap"},
        };
    }
};

////////////////////////////////////////////////////////////

enum class font_type : u8 {
    Font8x8,
    Font6x8,
    Font5x7,
    Font5x5,
    Font5x4,
    Font4x6,
    Font4x5,
    Font4x4,
    Font3x5,
};

////////////////////////////////////////////////////////////

class tex_proxy {
public:
    tex_proxy(prop<gfx::image>& img, color clear);

    auto bounds() const -> rect_i;

    void clear(std::optional<rect_i> const& rect);

    void pixel(point_i pos, u8 color);
    void line(point_i start, point_i end, u8 color);
    void circle(point_i center, i32 radius, u8 ccolor, bool fill);
    void rect(rect_i const& rect, u8 color, bool fill);

    void blit(rect_i const& rect, string const& data, blit_settings settings);
    void print(point_i pos, string_view text, u8 color, font_type type);

    void socket(socket* socket);

private:
    static void draw(std::span<u8> data, i32 x, i32 y, size_i s, color color);

    prop<gfx::image>& _img;
    size_i            _imgSize;
    color             _clear;
};

////////////////////////////////////////////////////////////

auto decode_texture_pixels(string_view s, size_i size) -> std::vector<u8>;

////////////////////////////////////////////////////////////
