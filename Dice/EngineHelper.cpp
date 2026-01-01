// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "EngineHelper.hpp"

#include "Socket.hpp"

template <typename PlotFunc>
void draw_line(point_i start, point_i end, PlotFunc plot)
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

template <typename PlotFunc>
void draw_circle(point_i center, i32 radius, bool fill, PlotFunc plot)
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

template <typename PlotFunc>
void draw_rect(rect_i const& rect, bool fill, PlotFunc plot)
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

////////////////////////////////////////////////////////////

dmd_proxy::dmd_proxy(prop<grid<u8>>& dmd)
    : _dmd {dmd}
{
}

auto dmd_proxy::size() const -> size_i
{
    return DMD_SIZE;
}

void dmd_proxy::clear()
{
    _dmd = grid<u8> {DMD_SIZE, 0};
}

void dmd_proxy::line(point_i start, point_i end, u8 color)
{
    _dmd.mutate([&](auto& dmd) {
        draw_line(start, end, [&](i32 x, i32 y) {
            if (x >= 0 && x < dmd.width() && y >= 0 && y < dmd.height()) {
                dmd[x, y] = color;
            }
        });
    });
}

void dmd_proxy::circle(point_i center, i32 radius, u8 color, bool fill)
{
    _dmd.mutate([&](auto& dmd) {
        draw_circle(center, radius, fill, [&](i32 x, i32 y) {
            if (x >= 0 && x < dmd.width() && y >= 0 && y < dmd.height()) {
                dmd[x, y] = color;
            }
        });
    });
}

void dmd_proxy::rect(rect_i const& rect, u8 color, bool fill)
{
    _dmd.mutate([&](auto& dmd) {
        draw_rect(rect, fill, [&](i32 x, i32 y) {
            if (x >= 0 && x < dmd.width() && y >= 0 && y < dmd.height()) {
                dmd[x, y] = color;
            }
        });
    });
}

void dmd_proxy::blit(rect_i const& rect, string const& dotStr)
{
    if (rect.right() > _dmd->width() || rect.bottom() > _dmd->height()) { return; }
    auto const dots {decode_texture_pixels(dotStr, rect.Size)};

    _dmd.mutate([&](auto& dmd) { dmd.blit(rect, dots); });
}

void dmd_proxy::print(point_i pos, string_view text, u8 color)
{
    static constexpr std::array<std::array<u8, 5>, 50> font5x5 {
        {{0x1F, 0x13, 0x15, 0x19, 0x1F},
         {0x06, 0x0A, 0x12, 0x02, 0x1F},
         {0x1F, 0x01, 0x1F, 0x10, 0x1F},
         {0x1F, 0x01, 0x1F, 0x01, 0x1F},
         {0x12, 0x12, 0x1F, 0x02, 0x02},
         {0x1F, 0x10, 0x1F, 0x01, 0x1F},
         {0x1F, 0x10, 0x1F, 0x11, 0x1F},
         {0x1F, 0x01, 0x01, 0x01, 0x01},
         {0x1F, 0x11, 0x1F, 0x11, 0x1F},
         {0x1F, 0x11, 0x1F, 0x01, 0x01},
         {0x0E, 0x11, 0x1F, 0x11, 0x11},
         {0x1E, 0x11, 0x1E, 0x11, 0x1E},
         {0x0F, 0x10, 0x10, 0x10, 0x0F},
         {0x1E, 0x11, 0x11, 0x11, 0x1E},
         {0x1F, 0x10, 0x1E, 0x10, 0x1F},
         {0x1F, 0x10, 0x1F, 0x10, 0x10},
         {0x0E, 0x10, 0x17, 0x11, 0x0E},
         {0x11, 0x11, 0x1F, 0x11, 0x11},
         {0x1F, 0x04, 0x04, 0x04, 0x1F},
         {0x0F, 0x01, 0x01, 0x11, 0x0E},
         {0x11, 0x12, 0x1C, 0x12, 0x11},
         {0x10, 0x10, 0x10, 0x10, 0x1F},
         {0x11, 0x1B, 0x15, 0x11, 0x11},
         {0x11, 0x19, 0x15, 0x13, 0x11},
         {0x0E, 0x11, 0x11, 0x11, 0x0E},
         {0x1E, 0x11, 0x1E, 0x10, 0x10},
         {0x0E, 0x11, 0x15, 0x12, 0x0D},
         {0x1E, 0x11, 0x1F, 0x12, 0x11},
         {0x0F, 0x10, 0x0E, 0x01, 0x1E},
         {0x1F, 0x04, 0x04, 0x04, 0x04},
         {0x11, 0x11, 0x11, 0x11, 0x0E},
         {0x11, 0x11, 0x11, 0x0A, 0x04},
         {0x11, 0x11, 0x15, 0x1B, 0x11},
         {0x11, 0x0A, 0x04, 0x0A, 0x11},
         {0x11, 0x11, 0x0E, 0x04, 0x04},
         {0x1F, 0x02, 0x04, 0x08, 0x1F},
         {0x00, 0x0A, 0x04, 0x0A, 0x00},
         {0x00, 0x04, 0x0E, 0x04, 0x00},
         {0x00, 0x00, 0x04, 0x04, 0x08},
         {0x00, 0x00, 0x0E, 0x00, 0x00},
         {0x00, 0x00, 0x00, 0x00, 0x08},
         {0x00, 0x02, 0x04, 0x08, 0x00},
         {0x00, 0x04, 0x00, 0x04, 0x00},
         {0x00, 0x04, 0x00, 0x04, 0x04},
         {0x02, 0x04, 0x08, 0x04, 0x02},
         {0x00, 0x0E, 0x00, 0x0E, 0x00},
         {0x08, 0x04, 0x02, 0x04, 0x08},
         {0x0E, 0x11, 0x06, 0x00, 0x04},
         {0x04, 0x04, 0x04, 0x00, 0x04},
         {0x04, 0x0A, 0x15, 0x0A, 0x04}}};

    _dmd.mutate([&](auto& dmd) {
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

            for (i32 y {0}; y < 5; ++y) {
                u8 const row {font5x5[index][y]};
                for (i32 x {0}; x < 5; ++x) {
                    if ((row & (1 << (4 - x))) == 0) { continue; }
                    point_i const p {static_cast<i32>(pos.X + (i * 6) + x), pos.Y + (y)};
                    if (p.X < DMD_SIZE.Width && p.Y < DMD_SIZE.Height) {
                        dmd[p.X, p.Y] = color;
                    }
                }
            }
        }
    });
}

void dmd_proxy::draw_socket(socket* socket, rect_f const& dmdBounds)
{
    std::unordered_map<socket_state, u8> borderColors {
        {socket_state::Accept, 0xA},
        {socket_state::Hover, 0x1},
        {socket_state::Idle, 0x1},
        {socket_state::Reject, 0x3},
    };

    point_f socketPos {socket->bounds().Position};
    point_i pos {static_cast<i32>(std::round((socketPos.X - dmdBounds.left()) / (dmdBounds.width() / DMD_SIZE.Width))),
                 static_cast<i32>(std::round((socketPos.Y - dmdBounds.top()) / (dmdBounds.height() / DMD_SIZE.Height)))};

    rect_i box {pos, size_i {13, 13}};
    box = box.as_inset_by(point_i {-1, -1});

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
    , _clear {clear}
{
    this->clear(std::nullopt);
}

auto tex_proxy::bounds() const -> rect_i { return {point_i::Zero, _img->info().Size}; }

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

void tex_proxy::line(point_i start, point_i end, u8 c)
{
    color const col {PALETTE[c]};
    _img.mutate([&](auto& img) {
        auto       pixels {img.data()};
        auto const size {img.info().Size};
        draw_line(start, end, [&](i32 x, i32 y) {
            draw(pixels, x, y, size, col);
        });
    });
}

void tex_proxy::circle(point_i center, i32 radius, u8 c, bool fill)
{
    color const col {PALETTE[c]};
    _img.mutate([&](auto& img) {
        auto       pixels {img.data()};
        auto const size {img.info().Size};
        draw_circle(center, radius, fill, [&](i32 x, i32 y) {
            draw(pixels, x, y, size, col);
        });
    });
}

void tex_proxy::rect(rect_i const& rect, u8 c, bool fill)
{
    color const col {PALETTE[c]};
    _img.mutate([&](auto& img) {
        auto       pixels {img.data()};
        auto const size {img.info().Size};
        draw_rect(rect, fill, [&](i32 x, i32 y) {
            draw(pixels, x, y, size, col);
        });
    });
}

void tex_proxy::blit(rect_i const& rect, string const& data, blit_settings settings)
{
    if (rect.right() > _img->info().Size.Width || rect.bottom() > _img->info().Size.Height) { return; }
    auto const dots {decode_texture_pixels(data, rect.Size)};

    u32 const  rotation {settings.Rotation.value_or(0)};
    bool const flip_h {settings.FlipH.value_or(false)};
    bool const flip_v {settings.FlipV.value_or(false)};

    i32 const w {rect.Size.Width};
    i32 const h {rect.Size.Height};

    auto const map_xy {[&](i32 x, i32 y) -> point_i {
        i32 const r {w - 1};
        i32 const b {h - 1};
        i32 const fx {flip_h ? r - x : x};
        i32 const fy {flip_v ? h - 1 - y : y};
        switch (rotation) {
        case 90:  return {b - fy, fx};
        case 180: return {r - fx, b - fy};
        case 270: return {fy, r - fx};
        default:  return {fx, fy};
        }
    }};

    _img.mutate([&](auto& img) {
        auto const size {_img->info().Size};
        auto       pixels {img.data()};
        for (i32 y {0}; y < h; ++y) {
            for (i32 x {0}; x < w; ++x) {
                point_i const dst {rect.top_left() + map_xy(x, y)};
                u8 const      srcIdx {dots[x + (y * w)]};
                color const   col {settings.Transparent == srcIdx ? colors::Transparent : PALETTE[srcIdx]};
                draw(pixels, dst.X, dst.Y, size, col);
            }
        }
    });
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

    auto const isRLE {[](string_view sv) -> bool {
        return sv.size() > 1 && (sv[1] >= 'a' && sv[1] <= 'z');
    }};

    if (isRLE(s)) {
        usize i {0};
        while (i < s.size()) {
            char digitChar {s[i++]};
            u8   val {0};
            if (digitChar >= '0' && digitChar <= '9') {
                val = digitChar - '0';
            } else if (digitChar >= 'A' && digitChar <= 'F') {
                val = 10 + (digitChar - 'A');
            }

            usize start {i};
            while (i < s.size() && s[i] >= 'a' && s[i] <= 'z') { ++i; }

            u32 const run {from_base26(std::string_view(s.data() + start, i - start))};

            dots.insert(dots.end(), run, val);
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
