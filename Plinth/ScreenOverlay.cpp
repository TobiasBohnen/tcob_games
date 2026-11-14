// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ScreenOverlay.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <span>

#include "Common.hpp"
#include "Player.hpp"
#include "TextureCache.hpp"

////////////////////////////////////////////////////////////

struct blit_params {
    u8 const* Tex {};
    i32       TexPitch {};
    rect_i    Src;
    rect_i    Dst;
};

void BlitMasked(u32* dst, size_i screenSize, blit_params const& p)
{
    auto const srcOrigin {p.Src.Position};
    auto const srcSize {p.Src.Size};
    auto const dstPos {p.Dst.Position};
    auto const drawSize {p.Dst.Size};

    i32 const xStart {std::max(0, -dstPos.X)};
    i32 const xEnd {std::min(drawSize.Width, screenSize.Width - dstPos.X)};
    i32 const yStart {std::max(0, -dstPos.Y)};
    i32 const yEnd {std::min(drawSize.Height, screenSize.Height - dstPos.Y)};
    if (xStart >= xEnd || yStart >= yEnd) { return; }

    for (i32 y {yStart}; y < yEnd; ++y) {
        i32 const  texY {srcOrigin.Y + (y * srcSize.Height / drawSize.Height)};
        u32* const row {dst + PixelIndex(screenSize.Width, dstPos.X + xStart, y + dstPos.Y)};

        for (i32 x {xStart}; x < xEnd; ++x) {
            i32 const texX {srcOrigin.X + (x * srcSize.Width / drawSize.Width)};
            i32 const texOffset {(texX + (texY * p.TexPitch)) * TEXTURE_BPP};
            if (IsMagenta(p.Tex, texOffset)) { continue; }

            row[x - xStart] = 0xFF000000u | (static_cast<u32>(p.Tex[texOffset + 2]) << 16) | (static_cast<u32>(p.Tex[texOffset + 1]) << 8) | static_cast<u32>(p.Tex[texOffset + 0]);
        }
    }
}

screen_overlay::screen_overlay(texture_cache& cache)
    : _cache {cache}
{
}

void screen_overlay::update(milliseconds deltaTime)
{
    if (!_message.empty()) {
        _messageTimer -= deltaTime;
        if (_messageTimer.count() <= 0) {
            _message.clear();
        }
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
    constexpr static f64 REFERENCE_HEIGHT {360};
    constexpr static f64 BOB_MULTIPLIER {2.0};

    auto* const tex {_cache.texture(handTexture, 0)};
    auto const  texSize {_cache.texture_size(handTexture, 0)};

    f64 const    scale {static_cast<f64>(_screenSize.Height) / REFERENCE_HEIGHT};
    size_i const drawSize {static_cast<i32>(std::round(texSize.Width * scale)),
                           static_cast<i32>(std::round(texSize.Height * scale))};
    i32 const    bobOffsetY {static_cast<i32>(player.bob_amount() * BOB_MULTIPLIER)};
    i32 const    drawX {static_cast<i32>((_screenSize.Width - drawSize.Width) * 0.75)};
    i32 const    drawY {static_cast<i32>(_screenSize.Height - (drawSize.Height * 0.75)) + bobOffsetY};

    BlitMasked(_screen.data(), _screenSize,
               {.Tex      = tex,
                .TexPitch = texSize.Width,
                .Src      = {0, 0, texSize.Width, texSize.Height},
                .Dst      = {drawX, drawY, drawSize.Width, drawSize.Height}});
}

void screen_overlay::draw_hud(player const& player)
{
    constexpr static f64 REFERENCE_HEIGHT {180};

    constexpr f64 COMPASS_NORTH_POS {0.375};
    constexpr f64 COMPASS_TOP_MARGIN {5.0};
    constexpr u32 COMPASS_MARKER_COLOR {0xFF0000FFu};

    auto const* compassTex {_cache.texture(compassTexture, 0)};
    if (!compassTex) { return; }

    auto const compassTexSize {_cache.texture_size(compassTexture, 0)};
    i32 const  texWidth {compassTexSize.Width};
    i32 const  texHeight {compassTexSize.Height};
    i32 const  compassVisibleWidth {texWidth / 4 * 3};

    f64 const scale {static_cast<f64>(_screenSize.Height) / REFERENCE_HEIGHT};

    i32 const drawHeight {static_cast<i32>(std::round(texHeight * scale))};
    i32 const drawWidth {static_cast<i32>(std::round(compassVisibleWidth * scale))};

    i32 const drawX {(_screenSize.Width - drawWidth) / 2};
    i32 const drawY {static_cast<i32>(COMPASS_TOP_MARGIN * scale)};

    point_d const worldNorth {0.0, -1.0};
    f64 const     forwardDot {player.Direction.dot(worldNorth)};
    f64 const     rightDot {player.Direction.as_perpendicular().dot(worldNorth)};
    f64 const     relativeAngle {-std::atan2(rightDot, forwardDot)};

    // Texture pixel that sits at the screen center for the current heading.
    i32 const center {static_cast<i32>(std::lround((COMPASS_NORTH_POS + (relativeAngle / TAU)) * texWidth))};

    i32 srcStart {(center - (compassVisibleWidth / 2)) % texWidth};
    if (srcStart < 0) { srcStart += texWidth; }

    i32 const firstSrcWidth {std::min(compassVisibleWidth, texWidth - srcStart)};
    i32 const secondSrcWidth {compassVisibleWidth - firstSrcWidth};

    i32 const firstDrawWidth {std::min(drawWidth, static_cast<i32>(std::round(firstSrcWidth * scale)))};
    i32 const secondDrawWidth {drawWidth - firstDrawWidth};

    BlitMasked(_screen.data(), _screenSize,
               {.Tex      = compassTex,
                .TexPitch = texWidth,
                .Src      = {srcStart, 0, firstSrcWidth, texHeight},
                .Dst      = {drawX, drawY, firstDrawWidth, drawHeight}});

    if (secondSrcWidth > 0 && secondDrawWidth > 0) {
        BlitMasked(_screen.data(), _screenSize,
                   {.Tex      = compassTex,
                    .TexPitch = texWidth,
                    .Src      = {0, 0, secondSrcWidth, texHeight},
                    .Dst      = {drawX + firstDrawWidth, drawY, secondDrawWidth, drawHeight}});
    }

    i32 const markerX {drawX + (drawWidth / 2)};
    i32 const markerY {drawY};
    i32 const markerHeight {drawY + drawHeight};
    for (i32 y {markerY}; y < markerHeight; ++y) {
        _screen[PixelIndex(_screenSize.Width, markerX, y)] = COMPASS_MARKER_COLOR;
    }
}

void screen_overlay::draw_message()
{
    if (_message.empty()) { return; }

    auto* const tex {_cache.texture(fontTexture, 0)};
    auto const  texSize {_cache.texture_size(fontTexture, 0)};
    auto const  charSize {texSize.Height};

    point_i const offset {5, 5};

    for (i32 i {0}; i < std::ssize(_message); ++i) {
        char const c {_message[i]};

        // Explicit ASCII mapping: locale-independent, and unsupported characters are skipped.
        i32 index {-1};
        if (c >= '0' && c <= '9') {
            index = c - '0';
        } else if (c >= 'a' && c <= 'z') {
            index = 10 + (c - 'a');
        } else if (c >= 'A' && c <= 'Z') {
            index = 10 + (c - 'A');
        }
        if (index < 0) { continue; }

        BlitMasked(_screen.data(), _screenSize,
                   {.Tex      = tex,
                    .TexPitch = texSize.Width,
                    .Src      = {index * charSize, 0, charSize, charSize},
                    .Dst      = {offset.X + (i * (charSize + 1)), offset.Y, charSize, charSize}});
    }
}
