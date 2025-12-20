// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "EngineHelper.hpp"
#include "Painter.hpp"

////////////////////////////////////////////////////////////

dmd_proxy::dmd_proxy(prop<grid<u8>>& dmd)
    : _dmd {dmd}
{
}
void dmd_proxy::clear()
{
    _dmd = grid<u8> {DMD_SIZE, 0};
}
void dmd_proxy::blit(rect_i const& rect, string const& dotStr)
{
    if (rect.right() > _dmd->width() || rect.bottom() > _dmd->height()) { return; }
    auto const dots {get_pixel(dotStr, rect.Size)};

    _dmd.mutate([&](auto& dmd) { dmd.blit(rect, dots); });
}
void dmd_proxy::print(point_i pos, string_view text, color color)
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

    i32 colorIdx {-1};
    for (i32 i {0}; i < PALETTE.size(); ++i) {
        if (PALETTE[i] == color) {
            colorIdx = i;
            break;
        }
    }
    if (colorIdx == -1) { return; }

    _dmd.mutate([&](auto& dmd) {
        for (i32 i {0}; i < text.size(); ++i) {
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
            }

            for (i32 y {0}; y < 5; ++y) {
                u8 const row {font5x5[index][y]};
                for (i32 x {0}; x < 5; ++x) {
                    if ((row & (1 << (4 - x))) == 0) { continue; }
                    point_i const p {pos.X + (i * 6) + x, pos.Y + (y)};
                    if (p.X < DMD_SIZE.Width && p.Y < DMD_SIZE.Height) {
                        dmd[p.X, p.Y] = static_cast<u8>(colorIdx);
                    }
                }
            }
        }
    });
}

////////////////////////////////////////////////////////////

auto sfx_proxy::pickup_coin(u64 seed) -> audio::sound_wave { return audio::sound_generator {random::prng_split_mix_64 {seed}}.generate_pickup_coin(); }
auto sfx_proxy::laser_shoot(u64 seed) -> audio::sound_wave { return audio::sound_generator {random::prng_split_mix_64 {seed}}.generate_laser_shoot(); }
auto sfx_proxy::explosion(u64 seed) -> audio::sound_wave { return audio::sound_generator {random::prng_split_mix_64 {seed}}.generate_explosion(); }
auto sfx_proxy::powerup(u64 seed) -> audio::sound_wave { return audio::sound_generator {random::prng_split_mix_64 {seed}}.generate_powerup(); }
auto sfx_proxy::hit_hurt(u64 seed) -> audio::sound_wave { return audio::sound_generator {random::prng_split_mix_64 {seed}}.generate_hit_hurt(); }
auto sfx_proxy::jump(u64 seed) -> audio::sound_wave { return audio::sound_generator {random::prng_split_mix_64 {seed}}.generate_jump(); }
auto sfx_proxy::blip_select(u64 seed) -> audio::sound_wave { return audio::sound_generator {random::prng_split_mix_64 {seed}}.generate_blip_select(); }
auto sfx_proxy::random(u64 seed) -> audio::sound_wave { return audio::sound_generator {random::prng_split_mix_64 {seed}}.generate_random(); }
