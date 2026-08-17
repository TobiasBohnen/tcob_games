// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "TextureCache.hpp"

#include "Common.hpp"
#include "Voxel.hpp"

auto texture_cache::get_entry(i32 idx, i32 variant) const -> texture_entry const&
{
    auto const& variants {_directory.at(idx)};
    auto const  it {variants.find(variant)};
    return it != variants.end() ? it->second : variants.at(0);
}

void texture_cache::load_vox(i32 idx, string const& file)
{
    usize const HEADER_SIZE {sizeof(u32) + sizeof(i32) + sizeof(i32)};

    u32 const crc {io::file_hasher(file).crc32()};

    string     inFile;
    auto const checkCache {[&](string const& file) {
        if (io::is_file(file)) {
            io::ifstream str {file};
            u32 const    cCrc {str.read<u32>()};
            i32 const    cSize {str.read<i32>()};
            i32 const    cFac {str.read<i32>()};

            bool const retValue {cCrc == crc && cSize == VOXEL_SIZE && cFac == NUM_FACINGS};
            if (retValue) { inFile = file; }
            return retValue;
        }
        return false;
    }};

    string const outFile {"cache/" + io::get_filename(file) + ".blob"};
    string const resFile {"res/cache/" + io::get_filename(file) + ".blob"};
    if (!checkCache(outFile) && !checkCache(resFile)) {
        io::delete_file(outFile);
        io::ofstream str {outFile};
        str.write<u32>(crc);
        str.write<i32>(VOXEL_SIZE);
        str.write<i32>(NUM_FACINGS);

        auto const voxels {load_vox_file(file)};
        auto       facings {voxels->bake_facings(VOXEL_SIZE, -90, NUM_FACINGS)};
        str.write_filtered(std::as_bytes(std::span {facings}), io::zlib_filter {});
        inFile = outFile;
    }

    for (i32 i {0}; i < NUM_FACINGS; ++i) {
        _directory[idx][i].Offset = _textures.size() + (i * VOXEL_SIZE * VOXEL_SIZE * TEXTURE_BPP);
        _directory[idx][i].Size   = {VOXEL_SIZE, VOXEL_SIZE};
    }

    io::ifstream str {inFile};
    str.seek(HEADER_SIZE, io::seek_dir::Begin);
    auto const bytes {str.read_filtered(str.size_in_bytes() - HEADER_SIZE, io::zlib_filter {})};
    _textures.append_range(std::span<u8 const> {reinterpret_cast<u8 const*>(bytes.data()), bytes.size()});
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

    // VOXELS
    load_vox(sprite1Texture, "res/ball1.vox");

    // PLACEHOLDER END
}
