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
        size_i Size;
        i32    Facing {0};
    };

    std::vector<pending_load> const loads {
        {.Tex = 1, .Path = "res/wall0.png", .Size = WALL_SIZE},
        {.Tex = 2, .Path = "res/wall1.png", .Size = WALL_SIZE},
        {.Tex = 3, .Path = "res/wall2.png", .Size = WALL_SIZE},
        {.Tex = 4, .Path = "res/wall3.png", .Size = WALL_SIZE},
        {.Tex = 5, .Path = "res/wall4.png", .Size = WALL_SIZE},
        {.Tex = 6, .Path = "res/wall5.png", .Size = WALL_SIZE},
        {.Tex = 7, .Path = "res/wall6.png", .Size = WALL_SIZE},
        {.Tex = 8, .Path = "res/wall7.png", .Size = WALL_SIZE},
        {.Tex = door1Texture, .Path = "res/door.png", .Size = WALL_SIZE},
        {.Tex = door1FrameTexture, .Path = "res/door_frame.png", .Size = WALL_SIZE},
        {.Tex = 9, .Path = "res/floor.png", .Size = WALL_SIZE},
        {.Tex = 10, .Path = "res/ceiling.png", .Size = WALL_SIZE},
        {.Tex = 14, .Path = "res/sky.png", .Size = SKY_SIZE},
        {.Tex = 15, .Path = "res/transparent.png", .Size = WALL_SIZE},

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
        totalBytes += l.Size.area() * TEXTURE_BPP;
    }
    _textures.resize(totalBytes);

    for (auto const& l : loads) {
        auto img {gfx::image::Load(l.Path).value()};
        img = gfx::filters::alpha_remover {}(img);

        u8* const   dst {texture(l.Tex, l.Facing)};
        isize const byteCount {l.Size.area() * TEXTURE_BPP};
        for (isize idx {0}; idx < byteCount; ++idx) {
            dst[idx] = img.ptr()[idx];
        }
    }
}
