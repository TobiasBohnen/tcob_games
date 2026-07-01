// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "TextureCache.hpp"

#include "Common.hpp"

auto texture_cache::get_entry(i32 idx, i32 facing) const -> texture_entry const&
{
    auto const& facings {_directory.at(idx)};
    auto const  it {facings.find(facing)};
    return it != facings.end() ? it->second : facings.at(0);
}

auto texture_cache::texture(i32 idx, i32 facing) -> u8*
{
    return _textures.data() + get_entry(idx, facing).Offset;
}

auto texture_cache::texture_size(i32 idx, i32 facing) const -> size_i
{
    return get_entry(idx, facing).Size;
}

void texture_cache::load()
{
    struct pending_load {
        i32    Tex {0};
        string Path;
        i32    Facing {0};
    };

    std::vector<pending_load> const loads {
        {.Tex = 1, .Path = "res/wall0.png"},
        {.Tex = 2, .Path = "res/wall1.png"},
        {.Tex = 3, .Path = "res/wall2.png"},
        {.Tex = 4, .Path = "res/wall3.png"},
        {.Tex = 5, .Path = "res/wall4.png"},
        {.Tex = 6, .Path = "res/wall5.png"},
        {.Tex = 7, .Path = "res/wall6.png"},
        {.Tex = 8, .Path = "res/wall7.png"},
        {.Tex = door1Texture, .Path = "res/door.png"},
        {.Tex = door1FrameTexture, .Path = "res/door_frame.png"},
        {.Tex = 9, .Path = "res/floor.png"},
        {.Tex = 10, .Path = "res/ceiling.png"},
        {.Tex = 14, .Path = "res/sky.png"},
        {.Tex = 15, .Path = "res/transparent.png"},

        {.Tex = sprite1Texture, .Path = "res/enemy0-0.png", .Facing = 0},
        {.Tex = sprite1Texture, .Path = "res/enemy0-1.png", .Facing = 1},
        {.Tex = sprite1Texture, .Path = "res/enemy0-2.png", .Facing = 2},
        {.Tex = sprite1Texture, .Path = "res/enemy0-3.png", .Facing = 3},
        {.Tex = sprite1Texture, .Path = "res/enemy0-4.png", .Facing = 4},
        {.Tex = sprite1Texture, .Path = "res/enemy0-5.png", .Facing = 5},
        {.Tex = sprite1Texture, .Path = "res/enemy0-6.png", .Facing = 6},
        {.Tex = sprite1Texture, .Path = "res/enemy0-7.png", .Facing = 7},
    };

    usize totalBytes {0};
    for (auto const& l : loads) {
        _directory[l.Tex][l.Facing].Offset = totalBytes;
        auto const size {gfx::image::LoadInfo(l.Path)->Size};
        _directory[l.Tex][l.Facing].Size = size;
        totalBytes += size.area() * TEXTURE_BPP;
    }
    _textures.resize(totalBytes);

    for (auto const& l : loads) {
        auto img {gfx::image::Load(l.Path).value()};
        img = gfx::filters::alpha_remover {}(img);

        u8* const   dst {texture(l.Tex, l.Facing)};
        isize const byteCount {img.info().Size.area() * TEXTURE_BPP};
        for (isize idx {0}; idx < byteCount; ++idx) {
            dst[idx] = img.ptr()[idx];
        }
    }
}
