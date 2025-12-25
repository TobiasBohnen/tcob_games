// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "EngineHelper.hpp"

////////////////////////////////////////////////////////////

dmd_proxy::dmd_proxy(prop<grid<u8>>& dmd)
    : _dmd {dmd}
{
}
void dmd_proxy::clear()
{
    _dmd = grid<u8> {DMD_SIZE, 0};
}
void dmd_proxy::line(point_i start, point_i end, u8 color)
{
    _dmd.mutate([&](auto& dmd) {
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
            if (x0 >= 0 && x0 < dmd.width() && y0 >= 0 && y0 < dmd.height()) {
                dmd[x0, y0] = color;
            }

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
    });
}
void dmd_proxy::circle(point_i center, i32 radius, u8 color, bool fill)
{
    _dmd.mutate([&](auto& dmd) {
        i32 x {radius};
        i32 y {0};
        i32 err {0};

        auto const plot {[&](i32 const px, i32 const py) {
            if (px >= 0 && px < dmd.width() && py >= 0 && py < dmd.height()) {
                dmd[px, py] = color;
            }
        }};

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
    });
}
void dmd_proxy::rect(rect_i const& rect, u8 color, bool fill)
{
    _dmd.mutate([&](auto& dmd) {
        i32 const x0 {rect.left()};
        i32 const y0 {rect.top()};
        i32 const x1 {rect.right()};
        i32 const y1 {rect.bottom()};

        for (i32 y {y0}; y < y1; ++y) {
            for (i32 x {x0}; x < x1; ++x) {
                if (x < 0 || x >= dmd.width() || y < 0 || y >= dmd.height()) { continue; }

                bool const isEdge {x == x0 || x == x1 - 1 || y == y0 || y == y1 - 1};
                if (fill || isEdge) {
                    dmd[x, y] = color;
                }
            }
        }
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

////////////////////////////////////////////////////////////

auto sfx_proxy::pickup_coin(u64 seed) -> audio::sound_wave { return audio::sound_generator {}.generate_pickup_coin(seed); }
auto sfx_proxy::laser_shoot(u64 seed) -> audio::sound_wave { return audio::sound_generator {}.generate_laser_shoot(seed); }
auto sfx_proxy::explosion(u64 seed) -> audio::sound_wave { return audio::sound_generator {}.generate_explosion(seed); }
auto sfx_proxy::powerup(u64 seed) -> audio::sound_wave { return audio::sound_generator {}.generate_powerup(seed); }
auto sfx_proxy::hit_hurt(u64 seed) -> audio::sound_wave { return audio::sound_generator {}.generate_hit_hurt(seed); }
auto sfx_proxy::jump(u64 seed) -> audio::sound_wave { return audio::sound_generator {}.generate_jump(seed); }
auto sfx_proxy::blip_select(u64 seed) -> audio::sound_wave { return audio::sound_generator {}.generate_blip_select(seed); }
auto sfx_proxy::random(u64 seed) -> audio::sound_wave { return audio::sound_generator {}.generate_random(seed); }

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
