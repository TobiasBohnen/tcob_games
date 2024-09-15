// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Tiles.hpp"
#include "assets/GameAssets.hpp"

namespace stn {

/////////////////////////////////////////////////////

tilemap::tilemap(assets const& assets)
    : _tiles {assets.get_texture("tiles0"), assets.get_tiles_def("tiles0")}
{
    for (usize y {0}; y < _tilemap.size(); ++y) {
        for (usize x {0}; x < _tilemap[y].size(); ++x) {
            if (y == 0 || y == _tilemap.size() - 1 || x == 0 || x == _tilemap[y].size() - 1) {
                _tilemap[y][x] = _tiles.Set["solid3"].as<tile>();
            } else {
                _tilemap[y][x] = _tiles.Set["penta"].as<tile>();
            }
        }
    }

    std::array<std::array<std::string, 5>, 5> room {{{"wall-nw", "wall-n", "wall-n", "wall-n", "wall-ne"},
                                                     {"wall-w", "carpet0", "checker-small", "carpet1", "wall-e"},
                                                     {"wall-w", "solid1", "carpet2", "solid2", "wall-e"},
                                                     {"wall-w", "tile-neg", "checker", "tile", "wall-e"},
                                                     {"wall-sw", "wall-s", "wall-s", "wall-s", "wall-se"}}};
    i32                                       startX = 0;
    i32                                       startY = 0;
    for (size_t i = 0; i < room.size(); ++i) {
        for (size_t j = 0; j < room[i].size(); ++j) {
            // _tileMap[startY + i][startX + j] = _tiles.Set[room[i][j]].as<tile>();
        }
    }
}

auto tilemap::check_solid(point_i pos) const -> bool
{
    return _tilemap[pos.Y][pos.X].Solid;
}

auto tilemap::get_offset() const -> point_f
{
    return _offset;
}

void tilemap::set_offset(point_f pos)
{
    _offset.X = std::min(std::max(0.f, pos.X - 4.f), static_cast<f32>(MAP_TILES.Width - SCREEN_TILES.Width));
    _offset.Y = std::min(std::max(-1.f, pos.Y - 4.f), static_cast<f32>(MAP_TILES.Height - SCREEN_TILES.Height));
}

void tilemap::draw(gfx::canvas& canvas)
{
    for (i32 y {-1}; y < SCREEN_TILES.Height + 1; ++y) {
        for (i32 x {-1}; x < SCREEN_TILES.Width + 1; ++x) {
            i32 const yOff {y + static_cast<i32>(_offset.Y)};
            i32 const xOff {x + static_cast<i32>(_offset.X)};
            if (yOff < 0 || yOff >= _tilemap.size() || xOff < 0 || xOff >= _tilemap[yOff].size()) { continue; }

            auto const& tile {_tilemap[yOff][xOff]};
            if (tile.Texture.empty()) { continue; }

            canvas.set_fill_style(colors::White);
            canvas.draw_image(
                _tiles.Texture, tile.Texture,
                {{TILE_SIZE_F.Width * (xOff - _offset.X), TILE_SIZE_F.Height * (yOff - _offset.Y)}, TILE_SIZE_F});
        }
    }
}

}
