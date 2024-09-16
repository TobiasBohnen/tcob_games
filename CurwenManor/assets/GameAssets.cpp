// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GameAssets.hpp"

#include "res_cutscenes.hpp"
#include "res_font.hpp"
#include "res_images.hpp"
#include "res_json.hpp"

namespace stn {

assets::assets(game& game)
    : _group {game.get_library().create_or_get_group("res")}
{
    // fonts
    _group.add_bucket<gfx::font_family>();
    auto fontFamily {_group.get_bucket<gfx::font_family>()->create_or_get("font", nullptr, "font")};

    gfx::font_family::SingleFont(*fontFamily, font_ttf);
    _font = fontFamily->get_font({}, 8).get_obj();
    _font->Render.connect([](auto&& pixels) { for (auto& pix : pixels) { pix = pix > 128 ? 255 : 0; } });
    _font->render_text("abcdefghijklmopqrstuvwxyz123456790", false, false);

    // textures
    _scratchTex.create(CANVAS_SIZE, 1, gfx::texture::format::RGBA8);
    _scratchTex.add_region("default", {{0, 0, 1, 1}, 0});

    _group.add_bucket<gfx::texture>();
    {
        gfx::image img {gfx::image::CreateEmpty(CANVAS_SIZE, gfx::image::format::RGB)};

        std::array<std::pair<std::string, std::span<u8 const>>, 9> images {{
            {"title0", pix_title_0},
            {"title1", pix_title_1},
            {"title2", pix_title_2},
            {"title3", pix_title_3},
            {"title4", pix_title_4},
            {"title5", pix_title_5},
            {"title6", pix_title_6},
            {"title7", pix_title_7},
            {"cutscenes0", pix_cutscenes_0},
        }};
        for (auto const& [k, v] : images) {
            for (i32 y {0}; y < CANVAS_SIZE.Height; ++y) {
                for (i32 x {0}; x < CANVAS_SIZE.Width; ++x) {
                    i32 const idx {y * CANVAS_SIZE.Width + x};
                    u8 const  col {v[idx]};
                    switch (col) {
                    case 0: img.set_pixel({x, y}, COLOR0); break;
                    case 1: img.set_pixel({x, y}, COLOR1); break;
                    case 2: img.set_pixel({x, y}, COLOR2); break;
                    case 3: img.set_pixel({x, y}, COLOR3); break;
                    default: assert(false); break;
                    }
                }
            }
            auto texture {_group.get_bucket<gfx::texture>()->create_or_get(k, nullptr)};
            texture->create(CANVAS_SIZE, 1, gfx::texture::format::RGB8);
            texture->update_data(img.get_data(), 0);
            texture->add_region("default", {{0, 0, 1, 1}, 0});
        }
    }

    {
        gfx::image img {gfx::image::CreateEmpty(TILE_SIZE, gfx::image::format::RGBA)};

        std::array<std::pair<std::string, std::span<u8 const>>, 2> tileSets {{
            {"tiles0", pix_tiles_0},
            {"player", pix_player_0},
        }};

        for (auto const& [k, v] : tileSets) {
            auto texture {_group.get_bucket<gfx::texture>()->create_or_get(k, nullptr)};
            texture->create(TILE_SIZE, TILE_COUNT_TOTAL, gfx::texture::format::RGBA8);
            texture->add_region("default", {{0, 0, 1, 1}, 0});

            for (i32 i {0}; i < TILE_COUNT_TOTAL; ++i) {
                i32 const x0 {i % TILE_COUNT.Width * TILE_SIZE.Width};
                i32 const y0 {i / TILE_COUNT.Width * TILE_SIZE.Height};
                for (i32 y {0}; y < TILE_SIZE.Height; ++y) {
                    i32 const offset {((y + y0) * TILESET_SIZE.Width) + x0};
                    for (i32 x {0}; x < TILE_SIZE.Width; ++x) {
                        i32 const idx {offset + x};
                        assert(idx < v.size());
                        u8 const col {v[idx]};
                        switch (col) {
                        case 0: img.set_pixel({x, y}, colors::Transparent); break;
                        case 1: img.set_pixel({x, y}, COLOR1); break;
                        case 2: img.set_pixel({x, y}, COLOR2); break;
                        case 3: img.set_pixel({x, y}, COLOR3); break;
                        default: assert(false); break;
                        }
                    }
                }
                texture->update_data(img.get_data(), i);

                gfx::texture_region reg;
                reg.Level  = i;
                reg.UVRect = {0, 0, 1, 1};
                texture->add_region("tile" + std::to_string(i), reg);
            }
        }
    }

    // json
    _cutsceneObj.parse(cutscene_json, ".json");
    _tilesObj.parse(tiles_json, ".json");
}

auto assets::get_default_font() -> gfx::font*
{
    return _font;
}

auto assets::get_texture(std::string const& name) const -> gfx::texture*
{
    return _group.get<gfx::texture>(name).get_obj();
}

auto assets::get_scratch_texture() -> gfx::texture*
{
    return &_scratchTex;
}

auto assets::get_cutscene_texts() -> data::config::object const&
{
    return _cutsceneObj;
}

auto assets::get_tiles_def(std::string const& name) const -> data::config::object
{
    return _tilesObj[name].as<data::config::object>();
}

auto assets::get_group() -> tcob::assets::group&
{
    return _group;
}

}