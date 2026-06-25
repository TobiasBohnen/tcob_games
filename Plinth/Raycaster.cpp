// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Raycaster.hpp"

#include <algorithm>
#include <cmath>

#include "Cache.hpp"
#include "Common.hpp"
#include "Level.hpp"
#include "Player.hpp"
#include "Walls.hpp"

raycaster::raycaster(cache& cache, size_i screenSize)
    : _cache {cache}
    , _screenSize {screenSize}
{
    _zBuffer.resize(_screenSize.Width);

    _rowDist.resize(_screenSize.Height);
    for (i32 y {0}; y < _screenSize.Height; y++) {
        if (2.0 * y == _screenSize.Height) {
            _rowDist[y] = std::numeric_limits<f64>::infinity();
            continue;
        }
        _rowDist[y] = _screenSize.Height / ((2.0 * y) - _screenSize.Height);
    }

    _spriteDepthBuffer.resize(static_cast<usize>(_screenSize.Width) * _screenSize.Height);
}

void raycaster::prepare_draw()
{
    std::ranges::fill(_spriteDepthBuffer, std::numeric_limits<f64>::infinity());
}

void raycaster::draw(level const& level, player const& player, u32* screenBuf, i32 columnStart, i32 columnEnd)
{
    draw_walls(level, player, screenBuf, columnStart, columnEnd);
    draw_sprites(level, player, screenBuf, columnStart, columnEnd);
}

void raycaster::draw_walls(level const& level, player const& player, u32* screenBuf, i32 columnStart, i32 columnEnd)
{
    for (isize x {columnStart}; x < columnEnd; x++) {
        // WALL CASTING
        // calculate ray position and direction
        f64 const     cameraX {(2.0 * x / _screenSize.Width) - 1.0}; // x-coordinate in camera space
        point_d const rayDir {player.Direction + (player.Plane * cameraX)};

        point_i map {static_cast<point_i>(player.Pos)};

        point_d const deltaDist {(rayDir.X == 0) ? 1e30 : std::abs(1 / rayDir.X), (rayDir.Y == 0) ? 1e30 : std::abs(1 / rayDir.Y)};

        point_i step {};
        point_d sideDist {};
        if (rayDir.X < 0) {
            step.X     = -1;
            sideDist.X = (player.Pos.X - map.X) * deltaDist.X;
        } else {
            step.X     = 1;
            sideDist.X = (map.X + 1.0 - player.Pos.X) * deltaDist.X;
        }
        if (rayDir.Y < 0) {
            step.Y     = -1;
            sideDist.Y = (player.Pos.Y - map.Y) * deltaDist.Y;
        } else {
            step.Y     = 1;
            sideDist.Y = (map.Y + 1.0 - player.Pos.Y) * deltaDist.Y;
        }

        wall_hit hitResult {};

        // check player cell
        {
            auto const intersect {[&](auto&& c) { return c.intersect(map, player.Pos, rayDir, sideDist.X < sideDist.Y, 0.0); }};
            auto const wallHit {std::visit(intersect, level.Map[map])};
            if (wallHit.Hit) { hitResult = wallHit; }
        }
        // DDA
        if (!hitResult.Hit) {
            bool       side {false}; // was a NS or a EW wall hit?
            auto const intersect {[&](auto&& c) -> wall_hit { return c.intersect(map, player.Pos, rayDir, side, !side ? sideDist.X - deltaDist.X : sideDist.Y - deltaDist.Y); }};

            for (;;) {
                // jump to next map square, either in x-direction, or in y-direction
                if (sideDist.X < sideDist.Y) {
                    sideDist.X += deltaDist.X;
                    map.X += step.X;
                    side = false;
                } else {
                    sideDist.Y += deltaDist.Y;
                    map.Y += step.Y;
                    side = true;
                }

                if (!map_t::Size.contains(map)) { break; }
                auto const wallHit {std::visit(intersect, level.Map[map])};
                if (wallHit.Hit) {
                    hitResult = wallHit;
                    break;
                }
            }
        }

        if (!hitResult.Hit) { continue; } // TODO: fake wall instead?
        f64 const perpWallDist {hitResult.Distance};
        _zBuffer[x] = perpWallDist;

        // Calculate height of line to draw on screen
        i32 const lineHeight {static_cast<i32>(player.ProjPlaneDist / perpWallDist)};

        // calculate lowest and highest pixel to fill in current stripe
        i32 const drawStart {std::max((-lineHeight / 2) + (_screenSize.Height / 2), 0)};
        i32 const drawEnd {std::min((lineHeight / 2) + (_screenSize.Height / 2), _screenSize.Height - 1)};

        // calculate value of wallX
        f64 const wallX {hitResult.SegmentT};

        {
            auto const*  tex {_cache.texture(hitResult.Texture, 0)};
            size_i const texSize {wallSize};

            // x coordinate on the texture
            i32 const texX {static_cast<i32>(wallX * static_cast<f64>(texSize.Width))};

            // How much to increase the texture coordinate per screen pixel
            f64 const texStep {1.0 * texSize.Height / lineHeight};
            // Starting texture coordinate
            f64       texPos {(drawStart - (_screenSize.Height / 2) + (lineHeight / 2)) * texStep};

            f64 const wallFaceFactor {hitResult.Side ? 0.5 : 1.0};
            f64 const wallFogFactor {std::max(1.0 - (perpWallDist / level.FogDistance), level.FogMin)};
            f64 const wallDarkenFactor {wallFaceFactor * wallFogFactor};

            for (i32 y {drawStart}; y < drawEnd; y++) {
                i32 const texY {static_cast<i32>(texPos) & (texSize.Height - 1)};
                texPos += texStep;
                cache::copy(screenBuf + x + ((_screenSize.Height - y - 1) * _screenSize.Width), tex, (texX + (texY * texSize.Width)) * textureBPP, wallDarkenFactor);
            }
        }

        // FLOOR CASTING (vertical version, directly after drawing the vertical wall stripe for the current x)
        point_d const floorWall {player.Pos + (rayDir * perpWallDist)};

        // draw the floor from drawEnd to the bottom of the screen
        f64 const    invPerpWallDist {1.0 / perpWallDist};
        auto const*  floorTex {_cache.texture(floorTexture, 0)};
        size_i const floorSize {wallSize};
        auto const*  ceilTex {_cache.texture(ceilingTexture, 0)};

        for (i32 y {drawEnd}; y < _screenSize.Height; y++) {
            f64 const weight {std::min(_rowDist[y] * invPerpWallDist, 1.0)};

            point_d const currentFloor {(weight * floorWall.X) + ((1.0 - weight) * player.Pos.X), (weight * floorWall.Y) + ((1.0 - weight) * player.Pos.Y)};

            point_d const delta {currentFloor.X - player.Pos.X, currentFloor.Y - player.Pos.Y};
            f64 const     floorDist {std::sqrt((delta.X * delta.X) + (delta.Y * delta.Y))};
            f64 const     fogFactor {std::max(1.0 - (floorDist / level.FogDistance), level.FogMin)};

            i32 const texelX {static_cast<i32>(currentFloor.X * floorSize.Width) & (floorSize.Width - 1)};
            i32 const texelY {static_cast<i32>(currentFloor.Y * floorSize.Height) & (floorSize.Height - 1)};
            i32 const texelOffset {(texelX + (texelY * floorSize.Width)) * textureBPP};

            cache::copy(screenBuf + x + ((_screenSize.Height - y - 1) * _screenSize.Width), floorTex, texelOffset, fogFactor);
            cache::copy(screenBuf + x + (y * _screenSize.Width), ceilTex, texelOffset, fogFactor);
        }
    }
}

static auto is_magenta(u8 const* tex, i32 offset) -> bool
{
    return tex[offset + 0] == 0x98 && tex[offset + 1] == 0x00 && tex[offset + 2] == 0x88;
}

static auto sprite_facing_index(degree_d spriteFacing, point_d spritePos, point_d cameraPos) -> i32
{
    auto const viewAngle {spritePos.angle_to(cameraPos)};
    auto const relativeAngle {(viewAngle - spriteFacing).as_normalized(angle_normalize::PositiveFullTurn)};

    constexpr f64 wedge {360.0 / 8.0};
    i32 const     index {static_cast<i32>((relativeAngle.Value + (wedge / 2.0)) / wedge) % 8};

    return index;
}

void raycaster::draw_sprites(level const& level, player const& player, u32* screenBuf, i32 columnStart, i32 columnEnd)
{
    // camera plane determinant, used to "unrotate" sprite position into camera space
    f64 const invDet {1.0 / player.Plane.cross(player.Direction)};

    for (sprite const& spr : level.Sprites) {
        // translate sprite position relative to camera
        point_d const relPos {spr.Pos - player.Pos};

        // transform with the inverse camera matrix
        f64 const transformX {invDet * relPos.cross(player.Direction)};
        f64 const transformY {invDet * player.Plane.cross(relPos)};
        if (transformY <= 0) { continue; }                       // behind camera

        i32 const     spriteScreenX {static_cast<i32>((_screenSize.Width / 2.0) * (1.0 + (transformX / transformY)))};
        f64 const     scale {player.ProjPlaneDist / transformY}; // size of "one world unit" in screen pixels at this depth
        size_i const  spriteSize {static_cast<i32>(std::abs(scale)) * size_i {spr.Size}};
        point_i const drawStart {std::max({(-spriteSize.Width / 2) + spriteScreenX, 0, columnStart}),
                                 std::max((-spriteSize.Height / 2) + (_screenSize.Height / 2), 0)};
        point_i const drawEnd {std::min({(spriteSize.Width / 2) + spriteScreenX, _screenSize.Width - 1, columnEnd}),
                               std::min((spriteSize.Height / 2) + (_screenSize.Height / 2), _screenSize.Height - 1)};
        if (drawStart.X >= drawEnd.X) { continue; } // sprite has no columns in this thread's range

        i32 const spriteLeft {spriteScreenX - (spriteSize.Width / 2)};
        i32 const spriteTop {(_screenSize.Height / 2) - (spriteSize.Height / 2)};

        i32 const    facing {sprite_facing_index(spr.Facing, spr.Pos, player.Pos)};
        auto const*  tex {_cache.texture(spr.Texture, facing)};
        size_i const texSize {_cache.texture_size(spr.Texture, facing)};

        f64 const texStepY {1.0 * texSize.Height / spriteSize.Height};
        f64 const texPosYStart {(drawStart.Y - spriteTop) * texStepY};

        f64 const spriteFogFactor {std::max(1.0 - (transformY / level.FogDistance), level.FogMin)};

        for (i32 stripe {drawStart.X}; stripe < drawEnd.X; ++stripe) {
            i32 const texX {((stripe - spriteLeft) * texSize.Width) / spriteSize.Width};

            if (transformY >= _zBuffer[stripe]) { continue; }

            f64 texPos {texPosYStart};
            for (i32 y {drawStart.Y}; y < drawEnd.Y; ++y) {
                i32 texY {static_cast<i32>(texPos) & (texSize.Height - 1)};

                texPos += texStepY;

                i32 const offset {(texX + (texY * texSize.Width)) * textureBPP};
                if (is_magenta(tex, offset)) { continue; }

                isize const depthIndex {stripe + (static_cast<isize>(y) * _screenSize.Width)};
                if (transformY < _spriteDepthBuffer[depthIndex]) {
                    _spriteDepthBuffer[depthIndex] = transformY;
                    cache::copy(screenBuf + stripe + ((_screenSize.Height - y - 1) * _screenSize.Width), tex, offset, spriteFogFactor);
                }
            }
        }
    }
}
