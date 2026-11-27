// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ScreenOverlay.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <span>

#include "Common.hpp"
#include "Level.hpp"
#include "LevelObject.hpp"
#include "Player.hpp"
#include "TextureCache.hpp"

////////////////////////////////////////////////////////////

static constexpr i32 UI_REFERENCE_HEIGHT {180};
static constexpr i32 FONT_GLYPH_SIZE {5};
static constexpr u32 TEXT_SHADOW_COLOR {0xFF000000u};
static constexpr i32 FONT_FALLBACK_GLYPH {47};

struct blit_params {
    u8 const* Tex {};
    i32       TexPitch {};
    rect_i    Src;
    rect_i    Dst;
    u32       Tint {0}; // 0 = use texture color, otherwise draw every opaque texel in this color
};

static void BlitMasked(u32* dst, size_i _screenSize, blit_params const& p)
{
    auto const srcOrigin {p.Src.Position};
    auto const srcSize {p.Src.Size};
    auto const dstPos {p.Dst.Position};
    auto const drawSize {p.Dst.Size};

    i32 const xStart {std::max(0, -dstPos.X)};
    i32 const xEnd {std::min(drawSize.Width, _screenSize.Width - dstPos.X)};
    i32 const yStart {std::max(0, -dstPos.Y)};
    i32 const yEnd {std::min(drawSize.Height, _screenSize.Height - dstPos.Y)};
    if (xStart >= xEnd || yStart >= yEnd) { return; }

    for (i32 y {yStart}; y < yEnd; ++y) {
        i32 const  texY {srcOrigin.Y + (y * srcSize.Height / drawSize.Height)};
        u32* const row {dst + PixelIndex(_screenSize.Width, dstPos.X + xStart, y + dstPos.Y)};

        for (i32 x {xStart}; x < xEnd; ++x) {
            i32 const texX {srcOrigin.X + (x * srcSize.Width / drawSize.Width)};
            i32 const texOffset {(texX + (texY * p.TexPitch)) * TEXTURE_BPP};
            if (IsMagenta(p.Tex, texOffset)) { continue; }

            if (p.Tint != 0) {
                row[x - xStart] = p.Tint;
            } else {
                row[x - xStart] = 0xFF000000u | (static_cast<u32>(p.Tex[texOffset + 2]) << 16) | (static_cast<u32>(p.Tex[texOffset + 1]) << 8) | static_cast<u32>(p.Tex[texOffset + 0]);
            }
        }
    }
}

static auto UIScale(size_i screenSize) -> i32
{
    return std::max(1, screenSize.Height / UI_REFERENCE_HEIGHT);
}

static auto GlyphIndex(char c) -> i32
{
    if (c >= '0' && c <= '9') { return c - '0'; }
    if (c >= 'a' && c <= 'z') { return 10 + (c - 'a'); }
    if (c >= 'A' && c <= 'Z') { return 10 + (c - 'A'); }

    switch (c) {
    case '*':  return 36;
    case '+':  return 37;
    case '/':  return 38;
    case '-':  return 39;
    case '!':  return 40;
    case '?':  return 41;
    case ';':  return 42;
    case ':':  return 43;
    case '.':  return 44;
    case ',':  return 45;
    case ' ':  return -1;
    case '\t': return -1;
    default:   return FONT_FALLBACK_GLYPH;
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

auto screen_overlay::draw(level const& lvl, player const& player, std::span<u32> screen, size_i screenSize) -> std::span<u32>
{
    _screen     = screen;
    _screenSize = screenSize;

    draw_target(lvl);
    draw_weapon(player);

    draw_screen_effect();

    draw_compass(player);
    draw_message();

    return _screen;
}

void screen_overlay::show_message(string const& msg)
{
    _messageTimer = 2s;
    _message      = msg;
}

void screen_overlay::draw_screen_effect() const
{
    if (ScreenEffect) {
        ScreenEffect(_screen);
    }
}

void screen_overlay::draw_target(level const& lvl) const
{
    constexpr u32 TARGET_COLOR {0xFF878F99u};
    constexpr u32 TAG_BACK_COLOR {0xFF101010u};
    constexpr i32 CORNER_LEN {6};
    constexpr i32 TICK_LEN {2};

    i32 const maxScale {UIScale(_screenSize)};

    for (auto const& o : lvl.objects()) {
        auto const& obj {base_of(o)};
        if (!obj.is_seen()) { continue; }
        auto const* vo {obj.voxel()};
        if (!vo || !vo->ScreenBounds) { continue; }

        auto const bounds {*vo->ScreenBounds};

        i32 const left {bounds.Left};
        i32 const top {bounds.Top};
        i32 const right {bounds.Right};
        i32 const bottom {bounds.Bottom};

        assert(left >= 0 && left < _screenSize.Width && right > 0 && right <= _screenSize.Width && top >= 0 && top < _screenSize.Height && bottom > 0 && bottom <= _screenSize.Height);

        i32 const w {right - left};
        i32 const h {bottom - top};

        // Bracket thickness shrinks for small (distant) targets.
        i32 const  s {std::clamp(std::min(w, h) / 16, 1, maxScale)};
        i32 const  arm {std::min(CORNER_LEN * s, std::min(w, h) / 2)};
        u32* const px {_screen.data()};
        i32 const  pitch {_screenSize.Width};

        // The assert above guarantees the box is on screen, so no clamping is needed.
        auto const set {[&](i32 x, i32 y) { px[PixelIndex(pitch, x, y)] = TARGET_COLOR; }};

        // corner brackets: one pass, 8 pixels per step
        for (i32 i {0}; i < arm; ++i) {
            for (i32 t {0}; t < s; ++t) {
                set(left + i, top + t);
                set(left + t, top + i);
                set(right - 1 - i, top + t);
                set(right - 1 - t, top + i);
                set(left + i, bottom - 1 - t);
                set(left + t, bottom - 1 - i);
                set(right - 1 - i, bottom - 1 - t);
                set(right - 1 - t, bottom - 1 - i);
            }
        }

        // mid-edge ticks, only when the box is large enough to not touch the corners
        if (w >= 16 * s && h >= 16 * s) {
            i32 const cx {left + (w / 2)};
            i32 const cy {top + (h / 2)};
            i32 const tick {TICK_LEN * s};
            for (i32 i {0}; i < tick; ++i) {
                for (i32 t {0}; t < s; ++t) {
                    set(cx + t, top + i);
                    set(cx + t, bottom - 1 - i);
                    set(left + i, cy + t);
                    set(right - 1 - i, cy + t);
                }
            }
        }

        // Label: scale shrinks with the box (min 1x). If the text doesn't fit, it is cut and ends with "...".
        constexpr u32 TAG_TEXT_COLOR {0xFFFFFFFFu};
        constexpr i32 CHAR_ADVANCE {FONT_GLYPH_SIZE + 1};
        constexpr i32 TAG_PADDING {4};    // accent bar + gaps, in texels
        constexpr i32 ELLIPSIS_WIDTH {5}; // 3 dots with 1 texel gaps

        string_view const label {obj.name()};
        i32 const         labelLen {static_cast<i32>(label.size())};
        i32 const         fullTexels {(labelLen * CHAR_ADVANCE) - 1 + TAG_PADDING};

        i32 const ls {std::clamp(w / fullTexels, 1, maxScale)};
        i32 const fitChars {((w / ls) - (TAG_PADDING - 1)) / CHAR_ADVANCE}; // chars that fit without ellipsis

        bool ellipsis {false};
        i32  shown {labelLen};
        if (fitChars < labelLen) {
            ellipsis = true;
            shown    = fitChars - 1; // one slot is taken by the dots
        }
        if (shown < 1) { continue; }

        i32 const tagTexelsW {(shown * CHAR_ADVANCE) + (ellipsis ? ELLIPSIS_WIDTH : -1) + TAG_PADDING};
        i32 const tagW {tagTexelsW * ls};
        i32 const tagH {(FONT_GLYPH_SIZE + 2) * ls};
        i32 const tagX {std::clamp(left, 0, std::max(0, _screenSize.Width - tagW))};
        i32 const tagY {std::max(0, top - tagH - ls)};

        // label plate: dark background, first `ls` columns are the accent bar
        for (i32 y {0}; y < tagH; ++y) {
            for (i32 x {0}; x < tagW; ++x) {
                px[PixelIndex(pitch, tagX + x, tagY + y)] = x < ls ? TARGET_COLOR : TAG_BACK_COLOR;
            }
        }

        i32 const textX {tagX + (2 * ls)};
        i32 const textY {tagY + ls};
        draw_text(label.substr(0, static_cast<usize>(shown)), {textX, textY}, ls, false);

        if (ellipsis) {
            // three 1-texel dots on the glyph baseline
            i32 const dotsX {textX + (shown * CHAR_ADVANCE * ls)};
            i32 const dotsY {textY + ((FONT_GLYPH_SIZE - 1) * ls)};
            for (i32 d {0}; d < 3; ++d) {
                for (i32 dy {0}; dy < ls; ++dy) {
                    for (i32 dx {0}; dx < ls; ++dx) {
                        px[PixelIndex(pitch, dotsX + (d * 2 * ls) + dx, dotsY + dy)] = TAG_TEXT_COLOR;
                    }
                }
            }
        }
    }
}

void screen_overlay::draw_weapon(player const& player) const
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

void screen_overlay::draw_compass(player const& player) const
{
    constexpr f64 COMPASS_NORTH_POS {0.375};
    constexpr i32 COMPASS_TOP_MARGIN {5};
    constexpr u32 COMPASS_MARKER_COLOR {0xFF0000FFu};

    auto const* compassTex {_cache.texture(compassTexture, 0)};
    if (!compassTex) { return; }

    auto const compassTexSize {_cache.texture_size(compassTexture, 0)};
    i32 const  texWidth {compassTexSize.Width};
    i32 const  texHeight {compassTexSize.Height};
    i32 const  compassVisibleWidth {texWidth / 4 * 3};

    i32 const scale {UIScale(_screenSize)};

    i32 const drawHeight {texHeight * scale};
    i32 const drawWidth {compassVisibleWidth * scale};

    i32 const drawX {(_screenSize.Width - drawWidth) / 2};
    i32 const drawY {COMPASS_TOP_MARGIN * scale};

    point_d const worldNorth {0.0, -1.0};
    f64 const     forwardDot {player.Direction.dot(worldNorth)};
    f64 const     rightDot {player.Direction.as_perpendicular().dot(worldNorth)};
    f64 const     relativeAngle {-std::atan2(rightDot, forwardDot)};

    i32 const center {static_cast<i32>(std::lround((COMPASS_NORTH_POS + (relativeAngle / TAU)) * texWidth))};

    i32 srcStart {(center - (compassVisibleWidth / 2)) % texWidth};
    if (srcStart < 0) { srcStart += texWidth; }

    i32 const firstSrcWidth {std::min(compassVisibleWidth, texWidth - srcStart)};
    i32 const secondSrcWidth {compassVisibleWidth - firstSrcWidth};

    i32 const firstDrawWidth {firstSrcWidth * scale};
    i32 const secondDrawWidth {drawWidth - firstDrawWidth};

    BlitMasked(_screen.data(), _screenSize,
               {.Tex      = compassTex,
                .TexPitch = texWidth,
                .Src      = {srcStart, 0, firstSrcWidth, texHeight},
                .Dst      = {drawX, drawY, firstDrawWidth, drawHeight}});

    if (secondSrcWidth > 0) {
        BlitMasked(_screen.data(), _screenSize,
                   {.Tex      = compassTex,
                    .TexPitch = texWidth,
                    .Src      = {0, 0, secondSrcWidth, texHeight},
                    .Dst      = {drawX + firstDrawWidth, drawY, secondDrawWidth, drawHeight}});
    }

    i32 const markerX {drawX + (drawWidth / 2)};
    i32 const markerY {drawY};
    i32 const markerHeight {drawY + drawHeight};
    i32 const markerWidth {(markerHeight / 2) | 1};
    for (i32 y {markerY}; y < markerHeight; ++y) {
        if (y == markerY || y == markerHeight - 1) {
            for (i32 x {0}; x < markerWidth; ++x) {
                _screen[PixelIndex(_screenSize.Width, markerX + x - (markerWidth / 2), y)] = COMPASS_MARKER_COLOR;
            }
        } else {
            _screen[PixelIndex(_screenSize.Width, markerX, y)] = COMPASS_MARKER_COLOR;
        }
    }
}

void screen_overlay::draw_message() const
{
    i32 const     scale {UIScale(_screenSize)};
    point_i const offset {5 * scale, 15 * scale};
    draw_text(_message, offset, 0, true);
}

void screen_overlay::draw_text(string_view text, point_i pos, i32 textScale, bool shadow) const
{
    if (text.empty()) { return; }

    auto* const tex {_cache.texture(FONT_TEXTURE_NORMAL, 0)};
    if (!tex) { return; }
    auto const texSize {_cache.texture_size(FONT_TEXTURE_NORMAL, 0)};
    assert(texSize.Height == FONT_GLYPH_SIZE);
    assert(texSize.Width == (FONT_FALLBACK_GLYPH + 1) * FONT_GLYPH_SIZE);

    i32 const scale {textScale > 0 ? textScale : UIScale(_screenSize)};
    i32 const advance {(FONT_GLYPH_SIZE + 1) * scale};

    // pass 0 = shadow, pass 1 = text, so a shadow never covers a neighbouring glyph
    for (i32 pass {shadow ? 0 : 1}; pass < 2; ++pass) {
        point_i   cursor {pos};
        i32 const offset {pass == 0 ? scale : 0};

        for (char const c : text) {
            if (c == '\n') {
                cursor = {pos.X, cursor.Y + advance};
                continue;
            }

            if (i32 const index {GlyphIndex(c)}; index >= 0) {
                BlitMasked(_screen.data(), _screenSize,
                           {.Tex      = tex,
                            .TexPitch = texSize.Width,
                            .Src      = {index * FONT_GLYPH_SIZE, 0, FONT_GLYPH_SIZE, FONT_GLYPH_SIZE},
                            .Dst      = {cursor.X + offset, cursor.Y + offset, FONT_GLYPH_SIZE * scale, FONT_GLYPH_SIZE * scale},
                            .Tint     = pass == 0 ? TEXT_SHADOW_COLOR : 0u});
            }
            cursor.X += advance;
        }
    }
}
