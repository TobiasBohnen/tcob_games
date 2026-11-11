// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ScreenOverlay.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <span>
#include <vector>

#include "Common.hpp"
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

void screen_overlay::update(milliseconds deltaTime)
{
    _messageTimer -= deltaTime;
    if (_messageTimer.count() <= 0) {
        _message.clear();
    }
}

auto screen_overlay::draw(player const& player, std::span<u32> screen, size_i screenSize) -> std::span<u32>
{
    _screen     = screen;
    _screenSize = screenSize;

    draw_weapon(player);

    draw_screen_effect();

    draw_hud(player);
    draw_message();

    return _screen;
}

void screen_overlay::show_message(string const& msg)
{
    _messageTimer = 2s;
    _message      = msg;
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
    i32 const     bobOffsetY {static_cast<i32>(player.bob_amount() * WEAPON_BOB_MULTIPLIER)};
    point_i const offset {static_cast<i32>((_screenSize.Width - drawSize.Width) * 0.75),
                          static_cast<i32>(_screenSize.Height - (drawSize.Height * 0.75)) + bobOffsetY};

    BlitMasked(_screen.data(), _screenSize, tex, texSize.Width, point_i {0, 0}, texSize, drawSize, offset, scale);
}

void screen_overlay::draw_hud(player const& p)
{
    // compass
    auto const* compassTex {_cache.texture(compassTexture, 0)};
    if (!compassTex) { return; }
    auto const compassTexSize {_cache.texture_size(compassTexture, 0)};

    point_d const worldNorth {0.0, -1.0};
    f64 const     forwardDot {p.Direction.dot(worldNorth)};
    f64 const     rightDot {p.Direction.as_perpendicular().dot(worldNorth)};
    f64 const     relativeAngle {-std::atan2(rightDot, forwardDot)};

    f64 const scale {_screenSize.Height / REFERENCE_HEIGHT};
    f64 const barWidth {300.0 * scale};
    f64 const barHeight {static_cast<f64>(compassTexSize.Height) * scale};

    f64 const screenX {(_screenSize.Width - barWidth) * 0.5};
    f64 const screenY {5.0 * scale};

    f64 const textureSpan {static_cast<f64>(compassTexSize.Width)};
    f64 const northTexPos {0.375 * textureSpan};
    f64 const centerOffsetTex {(relativeAngle / TAU) * textureSpan};

    f64 srcX {northTexPos + centerOffsetTex - ((barWidth / scale) * 0.5)};

    srcX = std::fmod(srcX, textureSpan);
    if (srcX < 0.0) { srcX += textureSpan; }

    f64 const visibleTexWidth {barWidth / scale};

    f64 const firstPartTexWidth {std::min(visibleTexWidth, textureSpan - srcX)};
    f64 const secondPartTexWidth {std::max(0.0, visibleTexWidth - firstPartTexWidth)};

    f64 const firstPartDrawWidth {firstPartTexWidth * scale};
    f64 const secondPartDrawWidth {secondPartTexWidth * scale};

    BlitMasked(
        _screen.data(),
        _screenSize,
        compassTex,
        compassTexSize.Width,
        point_i {static_cast<i32>(srcX), 0},
        size_i {static_cast<i32>(firstPartTexWidth), compassTexSize.Height},
        size_i {static_cast<i32>(firstPartDrawWidth), static_cast<i32>(barHeight)},
        point_i {static_cast<i32>(screenX), static_cast<i32>(screenY)},
        scale);

    if (secondPartTexWidth > 0.0) {
        BlitMasked(
            _screen.data(),
            _screenSize,
            compassTex,
            compassTexSize.Width,
            point_i {0, 0},
            size_i {static_cast<i32>(secondPartTexWidth), compassTexSize.Height},
            size_i {static_cast<i32>(secondPartDrawWidth), static_cast<i32>(barHeight)},
            point_i {static_cast<i32>(screenX + firstPartDrawWidth), static_cast<i32>(screenY)},
            scale);
    }

    i32 const markerX {static_cast<i32>(_screenSize.Width * 0.5)};
    i32 const markerY {static_cast<i32>(screenY)};
    i32 const markerHeight {static_cast<i32>(barHeight)};

    for (i32 y {markerY}; y < markerY + markerHeight; ++y) {
        _screen[PixelIndex(_screenSize, markerX, y)] = 0xFF0000FFu;
    }
}

void screen_overlay::draw_message()
{
    if (_message.empty()) { return; }

    auto* const tex {_cache.texture(fontTexture, 0)};
    auto const  texSize {_cache.texture_size(fontTexture, 0)};
    auto const  charSize {texSize.Height};

    point_i const offset {5, 5};
    size_i const  glyphSize {charSize, charSize};

    for (i32 i {0}; i < std::ssize(_message); ++i) {
        char const c {_message[i]};
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
