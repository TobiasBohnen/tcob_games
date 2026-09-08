// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "TextureCache.hpp"

#include "Common.hpp"
auto texture_cache::get_entry(i32 idx, i32 variant) const -> texture_entry const&
{
    auto const& variants {_directory.at(idx)};
    auto const  it {variants.find(variant)};
    return it != variants.end() ? it->second : variants.at(0);
}

void texture_cache::load_image(i32 idx, i32 variant, string const& file)
{
    auto img {*gfx::image::Load(file)};

    _directory[idx][variant].Offset = _textures.size();
    _directory[idx][variant].Size   = img.info().Size;

    _textures.append_range(gfx::filters::alpha_remover {}(img).data());
}

auto texture_cache::texture(i32 idx, i32 variant) -> u8*
{
    return _textures.data() + get_entry(idx, variant).Offset;
}

auto texture_cache::texture_size(i32 idx, i32 variant) const -> size_i
{
    return get_entry(idx, variant).Size;
}

void texture_cache::load()
{
    io::create_folder("cache");

    // PLACEHOLDER START

    // SPRITES
    load_image(1, 0, "res/wall0.png");
    load_image(2, 0, "res/wall1.png");
    load_image(3, 0, "res/wall2.png");
    load_image(4, 0, "res/wall3.png");
    load_image(5, 0, "res/wall4.png");
    load_image(6, 0, "res/wall5.png");
    load_image(7, 0, "res/wall7.png");
    load_image(8, 0, "res/wall7.png");
    load_image(9, 0, "res/wall7.png");
    load_image(door1Texture, 0, "res/door.png");
    load_image(door1FrameTexture, 0, "res/door_frame.png");
    load_image(10, 0, "res/floor.png");
    load_image(11, 0, "res/ceiling.png");
    load_image(14, 0, "res/sky.png");
    load_image(15, 0, "res/transparent.png");
    load_image(fontTexture, 0, "res/font.png");
    load_image(handTexture, 0, "res/hand.png");

    // PLACEHOLDER END
}
