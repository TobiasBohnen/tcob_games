// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Tiles.hpp"

#include "GameCanvas.hpp"
#include "Player.hpp"
#include "assets/GameAssets.hpp"

namespace stn {

/////////////////////////////////////////////////////

tilemap::tilemap(assets& assets)
    : _assets {assets}
    , _tiles {_assets.get_texture("tiles0"), _assets.get_tiles_def("tiles0")}
    , _objects {_assets.get_texture("objects0"), _assets.get_tiles_def("objects0")}
{
}

auto tilemap::check_solid(point_i pos) const -> bool
{
    auto const& tile {_tilemap[pos.Y][pos.X]};
    auto const& obj {_objmap[pos.Y][pos.X]};
    return tile.Solid && (obj.Solid || obj.Texture.empty()); // non-solid objects to tiles non-solid
}

auto tilemap::get_offset() const -> point_f
{
    return _offset;
}

void tilemap::set_offset(point_f pos)
{
    _offset.X = std::min(std::max(0.f, pos.X - 4.f), static_cast<f32>(MAP_TILES.Width - SCREEN_TILES.Width));
    _offset.Y = std::min(std::max(-1.f, pos.Y - 4.f), static_cast<f32>(MAP_TILES.Height - SCREEN_TILES.Height));

    _offset.X = std::round(_offset.X * TILE_SIZE_F.Width) / TILE_SIZE_F.Width;
    _offset.Y = std::round(_offset.Y * TILE_SIZE_F.Height) / TILE_SIZE_F.Height;
}

void tilemap::make_basic_layout(std::string const& walls, std::string const& floor)
{
    for (usize y {0}; y < _tilemap.size(); ++y) {
        for (usize x {0}; x < _tilemap[y].size(); ++x) {
            if (y == 0 || y == _tilemap.size() - 1 || x == 0 || x == _tilemap[y].size() - 1) {
                _tilemap[y][x] = _tiles.Set[walls].as<tile>();
            } else {
                _tilemap[y][x] = _tiles.Set[floor].as<tile>();
            }
        }
    }
}

void tilemap::set_tiles(std::array<std::array<std::string, MAP_TILES.Width>, MAP_TILES.Height> const& tiles)
{
    for (usize y {0}; y < tiles.size(); ++y) {
        for (usize x {0}; x < tiles[y].size(); ++x) {
            std::string name {tiles[y][x]};
            _tilemap[y][x] = name.empty() ? tile {} : _tiles.Set[name].as<tile>();
        }
    }
}

void tilemap::set_tiles(std::vector<std::vector<std::string>> const& tiles, point_i offset)
{
    for (i32 y {0}; y < tiles.size() && y < MAP_TILES.Height; ++y) {
        for (i32 x {0}; x < tiles[y].size() && x < MAP_TILES.Width; ++x) {
            std::string name {tiles[y][x]};
            _tilemap[offset.Y + y][offset.X + x] = name.empty() ? tile {} : _tiles.Set[name].as<tile>();
        }
    }
}

void tilemap::set_object(point_i pos, std::string const& obj)
{
    _objmap[pos.Y][pos.X] = _objects.Set[obj].as<tile>();
}

void tilemap::draw(canvas& canvas)
{
    auto& ctx {canvas.get_context()};

    for (i32 y {-1}; y < SCREEN_TILES.Height + 1; ++y) {
        for (i32 x {-1}; x < SCREEN_TILES.Width + 1; ++x) {
            i32 const yOff {y + static_cast<i32>(_offset.Y)};
            i32 const xOff {x + static_cast<i32>(_offset.X)};
            if (yOff < 0 || yOff >= _tilemap.size() || xOff < 0 || xOff >= _tilemap[yOff].size()) { continue; }

            f32 const   xPos {TILE_SIZE_F.Width * (xOff - _offset.X)};
            f32 const   yPos {TILE_SIZE_F.Height * (yOff - _offset.Y)};
            auto const& tile {_tilemap[yOff][xOff]};
            if (!tile.Texture.empty()) {
                canvas.draw_image(_tiles.Texture, tile.Texture, {{xPos, yPos}, TILE_SIZE_F});
            }

            auto const& obj {_objmap[yOff][xOff]};
            if (!obj.Texture.empty()) {
                canvas.draw_image(_objects.Texture, obj.Texture, {{xPos, yPos}, TILE_SIZE_F});
            }
        }
    }
}

void tilemap::set_lighting(lighting const& setting)
{
    _lighting = setting;
}

void tilemap::cast_shadow(point_f playerPos, point_f canvasPos, color& target) const
{
    point_i const playerTilePos {point_f {playerPos.X / TILE_SIZE_F.Width + _offset.X, playerPos.Y / TILE_SIZE_F.Height + _offset.Y}};
    point_i const canvasTilePos {point_f {canvasPos.X / TILE_SIZE_F.Width + _offset.X, canvasPos.Y / TILE_SIZE_F.Height + _offset.Y}};
    auto const    dist {playerPos.distance_to(canvasPos)};

    i32 shd {3};
    for (i32 i {0}; i < _lighting.Falloff.size(); ++i) {
        if (dist < _lighting.Falloff[i]) {
            shd = i;
            break;
        }
    }

    if (shd != 0) {
        if (shd == 3 || target == COLOR2) {
            target = COLOR3;
        } else if (target == COLOR0) {
            target = (shd == 1) ? COLOR1 : COLOR2;
        } else if (target == COLOR1) {
            target = (shd == 1) ? COLOR2 : COLOR3;
        }
    }

    if (target != COLOR3) {
        // ignores non-solid objects
        if (_tilemap[canvasTilePos.Y][canvasTilePos.X].Solid) {
            return;
        }

        func::linear<point_f> lin;
        lin.StartValue = playerPos;
        lin.EndValue   = canvasPos;

        point_i lastVisited {-1, -1};
        for (f32 i {0}; i <= 1; i += 0.01f) {
            auto const    pos {lin(i)};
            point_i const tilePos {point_f {pos.X / TILE_SIZE_F.Width + _offset.X, pos.Y / TILE_SIZE_F.Height + _offset.Y}};

            if (lastVisited == tilePos) { continue; }
            lastVisited = tilePos;
            if (tilePos.Y < 0 || tilePos.Y >= _tilemap.size() || tilePos.X < 0 && tilePos.X >= _tilemap[tilePos.Y].size()) { continue; }

            if (check_solid(tilePos)) {
                target = COLOR3;
                return;
            }
        }
    }
}

auto tilemap::is_in_cone(player const& player, point_f playerPos, point_f canvasPos) const -> bool
{
    auto const  angle {playerPos.angle_to(canvasPos)};
    auto const& cones {_lighting.VisionCones};
    switch (player.get_direction()) {
    case direction::Left:
        return (angle.Value >= cones.at(direction::Left).first && angle.Value <= cones.at(direction::Left).second);
    case direction::Right:
        return (angle.Value >= cones.at(direction::Right).first && angle.Value <= cones.at(direction::Right).second);
    case direction::Up:
        return (angle.Value >= cones.at(direction::Up).first || angle.Value <= cones.at(direction::Up).second);
    case direction::Down:
        return angle.Value >= cones.at(direction::Down).first && angle.Value <= cones.at(direction::Down).second;

    case direction::None: return false;
    }
}

void tilemap::draw_shadow(canvas& canvas, player const& player)
{
    auto&       ctx {canvas.get_context()};
    gfx::image& img {canvas.get_last_frame()};

    if (_lighting.Off) {
        auto* scratchTex {_assets.get_scratch_texture()};
        scratchTex->update_data(img.get_data(), 0);
        canvas.draw_image(scratchTex, "default", {point_f::Zero, CANVAS_SIZE_F});
        return;
    }

    point_f const playerPos {(player.get_position().X - _offset.X) * TILE_SIZE_F.Width + TILE_SIZE_F.Width / 2, (player.get_position().Y - _offset.Y) * TILE_SIZE_F.Height + TILE_SIZE_F.Height / 2};

    for (i32 y {0}; y < CANVAS_SIZE.Height; ++y) {
        for (i32 x {0}; x < CANVAS_SIZE.Width; ++x) {
            point_f const canvasPos {static_cast<f32>(x), static_cast<f32>(y)};

            bool const isInCone {is_in_cone(player, playerPos, canvasPos)};
            auto const col {img.get_pixel({x, y})};
            if (col == COLOR3) { continue; }

            color target {col};
            if (!isInCone) {
                target = COLOR3;
            } else {
                cast_shadow(playerPos, canvasPos, target);
            }

            img.set_pixel({x, y}, target);
        }
    }

    auto* scratchTex {_assets.get_scratch_texture()};
    scratchTex->update_data(img.get_data(), 0);
    canvas.draw_image(scratchTex, "default", {point_f::Zero, CANVAS_SIZE_F});
}
}
