// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Cache.hpp"

#include "Common.hpp"

cache::cache(size_i screenSize)
{
    _screen.resize(screenSize.area());
}

auto cache::screen() -> u32*
{
    return _screen.data();
}

auto cache::texture(i32 idx, i32 facing) -> u8*
{
    texture_entry const& entry {_directory[idx][facing].Size.Width != 0
                                    ? _directory[idx][facing]
                                    : _directory[idx][0]};
    return _textures.data() + entry.Offset;
}

auto cache::texture_size(i32 idx, i32 facing) const -> size_i
{
    texture_entry const& entry {_directory[idx][facing].Size.Width != 0
                                    ? _directory[idx][facing]
                                    : _directory[idx][0]};
    return entry.Size;
}

void cache::load()
{
    struct pending_load {
        i32    Tex {0};
        string Path;
        size_i Size;
        i32    Facing {0};
    };

    std::vector<pending_load> const loads {
        {.Tex = 1, .Path = "res/wall0.png", .Size = wallSize},
        {.Tex = 2, .Path = "res/wall1.png", .Size = wallSize},
        {.Tex = 3, .Path = "res/wall2.png", .Size = wallSize},
        {.Tex = 4, .Path = "res/wall3.png", .Size = wallSize},
        {.Tex = 5, .Path = "res/wall4.png", .Size = wallSize},
        {.Tex = 6, .Path = "res/wall5.png", .Size = wallSize},
        {.Tex = 7, .Path = "res/wall6.png", .Size = wallSize},
        {.Tex = 8, .Path = "res/wall7.png", .Size = wallSize},
        {.Tex = door1Texture, .Path = "res/door.png", .Size = wallSize},
        {.Tex = door1FrameTexture, .Path = "res/door_frame.png", .Size = wallSize},
        {.Tex = 9, .Path = "res/floor.png", .Size = wallSize},
        {.Tex = 10, .Path = "res/ceiling.png", .Size = wallSize},
        {.Tex = 14, .Path = "res/sky.png", .Size = skySize},

        {.Tex = sprite1Texture, .Path = "res/enemy0-0.png", .Size = {64, 64}, .Facing = 0},
        {.Tex = sprite1Texture, .Path = "res/enemy0-1.png", .Size = {64, 64}, .Facing = 1},
        {.Tex = sprite1Texture, .Path = "res/enemy0-2.png", .Size = {64, 64}, .Facing = 2},
        {.Tex = sprite1Texture, .Path = "res/enemy0-3.png", .Size = {64, 64}, .Facing = 3},
        {.Tex = sprite1Texture, .Path = "res/enemy0-4.png", .Size = {64, 64}, .Facing = 4},
        {.Tex = sprite1Texture, .Path = "res/enemy0-5.png", .Size = {64, 64}, .Facing = 5},
        {.Tex = sprite1Texture, .Path = "res/enemy0-6.png", .Size = {64, 64}, .Facing = 6},
        {.Tex = sprite1Texture, .Path = "res/enemy0-7.png", .Size = {64, 64}, .Facing = 7},
    };

    usize totalBytes {0};
    for (auto const& l : loads) {
        _directory[l.Tex][l.Facing].Offset = totalBytes;
        _directory[l.Tex][l.Facing].Size   = l.Size;
        totalBytes += static_cast<usize>(l.Size.Width) * l.Size.Height * textureBPP;
    }
    _textures.resize(totalBytes);

    for (auto const& l : loads) {
        auto img {gfx::image::Load(l.Path).value()};
        img = gfx::filters::alpha_remover {}(img);

        u8* const   dst {texture(l.Tex, l.Facing)};
        isize const byteCount {static_cast<isize>(l.Size.Width) * l.Size.Height * textureBPP};
        for (isize idx {0}; idx < byteCount; ++idx) {
            dst[idx] = img.ptr()[idx];
        }
    }
}

void cache::copy(u32* dst, u8 const* src, i32 srcIdx, f64 darken)
{
    set(dst, get(src, srcIdx), darken);
}

void cache::set(u32* raw, color c, f64 darken)
{
    c.R  = static_cast<u8>(c.R * darken);
    c.G  = static_cast<u8>(c.G * darken);
    c.B  = static_cast<u8>(c.B * darken);
    *raw = std::byteswap(c.value());
}

auto cache::get(u8 const* img, usize idx) -> color
{
    u8 const r {img[idx + 0]};
    u8 const g {img[idx + 1]};
    u8 const b {img[idx + 2]};
    return {r, g, b, 255};
}
