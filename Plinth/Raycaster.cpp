// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Raycaster.hpp"

#include <algorithm>
#include <cmath>

#include "Common.hpp"
#include "Level.hpp"
#include "Player.hpp"
#include "TextureCache.hpp"
#include "Walls.hpp"

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

static void set_pixel(u32* dst, u8 const* src, i32 srcIdx, f64 darken)
{
    u8 const r {static_cast<u8>(std::min(src[srcIdx + 0] * darken, 255.0))};
    u8 const g {static_cast<u8>(std::min(src[srcIdx + 1] * darken, 255.0))};
    u8 const b {static_cast<u8>(std::min(src[srcIdx + 2] * darken, 255.0))};
    *dst = (0xFF000000u) | (static_cast<u32>(b) << 16) | (static_cast<u32>(g) << 8) | static_cast<u32>(r);
}

static auto sprite_facing_index(degree_d spriteFacing, point_d spritePos, point_d cameraPos) -> i32
{
    auto const viewAngle {spritePos.angle_to(cameraPos)};
    auto const relativeAngle {(viewAngle - spriteFacing).as_normalized(angle_normalize::PositiveFullTurn)};

    constexpr f64 wedge {360.0 / 8.0};
    i32 const     index {static_cast<i32>((relativeAngle.Value + (wedge / 2.0)) / wedge) % 8};

    return index;
}

raycaster::raycaster(texture_cache& cache, size_i screenSize, f64 projPlaneDist)
    : _cache {cache}
    , _screen(screenSize.area())
    , _screenSize {screenSize}
    , _projPlaneDist {projPlaneDist}
{
    _zBuffer.resize(_screenSize.Width);
    _spriteDepthBuffer.resize(_screenSize.area());
}

auto raycaster::draw(level const& level, player const& player) -> u32 const*
{
    std::ranges::fill(_spriteDepthBuffer, std::numeric_limits<f64>::infinity());
    f64 const invFogDistance {1.0 / level.FogDistance};

    locate_service<task_manager>().run_parallel(
        [&](par_task const& ctx) {
            draw_columns(level, player, invFogDistance, static_cast<i32>(ctx.Start), static_cast<i32>(ctx.End));
            draw_sprites(level, player, invFogDistance, static_cast<i32>(ctx.Start), static_cast<i32>(ctx.End));
        },
        _screenSize.Width);

    draw_weapon(player);
    draw_hud(player);

    return _screen.data();
}

void raycaster::draw_columns(level const& level, player const& player, f64 invFogDistance, i32 columnStart, i32 columnEnd)
{
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

        point_i map {player.Position};

        point_d const deltaDist {(rayDir.X == 0) ? 1e30 : std::abs(1 / rayDir.X), (rayDir.Y == 0) ? 1e30 : std::abs(1 / rayDir.Y)};

        point_i step {};
        point_d sideDist {};
        if (rayDir.X < 0) {
            step.X     = -1;
            sideDist.X = (player.Position.X - map.X) * deltaDist.X;
        } else {
            step.X     = 1;
            sideDist.X = (map.X + 1.0 - player.Position.X) * deltaDist.X;
        }
        if (rayDir.Y < 0) {
            step.Y     = -1;
            sideDist.Y = (player.Position.Y - map.Y) * deltaDist.Y;
        } else {
            step.Y     = 1;
            sideDist.Y = (map.Y + 1.0 - player.Position.Y) * deltaDist.Y;
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
            auto const intersect {[&](auto&& c) { return c.intersect({map, player.Position, rayDir, sideDist.X < sideDist.Y, 0.0}); }};
            auto const wallHit {std::visit(intersect, level.Map[map])};
            if (wallHit.Hit) { process_hit(wallHit, map); }
        }

        // DDA
        if (!hitResult.Hit) {
            bool       side {false};
            auto const intersect {[&](auto&& c) -> wall_hit { return c.intersect({map, player.Position, rayDir, side, !side ? sideDist.X - deltaDist.X : sideDist.Y - deltaDist.Y}); }};

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

        draw_floor_ceiling_column(hitResult, level, player, x, rayDir, invFogDistance);

        draw_wall_column(hitResult, level, player, x, invFogDistance, false);

        for (i32 i {transparentCount - 1}; i >= 0; --i) {
            draw_wall_column(transparentHits[i], level, player, x, invFogDistance, true);
        }
    }
}

void raycaster::draw_wall_column(wall_hit const& hit, level const& level, player const& player, isize x, f64 invFogDistance, bool transparent)
{
    i32 const screenCenterY {(_screenSize.Height / 2) + static_cast<i32>(player.BobAmount)};
    i32 const lineHeight {static_cast<i32>(_projPlaneDist / hit.Distance)};

    i32 const yMinBound {(2 * screenCenterY) - _screenSize.Height + 1};
    i32 const yMaxBound {2 * screenCenterY};

    i32 const drawStart {std::max((-lineHeight / 2) + screenCenterY, yMinBound)};
    i32 const drawEnd {std::min((lineHeight / 2) + screenCenterY, yMaxBound + 1)};

    if (drawStart >= drawEnd) { return; }

    auto const* tex {_cache.texture(hit.Texture, 0)};
    i32 const   texX {static_cast<i32>(hit.SegmentT * static_cast<f64>(WALL_SIZE.Width))};
    f64 const   texStep {1.0 * WALL_SIZE.Height / lineHeight};

    i32 const rawDrawStart {(-lineHeight / 2) + screenCenterY};
    f64       texPos {(drawStart - rawDrawStart) * texStep};

    f64 const wallFogFactor {std::max(1.0 - (hit.Distance * invFogDistance), level.FogMin)};
    f64 const wallDarkenFactor {shade_from_side(hit.Side) * wallFogFactor * (level.AmbientLight + hit.Light)};

    u32* screenBuf {_screen.data()};
    for (i32 y {drawStart}; y < drawEnd; y++) {
        i32 const texY {static_cast<i32>(texPos) & (WALL_SIZE.Height - 1)};
        texPos += texStep;
        i32 const mirroredY {(2 * screenCenterY) - y};
        i32 const srcIdx {(texX + (texY * WALL_SIZE.Width)) * TEXTURE_BPP};

        bool const inBounds {mirroredY >= 0 && mirroredY < _screenSize.Height};

        if (transparent) {
            if (is_magenta(tex, srcIdx)) { continue; }
            if (inBounds) {
                isize const depthIndex {x + (static_cast<isize>(mirroredY) * _screenSize.Width)};
                _spriteDepthBuffer[depthIndex] = std::min(_spriteDepthBuffer[depthIndex], hit.Distance);
            }
        }

        if (inBounds) {
            set_pixel(screenBuf + x + (mirroredY * _screenSize.Width), tex, srcIdx, wallDarkenFactor);
        }
    }
}

void raycaster::draw_floor_ceiling_column(wall_hit const& hit, level const& level, player const& player, isize x, point_d rayDir, f64 invFogDistance)
{
    i32 const screenCenterY {(_screenSize.Height / 2) + static_cast<i32>(player.BobAmount)};
    i32 const lineHeight {static_cast<i32>(_projPlaneDist / hit.Distance)};
    i32 const drawEnd {std::min((lineHeight / 2) + (screenCenterY), _screenSize.Height - 1)};

    point_d const floorWall {player.Position + (rayDir * hit.Distance)};
    f64 const     invPerpWallDist {1.0 / hit.Distance};

    i32 const   fixedCenterY {_screenSize.Height / 2};
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

    u32* screenBuf {_screen.data()};

    i32 const maxFloorExtension {std::max(_screenSize.Height, (2 * screenCenterY)) + 1};

    for (i32 y {drawEnd}; y < maxFloorExtension; y++) {
        f64 const rowDist {y == screenCenterY ? std::numeric_limits<f64>::infinity()
                                              : _projPlaneDist / ((2.0 * y) - (2.0 * screenCenterY))};
        f64 const weight {std::min(rowDist * invPerpWallDist, 1.0)};

        point_d const currentFloor {(weight * floorWall.X) + ((1.0 - weight) * player.Position.X),
                                    (weight * floorWall.Y) + ((1.0 - weight) * player.Position.Y)};

        point_d const delta {currentFloor.X - player.Position.X, currentFloor.Y - player.Position.Y};
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

        i32 const mirroredY {(2 * screenCenterY) - y};

        if (mirroredY >= 0 && mirroredY < _screenSize.Height) {
            set_pixel(screenBuf + x + (mirroredY * _screenSize.Width), cellFloorTexPtr, texelOffset, cellFogFactor);
        }

        if (y >= 0 && y < _screenSize.Height) {
            if (level.IsSkybox) {
                i32 const skyTexY {static_cast<i32>(std::min(1.0 - (static_cast<f64>(y - fixedCenterY) / static_cast<f64>(_screenSize.Height - fixedCenterY)), 1.0) * SKY_SIZE.Height) & (SKY_SIZE.Height - 1)};
                i32 const skyOffset {(skyTexX + (skyTexY * SKY_SIZE.Width)) * TEXTURE_BPP};
                set_pixel(screenBuf + x + (y * _screenSize.Width), skyTex, skyOffset, 1.0);
            } else {
                set_pixel(screenBuf + x + (y * _screenSize.Width), cellCeilTexPtr, texelOffset, cellFogFactor);
            }
        }
    }
}

void raycaster::draw_sprites(level const& level, player const& player, f64 invFogDistance, i32 columnStart, i32 columnEnd)
{
    f64 const invDet {1.0 / player.Plane.cross(player.Direction)};
    i32 const screenCenterY {(_screenSize.Height / 2) + static_cast<i32>(player.BobAmount)};

    u32* screenBuf {_screen.data()};
    for (sprite const& spr : level.Sprites) {
        point_d const relPos {spr.Position - player.Position};

        f64 const transformX {invDet * relPos.cross(player.Direction)};
        f64 const transformY {invDet * player.Plane.cross(relPos)};
        if (transformY <= 0) { continue; }

        i32 const    spriteScreenX {static_cast<i32>((_screenSize.Width / 2.0) * (1.0 + (transformX / transformY)))};
        f64 const    scale {_projPlaneDist / transformY};
        size_i const spriteSize {static_cast<i32>(std::abs(scale)) * size_i {spr.Size}};

        i32 const yMinBound {(2 * screenCenterY) - _screenSize.Height + 1};
        i32 const yMaxBound {2 * screenCenterY};

        point_i const drawStart {std::max({(-spriteSize.Width / 2) + spriteScreenX, 0, columnStart}),
                                 std::max((-spriteSize.Height / 2) + screenCenterY, yMinBound)};
        point_i const drawEnd {std::min({(spriteSize.Width / 2) + spriteScreenX, _screenSize.Width - 1, columnEnd}),
                               std::min((spriteSize.Height / 2) + screenCenterY, yMaxBound + 1)};
        if (drawStart.X >= drawEnd.X) { continue; }
        if (drawStart.Y >= drawEnd.Y) { continue; }

        i32 const spriteLeft {spriteScreenX - (spriteSize.Width / 2)};
        i32 const spriteTop {(screenCenterY) - (spriteSize.Height / 2)};

        i32 const    facing {sprite_facing_index(spr.Facing, spr.Position, player.Position)};
        auto const*  tex {_cache.texture(spr.Texture, facing)};
        size_i const texSize {_cache.texture_size(spr.Texture, facing)};

        f64 const texStepY {1.0 * texSize.Height / spriteSize.Height};
        f64 const texPosYStart {(drawStart.Y - spriteTop) * texStepY};

        f64 spriteLight {0.0};
        std::visit([&](auto&& w) {
            if constexpr (requires { w.Light; }) { spriteLight = w.Light; }
        },
                   level.Map[point_i {spr.Position}]);

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

                i32 const mirroredY {(2 * screenCenterY) - y};
                if (mirroredY < 0 || mirroredY >= _screenSize.Height) { continue; }

                isize const depthIndex {stripe + (static_cast<isize>(mirroredY) * _screenSize.Width)};
                if (transformY < _spriteDepthBuffer[depthIndex]) {
                    _spriteDepthBuffer[depthIndex] = transformY;
                    set_pixel(screenBuf + stripe + (mirroredY * _screenSize.Width), tex, offset, spriteFogFactor);
                }
            }
        }
    }
}

void raycaster::draw_weapon(player const& player)
{
}

void raycaster::draw_hud(player const& player)
{
}
