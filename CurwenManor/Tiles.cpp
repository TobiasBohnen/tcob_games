// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Tiles.hpp"
#include "Player.hpp"
#include "assets/GameAssets.hpp"

namespace stn {

/////////////////////////////////////////////////////

tilemap::tilemap(assets& assets)
    : _assets {assets}
    , _tiles {_assets.get_texture("tiles0"), _assets.get_tiles_def("tiles0")}
{
    // TEST
    for (usize y {0}; y < _tilemap.size(); ++y) {
        for (usize x {0}; x < _tilemap[y].size(); ++x) {
            if (y == 0 || y == _tilemap.size() - 1 || x == 0 || x == _tilemap[y].size() - 1) {
                _tilemap[y][x] = _tiles.Set["solid3"].as<tile>();
            } else if (y == 4 && x % 4 < 3) {
                _tilemap[y][x] = _tiles.Set["wall-n"].as<tile>();
            } else {
                _tilemap[y][x] = _tiles.Set["grass1"].as<tile>();
            }
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

    _offset.X = std::round(_offset.X * TILE_SIZE_F.Width) / TILE_SIZE_F.Width;
    _offset.Y = std::round(_offset.Y * TILE_SIZE_F.Height) / TILE_SIZE_F.Height;
}

void tilemap::draw(canvas& canvas)
{
    auto& ctx {canvas.get_context()};

    for (i32 y {-1}; y < SCREEN_TILES.Height + 1; ++y) {
        for (i32 x {-1}; x < SCREEN_TILES.Width + 1; ++x) {
            i32 const yOff {y + static_cast<i32>(_offset.Y)};
            i32 const xOff {x + static_cast<i32>(_offset.X)};
            if (yOff < 0 || yOff >= _tilemap.size() || xOff < 0 || xOff >= _tilemap[yOff].size()) { continue; }

            auto const& tile {_tilemap[yOff][xOff]};
            if (tile.Texture.empty()) { continue; }

            f32 const xPos {std::floor(TILE_SIZE_F.Width * (xOff - _offset.X) + 0.5f)};
            f32 const yPos {std::floor(TILE_SIZE_F.Height * (yOff - _offset.Y) + 0.5f)};
            canvas.draw_image(_tiles.Texture, tile.Texture, {{xPos, yPos}, TILE_SIZE_F});
        }
    }
}

auto static angle_between_points(point_f p1, point_f p2) -> degree_f
{
    i32 val {static_cast<i32>(std::atan2(p2.Y - p1.Y, p2.X - p1.X) * (360.0f / TAU_F)) + 90};
    if (val < 0) {
        val += 360;
    }
    return val;
}

void tilemap::draw_shadow(canvas& canvas, player const& player)
{
    auto&       ctx {canvas.get_context()};
    gfx::image& img {canvas.get_last_frame()};

    point_f const playerPos {(player.get_position().X - _offset.X) * TILE_SIZE_F.Width + TILE_SIZE_F.Width / 2, (player.get_position().Y - _offset.Y) * TILE_SIZE_F.Height + TILE_SIZE_F.Height / 2};

    for (i32 y {0}; y < CANVAS_SIZE.Height; ++y) {
        for (i32 x {0}; x < CANVAS_SIZE.Width; ++x) {
            point_f const canvasPos {static_cast<f32>(x), static_cast<f32>(y)};

            bool isInCone {true};

            auto const angle {angle_between_points(playerPos, canvasPos)};
            switch (player.get_direction()) {
            case tcob::direction::Left:
                isInCone = (angle.Value >= 202 && angle.Value <= 338);
                break;
            case tcob::direction::Right:
                isInCone = (angle.Value >= 22 && angle.Value <= 158);
                break;
            case tcob::direction::Up:
                isInCone = (angle.Value >= 292 || angle.Value <= 68);
                break;
            case tcob::direction::Down:
                isInCone = angle.Value >= 112 && angle.Value <= 248;
                break;
            case tcob::direction::None: break;
            }

            auto const col {img.get_pixel({x, y})};
            if (col == COLOR3) { continue; }

            color target {col};
            if (!isInCone) {
                target = COLOR3;
            } else {
                auto const dist {playerPos.distance_to(canvasPos)};

                std::array<i32, 3> const falloff {25, 50, 60};
                i32                      shd {falloff.size()};
                for (i32 i {0}; i < falloff.size(); ++i) {
                    if (dist < falloff[i]) {
                        shd = i;
                        break;
                    }
                }

                if (shd != 0) {
                    if (shd == 3 || target == COLOR2) {
                        target = COLOR3;
                    } else if (col == COLOR0) {
                        target = (shd == 1) ? COLOR1 : COLOR2;
                    } else if (col == COLOR1) {
                        target = (shd == 1) ? COLOR2 : COLOR3;
                    }
                }

                if (target != COLOR3) {
                    point_i const canvasTilePos {point_f {canvasPos.X / TILE_SIZE_F.Width + _offset.X, canvasPos.Y / TILE_SIZE_F.Height + _offset.Y}};
                    auto const&   canvasTile {_tilemap[canvasTilePos.Y][canvasTilePos.X]};

                    if (!canvasTile.Solid || shd > 1) {
                        bool                  solid {false};
                        func::linear<point_f> lin;
                        lin.StartValue = playerPos;
                        lin.EndValue   = canvasPos;
                        for (f32 i {0}; i <= 1; i += 0.01f) {
                            auto const    pos {lin(i)};
                            point_i const tilePos {point_f {pos.X / TILE_SIZE_F.Width + _offset.X, pos.Y / TILE_SIZE_F.Height + _offset.Y}};
                            if (tilePos.Y >= 0 && tilePos.Y < _tilemap.size() && tilePos.X >= 0 && tilePos.X < _tilemap[tilePos.Y].size()) {
                                auto const& tile {_tilemap[tilePos.Y][tilePos.X]};

                                if (tile.Solid) {
                                    solid = true;
                                    break;
                                }
                            }
                        }

                        if (solid) { target = COLOR3; }
                    }
                }
            }

            img.set_pixel({x, y}, target);
        }
    }

    auto* scratchTex {_assets.get_scratch_texture()};
    scratchTex->update_data(img.get_data(), 0);

    canvas.draw_image(scratchTex, "default", {point_f::Zero, CANVAS_SIZE_F});
}
}
