// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Cache.hpp"

cache::cache(size_i screenSize)
{
    _screen.resize(screenSize.area());
}

auto cache::screen() -> u32*
{
    return _screen.data();
}

auto cache::texture(i32 idx) -> u8*
{
    return _textures.data() + _directory[idx].Offset;
}

auto cache::texture_size(i32 idx) const -> size_i
{
    return _directory[idx].Size;
}

void cache::load()
{
    struct pending_load {
        i32    Tex {};
        string Path;
        size_i Size;
    };

    std::vector<pending_load> const loads {
        {.Tex = 0, .Path = "res/wall0.png", .Size = wallSize},
        {.Tex = 1, .Path = "res/wall1.png", .Size = wallSize},
        {.Tex = 2, .Path = "res/wall2.png", .Size = wallSize},
        {.Tex = 3, .Path = "res/wall3.png", .Size = wallSize},
        {.Tex = 4, .Path = "res/wall4.png", .Size = wallSize},
        {.Tex = 5, .Path = "res/wall5.png", .Size = wallSize},
        {.Tex = 6, .Path = "res/wall6.png", .Size = wallSize},
        {.Tex = 7, .Path = "res/wall7.png", .Size = wallSize},
        {.Tex = floorTexture, .Path = "res/floor.png", .Size = wallSize},
        {.Tex = ceilingTexture, .Path = "res/ceiling.png", .Size = wallSize},
        {.Tex = sprite1Texture, .Path = "res/adventurer_idle.png", .Size = {128, 128}},
    };

    usize totalBytes {0};
    for (auto const& l : loads) {
        _directory[l.Tex].Offset = totalBytes;
        _directory[l.Tex].Size   = l.Size;
        totalBytes += static_cast<usize>(l.Size.Width) * l.Size.Height * textureBPP;
    }
    _textures.resize(totalBytes);

    for (auto const& l : loads) {
        auto img {gfx::image::Load(l.Path).value()};

        gfx::filters::color_changer magentafy;
        magentafy.From = colors::Transparent;
        magentafy.To   = colors::Magenta;
        img            = gfx::filters::alpha_remover {}(magentafy(img));

        u8* const   dst {texture(l.Tex)};
        isize const byteCount {static_cast<isize>(l.Size.Width) * l.Size.Height * textureBPP};
        for (isize idx {0}; idx < byteCount; ++idx) {
            dst[idx] = img.ptr()[idx];
        }
    }
}

void cache::copy(u32* dst, u8 const* src, i32 srcIdx, bool darken)
{
    set(dst, get(src, srcIdx), darken);
}

void cache::set(u32* raw, color c, bool darken)
{
    if (darken) {
        c.R /= 2;
        c.G /= 2;
        c.B /= 2;
    }
    *raw = std::byteswap(c.value());
}

auto cache::get(u8 const* img, usize idx) -> color
{
    u8 const r {img[idx + 0]};
    u8 const g {img[idx + 1]};
    u8 const b {img[idx + 2]};
    return {r, g, b, 255};
}
