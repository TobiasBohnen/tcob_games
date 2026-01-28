// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "TexProxy.hpp"

#include "Fonts.hpp"
#include "Socket.hpp"

void draw_line(point_i start, point_i end, auto&& plot)
{
    i32       x0 {start.X};
    i32       y0 {start.Y};
    i32 const x1 {end.X};
    i32 const y1 {end.Y};

    i32 const dx {std::abs(x1 - x0)};
    i32 const sx {x0 < x1 ? 1 : -1};
    i32 const dy {-std::abs(y1 - y0)};
    i32 const sy {y0 < y1 ? 1 : -1};
    i32       err {dx + dy};

    for (;;) {
        plot(x0, y0);

        i32 const e2 {2 * err};
        if (e2 >= dy) {
            if (x0 == x1) { break; }
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            if (y0 == y1) { break; }
            err += dx;
            y0 += sy;
        }
    }
}

void draw_circle(point_i center, i32 radius, bool fill, auto&& plot)
{
    if (radius == 1) {
        plot(center.X, center.Y);
        return;
    }

    i32 x {radius};
    i32 y {0};
    i32 err {0};

    while (x >= y) {
        if (fill) {
            for (i32 i {center.X - x}; i <= center.X + x; ++i) {
                plot(i, center.Y + y);
                plot(i, center.Y - y);
            }
            for (i32 i {center.X - y}; i <= center.X + y; ++i) {
                plot(i, center.Y + x);
                plot(i, center.Y - x);
            }
        } else {
            plot(center.X + x, center.Y + y);
            plot(center.X + y, center.Y + x);
            plot(center.X - y, center.Y + x);
            plot(center.X - x, center.Y + y);
            plot(center.X - x, center.Y - y);
            plot(center.X - y, center.Y - x);
            plot(center.X + y, center.Y - x);
            plot(center.X + x, center.Y - y);
        }

        ++y;
        if (err <= 0) {
            err += (2 * y) + 1;
        } else {
            --x;
            err += (2 * (y - x)) + 1;
        }
    }
}

void draw_rect(rect_i const& rect, bool fill, auto&& plot)
{
    i32 const x0 {rect.left()};
    i32 const y0 {rect.top()};
    i32 const x1 {rect.right()};
    i32 const y1 {rect.bottom()};

    for (i32 y {y0}; y < y1; ++y) {
        for (i32 x {x0}; x < x1; ++x) {
            bool const isEdge {x == x0 || x == x1 - 1 || y == y0 || y == y1 - 1};
            if (fill || isEdge) {
                plot(x, y);
            }
        }
    }
}

void draw_print(point_i pos, string_view text, font_type type, auto&& plot)
{
    auto const font {get_font(type)};
    i32 const  stride {font.Size.Width + 1};

    for (usize i {0}; i < text.size(); ++i) {
        char c {text[i]};
        i32  index {49};
        if (c >= '0' && c <= '9') {
            index = c - '0';
        } else if (c >= 'A' && c <= 'Z') {
            index = 10 + (c - 'A');
        } else if (c >= 'a' && c <= 'z') {
            index = 10 + (c - 'a');
        } else if (c >= '*' && c <= '/') {
            index = 36 + (c - '*');
        } else if (c >= ':' && c <= '?') {
            index = 42 + (c - ':');
        } else if (c == '!') {
            index = 48;
        } else if (c == ' ') {
            continue;
        }

        for (i32 y {0}; y < font.Size.Height; ++y) {
            u8 const row {font.Ptr[(index * font.Size.Height) + y]};
            for (i32 x {0}; x < font.Size.Width; ++x) {
                if ((row & (1 << (font.Size.Width - 1 - x))) == 0) { continue; }
                point_i const p {static_cast<i32>(pos.X + (i * stride) + x), pos.Y + y};
                plot(p.X, p.Y);
            }
        }
    }
}

void draw_socket(socket* socket, auto&& line, auto&& rect)
{
    if (!socket->is_empty()) { return; }

    std::unordered_map<socket_state, u8> borderColors {
        {socket_state::Accept, 0xA},
        {socket_state::Hover, 0x1},
        {socket_state::Idle, 0x1},
        {socket_state::Reject, 0x3},
    };

    point_i pos {socket->HUDPosition};

    rect_i box {pos, size_i {16, 16}};

    u8 const  color {borderColors[socket->state()]};
    i32 const len {3};

    i32 const x0 {box.Position.X};
    i32 const y0 {box.Position.Y};
    i32 const x1 {box.Position.X + box.Size.Width - 1};
    i32 const y1 {box.Position.Y + box.Size.Height - 1};

    line({x0, y0}, {x0 + len, y0}, color);
    line({x0, y0}, {x0, y0 + len}, color);
    line({x1, y0}, {x1 - len, y0}, color);
    line({x1, y0}, {x1, y0 + len}, color);
    line({x0, y1}, {x0 + len, y1}, color);
    line({x0, y1}, {x0, y1 - len}, color);
    line({x1, y1}, {x1 - len, y1}, color);
    line({x1, y1}, {x1, y1 - len}, color);

    box = box.as_inset_by(point_i {1, 1});

    auto const& colorsOpt {socket->face().Colors};
    if (!colorsOpt) {
        rect(box, 0x2, true);
    } else {
        std::vector<u8> colors(colorsOpt->begin(), colorsOpt->end());
        auto const      numColors {colors.size()};

        if (numColors == 2) {
            rect(box, colors[0], true);
            for (i32 i {0}; i < box.Size.Height; ++i) {
                point_i const start {box.Position.X + box.Size.Width - i, box.Position.Y + i};
                point_i const end {box.Position.X + box.Size.Width - 1, box.Position.Y + i};
                if (start.X < box.Position.X + box.Size.Width) {
                    line(start, end, colors[1]);
                }
            }
        } else {
            i32 currentX {box.Position.X};
            for (usize i {0}; i < numColors; ++i) {
                i32 const nextX {box.Position.X + static_cast<i32>((i + 1) * box.Size.Width / numColors)};
                rect({{currentX, box.Position.Y}, {nextX - currentX, box.Size.Height}}, colors[i], true);
                currentX = nextX;
            }
        }
    }
}

////////////////////////////////////////////////////////////

tex_proxy::tex_proxy(prop<gfx::image>& img, color clear)
    : _img {img}
    , _imgSize {_img->info().Size}
    , _clear {clear}
{
    this->clear(std::nullopt);
}

auto tex_proxy::bounds() const -> rect_i
{
    return {point_i::Zero, _imgSize};
}

void tex_proxy::clear(std::optional<rect_i> const& rect)
{
    _img.mutate([&](auto& img) {
        img.fill(rect ? *rect : rect_i {point_i::Zero, img.info().Size}, _clear);
    });
}

void tex_proxy::pixel(point_i pos, u8 color)
{
    _img.mutate([&](auto& img) {
        if (img.info().Size.contains(pos)) {
            img.set_pixel(pos, PALETTE[color]);
        }
    });
}

void tex_proxy::draw(std::span<u8> data, i32 x, i32 y, size_i s, color col)
{
    if (x >= 0 && x < s.Width && y >= 0 && y < s.Height) {
        isize const dstIdx {(x + (y * s.Width)) * 4};
        data[dstIdx + 0] = col.R;
        data[dstIdx + 1] = col.G;
        data[dstIdx + 2] = col.B;
        data[dstIdx + 3] = col.A;
    }
}

void tex_proxy::line(point_i start, point_i end, u8 color)
{
    auto const c {PALETTE[color]};
    _img.mutate([&](auto& img) {
        auto       pixels {img.data()};
        auto const size {img.info().Size};
        draw_line(start, end, [&](i32 x, i32 y) {
            draw(pixels, x, y, size, c);
        });
    });
}

void tex_proxy::circle(point_i center, i32 radius, u8 color, bool fill)
{
    auto const c {PALETTE[color]};
    _img.mutate([&](auto& img) {
        auto       pixels {img.data()};
        auto const size {img.info().Size};
        draw_circle(center, radius, fill, [&](i32 x, i32 y) {
            draw(pixels, x, y, size, c);
        });
    });
}

void tex_proxy::rect(rect_i const& rect, u8 color, bool fill)
{
    auto const c {PALETTE[color]};
    _img.mutate([&](auto& img) {
        auto       pixels {img.data()};
        auto const size {img.info().Size};
        draw_rect(rect, fill, [&](i32 x, i32 y) {
            draw(pixels, x, y, size, c);
        });
    });
}

void tex_proxy::blit(rect_i const& rect, string const& data, blit_settings settings)
{
    auto const dots {decode_texture_pixels(data, rect.Size)};

    i32 const imgWidth {rect.Size.Width};
    i32 const imgHeight {rect.Size.Height};

    f32 const scale {settings.Scale <= 0.0f ? 1.0f : settings.Scale};
    i32 const newWidth {static_cast<i32>(std::floor((imgWidth * scale) + 0.5f))};
    i32 const newHeight {static_cast<i32>(std::floor((imgHeight * scale) + 0.5f))};
    i32 const sr {newWidth - 1};
    i32 const sb {newHeight - 1};

    f64 const xFactor {static_cast<f64>(imgWidth) / newWidth};
    f64 const yFactor {static_cast<f64>(imgHeight) / newHeight};

    if (newWidth <= 0 || newHeight <= 0) { return; }

    auto const map_dst {[&](i32 sx, i32 sy) -> point_i {
        i32 const fx {settings.FlipH ? sr - sx : sx};
        i32 const fy {settings.FlipV ? sb - sy : sy};
        switch (settings.Rotation) {
        case 90:  return {sb - fy, fx};
        case 180: return {sr - fx, sb - fy};
        case 270: return {fy, sr - fx};
        default:  return {fx, fy};
        }
    }};

    _img.mutate([&](auto& img) {
        auto pixels {img.data()};
        for (i32 y {0}; y < newHeight; ++y) {
            for (i32 x {0}; x < newWidth; ++x) {
                i32 const sx {static_cast<i32>(std::round(x * xFactor))};
                i32 const sy {static_cast<i32>(std::round(y * yFactor))};
                i32 const srcIdx {sx + (sy * imgWidth)};

                assert(srcIdx >= 0 && srcIdx < std::ssize(dots));
                u8 palIndex {dots[srcIdx]};
                if (settings.Swap && settings.Swap->contains(palIndex)) {
                    palIndex = (*settings.Swap)[palIndex];
                }
                color const   col {settings.Transparent == palIndex ? colors::Transparent : PALETTE[palIndex]};
                point_i const dst {rect.top_left() + map_dst(x, y)};
                draw(pixels, dst.X, dst.Y, _imgSize, col);
            }
        }
    });
}

void tex_proxy::print(point_i pos, string_view text, u8 color, font_type type)
{
    auto const c {PALETTE[color]};
    _img.mutate([&](auto& img) {
        auto       pixels {img.data()};
        auto const size {img.info().Size};
        draw_print(pos, text, type, [&](i32 x, i32 y) {
            draw(pixels, x, y, size, c);
        });
    });
}

void tex_proxy::socket(class socket* socket)
{
    draw_socket(
        socket,
        [&](point_i start, point_i end, u8 color) { line(start, end, color); },
        [&](rect_i const& r, u8 color, bool fill) { rect(r, color, fill); });
}

////////////////////////////////////////////////////////////

auto decode_texture_pixels(string_view s, size_i size) -> std::vector<u8>
{
    static auto from_base26 {[](string_view sv) -> u32 {
        u32 n {0};
        for (char c : sv) {
            n = (n * 26) + (c - 'a' + 1);
        }
        return n;
    }};

    std::vector<u8> dots;
    dots.reserve(size.area());

    bool const isRLE {!s.empty() && s[0] == '!'};
    if (isRLE) {
        usize i {1};
        while (i < s.size()) {
            char digitChar {s[i++]};
            u8   val {0};
            if (digitChar >= '0' && digitChar <= '9') {
                val = digitChar - '0';
            } else if (digitChar >= 'A' && digitChar <= 'F') {
                val = 10 + (digitChar - 'A');
            }

            if (i < s.size() && s[i] >= 'a' && s[i] <= 'z') {
                usize start {i};
                while (i < s.size() && s[i] >= 'a' && s[i] <= 'z') { ++i; }
                u32 const run {from_base26(std::string_view(s.data() + start, i - start))};
                dots.insert(dots.end(), run, val);
            } else {
                dots.push_back(val);
            }
        }
    } else {
        for (char c : s) {
            if (c >= '0' && c <= '9') {
                dots.push_back(static_cast<u8>(c - '0'));
            } else if (c >= 'A' && c <= 'F') {
                dots.push_back(static_cast<u8>(10 + (c - 'A')));
            }
        }
    }

    return dots;
}
