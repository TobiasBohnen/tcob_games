// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ScreenOverlay.hpp"

#include <array>
#include <cctype>
#include <vector>

#include "Common.hpp"
#include "Level.hpp"
#include "Player.hpp"
#include "TextureCache.hpp"

////////////////////////////////////////////////////////////

static void BlitMasked(u32* dst, size_i screenSize, u8 const* tex, i32 texPitch, point_i srcOrigin, size_i srcSize, size_i drawSize, point_i dstPos, f64 scale)
{
    i32 const xStart {std::max(0, -dstPos.X)};
    i32 const xEnd {std::min(drawSize.Width, screenSize.Width - dstPos.X)};
    i32 const yStart {std::max(0, -dstPos.Y)};
    i32 const yEnd {std::min(drawSize.Height, screenSize.Height - dstPos.Y)};

    for (i32 y {yStart}; y < yEnd; ++y) {
        i32 const  texY {srcOrigin.Y + std::min(srcSize.Height - 1, static_cast<i32>(y / scale))};
        u32* const row {dst + PixelIndex(screenSize, dstPos.X, y + dstPos.Y)};

        for (i32 x {xStart}; x < xEnd; ++x) {
            i32 const texX {srcOrigin.X + std::min(srcSize.Width - 1, static_cast<i32>(x / scale))};
            i32 const texOffset {(texX + (texY * texPitch)) * TEXTURE_BPP};
            if (IsMagenta(tex, texOffset)) { continue; }

            row[x] = 0xFF000000u | (static_cast<u32>(tex[texOffset + 2]) << 16) | (static_cast<u32>(tex[texOffset + 1]) << 8) | static_cast<u32>(tex[texOffset + 0]);
        }
    }
}

screen_overlay::screen_overlay(texture_cache& cache)
    : _cache {cache}
    , _taskManager {locate_service<task_manager>()}
{
}

void screen_overlay::draw_screen_effect()
{
    if (ScreenEffect) {
        ScreenEffect(_screen);
    }
}

void screen_overlay::draw_weapon(player const& player)
{
    auto* const tex {_cache.texture(handTexture, 0)};
    auto const  texSize {_cache.texture_size(handTexture, 0)};

    f64 const     scale {_screenSize.Height / REFERENCE_HEIGHT};
    size_i const  drawSize {size_f {texSize} * scale};
    i32 const     bobOffsetY {static_cast<i32>(player.BobAmount * WEAPON_BOB_MULTIPLIER)};
    point_i const offset {static_cast<i32>((_screenSize.Width - drawSize.Width) * 0.75),
                          static_cast<i32>(_screenSize.Height - (drawSize.Height * 0.75)) + bobOffsetY};

    BlitMasked(_screen.data(), _screenSize, tex, texSize.Width, point_i {0, 0}, texSize, drawSize, offset, scale);
}

void screen_overlay::draw_hud(player const&)
{
    auto* const tex {_cache.texture(hudTexture, 0)};
    auto const  texSize {_cache.texture_size(hudTexture, 0)};

    f64 const    scale {_screenSize.Height / REFERENCE_HEIGHT};
    size_i const drawSize {size_f {texSize} * scale};

    BlitMasked(_screen.data(), _screenSize, tex, texSize.Width, point_i {0, 0}, texSize, drawSize, point_i {0, 0}, scale);
}

void screen_overlay::draw_message(level const& level)
{
    auto const& msg {level.get_message()};
    if (msg.empty()) { return; }

    auto* const tex {_cache.texture(fontTexture, 0)};
    auto const  texSize {_cache.texture_size(fontTexture, 0)};
    auto const  charSize {texSize.Height};

    point_i const offset {5, 5};
    size_i const  glyphSize {charSize, charSize};

    for (i32 i {0}; i < std::ssize(msg); ++i) {
        char const c {msg[i]};
        if (std::isspace(static_cast<unsigned char>(c))) { continue; }

        i32 index {0};
        if (std::isdigit(static_cast<unsigned char>(c))) {
            index = c - '0';
        } else if (std::isalpha(static_cast<unsigned char>(c))) {
            index = 10 + (std::tolower(static_cast<unsigned char>(c)) - 'a');
        }

        BlitMasked(_screen.data(), _screenSize, tex, texSize.Width, point_i {index * charSize, 0}, glyphSize, glyphSize, point_i {offset.X + (i * (charSize + 1)), offset.Y}, 1.0);
    }
}
