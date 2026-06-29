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

raycaster::raycaster(cache& cache, size_i screenSize, f64 projPlaneDist)
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
        _rowDist[y] = projPlaneDist / ((2.0 * y) - _screenSize.Height);
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

static auto is_magenta(u8 const* tex, i32 offset) -> bool
{
    return tex[offset + 0] == 0x98 && tex[offset + 1] == 0x00 && tex[offset + 2] == 0x88;
}

static auto shade_from_side(hit_side side) -> f64
{
    switch (side) {
    case hit_side::NorthSouth: return 1.0;
    case hit_side::WestEast:   return 0.5;
    case hit_side::Diagonal:   return 0.75;
    }
    std::unreachable();
}

void raycaster::draw_wall_column(wall_hit const& hit, f64 invFogDistance, level const& level, player const& player, u32* screenBuf, isize x, bool transparent)
{
    i32 const lineHeight {static_cast<i32>(player.ProjPlaneDist / hit.Distance)};
    i32 const drawStart {std::max((-lineHeight / 2) + (_screenSize.Height / 2), 0)};
    i32 const drawEnd {std::min((lineHeight / 2) + (_screenSize.Height / 2), _screenSize.Height - 1)};

    auto const* tex {_cache.texture(hit.Texture, 0)};
    i32 const   texX {static_cast<i32>(hit.SegmentT * static_cast<f64>(WALL_SIZE.Width))};
    f64 const   texStep {1.0 * WALL_SIZE.Height / lineHeight};
    f64         texPos {(drawStart - (_screenSize.Height / 2) + (lineHeight / 2)) * texStep};

    f64 const wallFogFactor {std::max(1.0 - (hit.Distance * invFogDistance), level.FogMin)};
    f64 const wallDarkenFactor {shade_from_side(hit.Side) * wallFogFactor * (level.AmbientLight + hit.Light)};

    for (i32 y {drawStart}; y < drawEnd; y++) {
        i32 const texY {static_cast<i32>(texPos) & (WALL_SIZE.Height - 1)};
        texPos += texStep;
        i32 const srcIdx {(texX + (texY * WALL_SIZE.Width)) * TEXTURE_BPP};
        if (transparent) {
            if (is_magenta(tex, srcIdx)) { continue; }
            isize const depthIndex {x + (static_cast<isize>(y) * _screenSize.Width)};
            _spriteDepthBuffer[depthIndex] = std::min(_spriteDepthBuffer[depthIndex], hit.Distance);
        }
        cache::copy(screenBuf + x + ((_screenSize.Height - y - 1) * _screenSize.Width), tex, srcIdx, wallDarkenFactor);
    }
}

void raycaster::draw_floor_ceiling_column(wall_hit const& hit, f64 invFogDistance, level const& level, player const& player, u32* screenBuf, isize x, point_d rayDir)
{
    i32 const lineHeight {static_cast<i32>(player.ProjPlaneDist / hit.Distance)};
    i32 const drawStart {std::max((-lineHeight / 2) + (_screenSize.Height / 2), 0)};
    i32 const drawEnd {std::min((lineHeight / 2) + (_screenSize.Height / 2), _screenSize.Height - 1)};

    point_d const floorWall {player.Pos + (rayDir * hit.Distance)};
    f64 const     invPerpWallDist {1.0 / hit.Distance};

    i32 const   skyTexX {level.IsSkybox ? static_cast<i32>(std::fmod((std::atan2(rayDir.Y, rayDir.X) / TAU) + 1.0, 1.0) * SKY_SIZE.Width) % SKY_SIZE.Width : 0};
    auto const* skyTex {level.IsSkybox ? _cache.texture(level.CeilingTexture, 0) : nullptr};

    point_i     lastFloorCell {-1, -1};
    i32         cellFloorTex {level.FloorTexture};
    i32         cellCeilTex {level.CeilingTexture};
    f64         cellLight {0.0};
    auto const* cellFloorTexPtr {_cache.texture(cellFloorTex, 0)};
    auto const* cellCeilTexPtr {_cache.texture(cellCeilTex, 0)};

    auto const get_cell {[&](auto&& w) {
        if constexpr (requires { w.FloorTexture; }) {
            if (w.FloorTexture != INVALID_INDEX) { cellFloorTex = w.FloorTexture; }
        }
        if constexpr (requires { w.CeilingTexture; }) {
            if (w.CeilingTexture != INVALID_INDEX) { cellCeilTex = w.CeilingTexture; }
        }
        if constexpr (requires { w.Light; }) {
            cellLight = w.Light;
        }
    }};

    for (i32 y {drawEnd}; y < _screenSize.Height; y++) {
        f64 const weight {std::min(_rowDist[y] * invPerpWallDist, 1.0)};

        point_d const currentFloor {(weight * floorWall.X) + ((1.0 - weight) * player.Pos.X),
                                    (weight * floorWall.Y) + ((1.0 - weight) * player.Pos.Y)};

        point_d const delta {currentFloor.X - player.Pos.X, currentFloor.Y - player.Pos.Y};
        f64 const     floorDist {std::sqrt((delta.X * delta.X) + (delta.Y * delta.Y))};
        f64 const     fogFactor {std::max(1.0 - (floorDist * invFogDistance), level.FogMin)};

        i32 const texelX {static_cast<i32>(currentFloor.X * FLOOR_SIZE.Width) & (FLOOR_SIZE.Width - 1)};
        i32 const texelY {static_cast<i32>(currentFloor.Y * FLOOR_SIZE.Height) & (FLOOR_SIZE.Height - 1)};
        i32 const texelOffset {(texelX + (texelY * FLOOR_SIZE.Width)) * TEXTURE_BPP};

        point_i const floorCell {static_cast<i32>(currentFloor.X), static_cast<i32>(currentFloor.Y)};
        if (floorCell != lastFloorCell) {
            lastFloorCell = floorCell;
            cellFloorTex  = level.FloorTexture;
            cellCeilTex   = level.CeilingTexture;
            cellLight     = 0.0;
            std::visit(get_cell, level.Map[floorCell]);
            cellFloorTexPtr = _cache.texture(cellFloorTex, 0);
            cellCeilTexPtr  = _cache.texture(cellCeilTex, 0);
        }

        f64 const cellFogFactor {fogFactor * (level.AmbientLight + cellLight)};
        cache::copy(screenBuf + x + ((_screenSize.Height - y - 1) * _screenSize.Width), cellFloorTexPtr, texelOffset, cellFogFactor);

        if (level.IsSkybox) {
            i32 const skyTexY {static_cast<i32>(std::min(static_cast<f64>(_screenSize.Height - y - 1) / static_cast<f64>(_screenSize.Height / 2), 1.0) * SKY_SIZE.Height) & (SKY_SIZE.Height - 1)};
            i32 const skyOffset {(skyTexX + (skyTexY * SKY_SIZE.Width)) * TEXTURE_BPP};
            cache::copy(screenBuf + x + (y * _screenSize.Width), skyTex, skyOffset, 1.0);
        } else {
            cache::copy(screenBuf + x + (y * _screenSize.Width), cellCeilTexPtr, texelOffset, cellFogFactor);
        }
    }
}

void raycaster::draw_walls(level const& level, player const& player, u32* screenBuf, i32 columnStart, i32 columnEnd)
{
    f64 const invFogDistance {1.0 / level.FogDistance};

    auto const get_light {[&](point_i const& cell) -> f64 {
        return std::visit([](auto&& w) -> f64 {
            if constexpr (requires { w.Light; }) { return w.Light; }
            return 0.0;
        },
                          level.Map[cell]);
    }};

    for (isize x {columnStart}; x < columnEnd; x++) {
        f64 const     cameraX {(2.0 * x / _screenSize.Width) - 1.0};
        point_d const rayDir {player.Direction + (player.Plane * cameraX)};

        point_i map {player.Pos};

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

        std::array<wall_hit, MAX_TRANSPARENT_WALLS> transparentHits {};
        i32                                         transparentCount {0};

        auto const process_hit {[&](wall_hit const& wallHit, point_i const& cell) {
            wall_hit h {wallHit};
            h.Light = get_light(cell);
            if (h.Transparent) {
                if (transparentCount < MAX_TRANSPARENT_WALLS) { transparentHits[transparentCount++] = h; }
            } else {
                hitResult = h;
            }
        }};

        // check player cell
        {
            auto const intersect {[&](auto&& c) { return c.intersect({map, player.Pos, rayDir, sideDist.X < sideDist.Y, 0.0}); }};
            auto const wallHit {std::visit(intersect, level.Map[map])};
            if (wallHit.Hit) { process_hit(wallHit, map); }
        }

        // DDA
        if (!hitResult.Hit) {
            bool       side {false};
            auto const intersect {[&](auto&& c) -> wall_hit { return c.intersect({map, player.Pos, rayDir, side, !side ? sideDist.X - deltaDist.X : sideDist.Y - deltaDist.Y}); }};

            for (;;) {
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
                    process_hit(wallHit, map);
                    if (hitResult.Hit) { break; }
                }
            }
        }

        if (!hitResult.Hit) { continue; }

        _zBuffer[x] = hitResult.Distance;
        draw_wall_column(hitResult, invFogDistance, level, player, screenBuf, x, false);
        draw_floor_ceiling_column(hitResult, invFogDistance, level, player, screenBuf, x, rayDir);

        for (i32 i {transparentCount - 1}; i >= 0; --i) {
            draw_wall_column(transparentHits[i], invFogDistance, level, player, screenBuf, x, true);
        }
    }
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
    f64 const invDet {1.0 / player.Plane.cross(player.Direction)};
    f64 const invFogDistance {1.0 / level.FogDistance};

    for (sprite const& spr : level.Sprites) {
        point_d const relPos {spr.Pos - player.Pos};

        f64 const transformX {invDet * relPos.cross(player.Direction)};
        f64 const transformY {invDet * player.Plane.cross(relPos)};
        if (transformY <= 0) { continue; }

        i32 const    spriteScreenX {static_cast<i32>((_screenSize.Width / 2.0) * (1.0 + (transformX / transformY)))};
        f64 const    scale {player.ProjPlaneDist / transformY};
        size_i const spriteSize {static_cast<i32>(std::abs(scale)) * size_i {spr.Size}};

        point_i const drawStart {std::max({(-spriteSize.Width / 2) + spriteScreenX, 0, columnStart}),
                                 std::max((-spriteSize.Height / 2) + (_screenSize.Height / 2), 0)};
        point_i const drawEnd {std::min({(spriteSize.Width / 2) + spriteScreenX, _screenSize.Width - 1, columnEnd}),
                               std::min((spriteSize.Height / 2) + (_screenSize.Height / 2), _screenSize.Height - 1)};
        if (drawStart.X >= drawEnd.X) { continue; }
        if (drawStart.Y >= drawEnd.Y) { continue; }

        i32 const spriteLeft {spriteScreenX - (spriteSize.Width / 2)};
        i32 const spriteTop {(_screenSize.Height / 2) - (spriteSize.Height / 2)};

        i32 const    facing {sprite_facing_index(spr.Facing, spr.Pos, player.Pos)};
        auto const*  tex {_cache.texture(spr.Texture, facing)};
        size_i const texSize {_cache.texture_size(spr.Texture, facing)};

        f64 const texStepY {1.0 * texSize.Height / spriteSize.Height};
        f64 const texPosYStart {(drawStart.Y - spriteTop) * texStepY};

        f64 spriteLight {0.0};
        std::visit([&](auto&& w) {
            if constexpr (requires { w.Light; }) { spriteLight = w.Light; }
        },
                   level.Map[point_i {spr.Pos}]);

        f64 const spriteFogFactor {std::max(1.0 - (transformY * invFogDistance), level.FogMin) * (level.AmbientLight + spriteLight)};

        for (i32 stripe {drawStart.X}; stripe < drawEnd.X; ++stripe) {
            i32 const texX {((stripe - spriteLeft) * texSize.Width) / spriteSize.Width};

            if (transformY >= _zBuffer[stripe]) { continue; }

            f64 texPos {texPosYStart};
            for (i32 y {drawStart.Y}; y < drawEnd.Y; ++y) {
                i32 const texY {static_cast<i32>(texPos) & (texSize.Height - 1)};
                texPos += texStepY;

                i32 const offset {(texX + (texY * texSize.Width)) * TEXTURE_BPP};
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
