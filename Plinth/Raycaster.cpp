// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Raycaster.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>
#include <tuple>
#include <utility>

#include "Common.hpp"
#include "Level.hpp"
#include "Player.hpp"
#include "TextureCache.hpp"
#include "Walls.hpp"

constexpr f64 EYE_HEIGHT {0.5}; // TODO: pull from player

static auto get_light(level const& level, point_i const& cell) -> f64
{
    return std::visit([](auto&& cell) -> f64 {
        if constexpr (requires { cell.Light; }) { return cell.Light; }
        return 0.0;
    },
                      level.get_cell(cell));
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

static void copy_pixel(u32* dst, i32 dstIdx, u8 const* src, i32 srcIdx, f64 darken)
{
    u8 const r {static_cast<u8>(std::min(src[srcIdx + 0] * darken, 255.0))};
    u8 const g {static_cast<u8>(std::min(src[srcIdx + 1] * darken, 255.0))};
    u8 const b {static_cast<u8>(std::min(src[srcIdx + 2] * darken, 255.0))};
    dst[dstIdx] = (0xFF000000u) | (static_cast<u32>(b) << 16) | (static_cast<u32>(g) << 8) | static_cast<u32>(r);
}

static auto compute_wall_screen_extent(f64 distance, i32 screenCenterY, f64 projPlaneDist) -> std::pair<i32, i32>
{
    f64 const lineHeight {projPlaneDist / distance};
    return {static_cast<i32>(std::round(screenCenterY - (lineHeight / 2.0))),
            static_cast<i32>(std::round(screenCenterY + (lineHeight / 2.0)))};
}

static auto voxel_light_from_heading(degree_f headingDegrees, degree_f azimuthOffset, degree_f elevation) -> vec3_d
{
    degree_f const azimuth {headingDegrees + azimuthOffset};
    return vec3_d {
        .X = azimuth.cos() * elevation.cos(),
        .Y = azimuth.sin() * elevation.cos(),
        .Z = elevation.sin(),
    }
        .normalized();
}

static auto voxel_face_normal(i32 axis, i32 sign) -> vec3_d
{
    vec3_d    n {};
    f64 const s {static_cast<f64>(sign)};
    switch (axis) {
    case 0:  n.X = s; break;
    case 1:  n.Y = s; break;
    default: n.Z = s; break;
    }
    return n;
}

////////////////////////////////////////////////////////////

raycaster::raycaster(texture_cache& cache, size_i screenSize, f64 projPlaneDist)
    : _cache {cache}
    , _screen(screenSize.area())
    , _screenSize {screenSize}
    , _projPlaneDist {projPlaneDist}
    , _taskManager {locate_service<task_manager>()}
{
    _zBuffer.resize(_screenSize.Width);
    _objectDepthBuffer.resize(_screenSize.area());
}

auto raycaster::draw(level& level, player const& player) -> u32 const*
{
    std::ranges::fill(_objectDepthBuffer, std::numeric_limits<f64>::infinity());
    f64 const invFogDistance {1.0 / level.Settings.FogDistance};

    _taskManager.run_parallel([&](par_task const& ctx) {
        draw_columns(level, player, invFogDistance, static_cast<i32>(ctx.Start), static_cast<i32>(ctx.End));
    },
                              _screenSize.Width);

    draw_sprites(level, player, invFogDistance);
    draw_voxel_objects(level, player, invFogDistance);

    draw_weapon(player);
    draw_hud(player);
    draw_message(level);

    return _screen.data();
}

void raycaster::draw_columns(level& level, player const& player, f64 invFogDistance, i32 columnStart, i32 columnEnd)
{
    for (isize x {columnStart}; x < columnEnd; x++) {
        f64 const     cameraX {(2.0 * x / _screenSize.Width) - 1.0};
        point_d const rayDir {player.Direction + (player.Plane * cameraX)};

        point_i       map {player.Position};
        point_d const deltaDist {(rayDir.X == 0) ? 1e30 : std::abs(1 / rayDir.X),
                                 (rayDir.Y == 0) ? 1e30 : std::abs(1 / rayDir.Y)};

        point_i step {rayDir.X < 0 ? -1 : 1, rayDir.Y < 0 ? -1 : 1};
        point_d sideDist {
            (rayDir.X < 0) ? (player.Position.X - map.X) * deltaDist.X : (map.X + 1.0 - player.Position.X) * deltaDist.X,
            (rayDir.Y < 0) ? (player.Position.Y - map.Y) * deltaDist.Y : (map.Y + 1.0 - player.Position.Y) * deltaDist.Y};

        wall_hit                                    hitResult {};
        std::array<wall_hit, MAX_TRANSPARENT_WALLS> transparentHits {};
        i32                                         transparentCount {0};

        auto const process_hit {[&](wall_hit const& wallHit, point_i const& cell) {
            wall_hit h {wallHit};
            h.Light = get_light(level, cell);
            if (h.Transparent) {
                if (transparentCount < MAX_TRANSPARENT_WALLS) { transparentHits[transparentCount++] = h; }
            } else {
                hitResult = h;
            }
        }};

        // check player cell
        {
            auto const intersect {[&](auto&& c) { return c.intersect({map, player.Position, rayDir, sideDist.X < sideDist.Y, 0.0}); }};
            auto const wallHit {std::visit(intersect, level.get_cell(map))};
            if (wallHit.Hit) { process_hit(wallHit, map); }
            level.mark_seen(map, player.Position);
        }

        // DDA
        if (!hitResult.Hit) {
            bool       side {false};
            auto const intersect {[&](auto&& c) -> wall_hit { return c.intersect({map, player.Position, rayDir, side, !side ? sideDist.X - deltaDist.X : sideDist.Y - deltaDist.Y}); }};

            while (map_t::Size.contains(map)) {
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

                level.mark_seen(map, player.Position);

                auto const wallHit {std::visit(intersect, level.get_cell(map))};
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
    auto const [wallTop, wallBottom] {compute_wall_screen_extent(hit.Distance, screenCenterY, _projPlaneDist)};

    i32 const drawStart {std::max(wallTop, 0)};
    i32 const drawEnd {std::min(wallBottom, _screenSize.Height)};

    if (drawStart >= drawEnd) { return; }

    auto const* tex {_cache.texture(hit.Texture, 0)};
    f64 const   texX {(1.0 - hit.SegmentT) * static_cast<f64>(WALL_SIZE.Width)};
    f64 const   texStep {1.0 * WALL_SIZE.Height / (wallBottom - wallTop)};
    f64         texPos {(drawStart - wallTop) * texStep};

    f64 const wallFogFactor {std::max(1.0 - (hit.Distance * invFogDistance), level.Settings.FogMin)};
    f64 const wallDarkenFactor {shade_from_side(hit.Side) * wallFogFactor * (level.Settings.AmbientLight + hit.Light)};

    u32* screenBuf {_screen.data()};
    for (i32 y {drawStart}; y < drawEnd; y++) {
        i32 const texY {((static_cast<i32>(texPos) % WALL_SIZE.Height) + WALL_SIZE.Height) % WALL_SIZE.Height};
        texPos += texStep;
        i32 const srcIdx {static_cast<i32>((texX + (texY * WALL_SIZE.Width))) * TEXTURE_BPP};

        if (transparent) {
            if (is_magenta(tex, srcIdx)) { continue; }
            isize const depthIndex {x + (static_cast<isize>(y) * _screenSize.Width)};
            _objectDepthBuffer[depthIndex] = std::min(_objectDepthBuffer[depthIndex], hit.Distance);
        }
        copy_pixel(screenBuf, x + (y * _screenSize.Width), tex, srcIdx, wallDarkenFactor);
    }
}

void raycaster::draw_floor_ceiling_column(wall_hit const& hit, level const& level, player const& player, isize x, point_d rayDir, f64 invFogDistance)
{
    i32 const screenCenterY {(_screenSize.Height / 2) + static_cast<i32>(player.BobAmount)};
    auto const [wallTop, wallBottom] {compute_wall_screen_extent(hit.Distance, screenCenterY, _projPlaneDist)};

    i32 const ceilingEnd {std::clamp(wallTop, 0, _screenSize.Height)};
    i32 const floorStart {std::clamp(wallBottom, 0, _screenSize.Height)};

    point_d const floorWall {player.Position + (rayDir * hit.Distance)};
    f64 const     invPerpWallDist {1.0 / hit.Distance};

    i32 const    fixedCenterY {_screenSize.Height / 2};
    size_i const texSize {_cache.texture_size(level.Settings.CeilingTexture, 0)};
    i32 const    skyTexX {level.Settings.IsSkybox ? static_cast<i32>(std::fmod((std::atan2(rayDir.Y, rayDir.X) / TAU) + 1.0, 1.0) * texSize.Width) % texSize.Width : 0};
    auto const*  skyTex {level.Settings.IsSkybox ? _cache.texture(level.Settings.CeilingTexture, 0) : nullptr};

    point_i     lastFloorCell {-1, -1};
    i32         cellFloorTex {level.Settings.FloorTexture};
    i32         cellCeilTex {level.Settings.CeilingTexture};
    f64         cellLight {0.0};
    auto const* cellFloorTexPtr {_cache.texture(cellFloorTex, 0)};
    auto const* cellCeilTexPtr {_cache.texture(cellCeilTex, 0)};

    auto const query_cell {[&](auto&& cell) {
        if constexpr (requires { cell.FloorTexture; }) {
            if (cell.FloorTexture != INVALID_INDEX) { cellFloorTex = cell.FloorTexture; }
        }
        if constexpr (requires { cell.CeilingTexture; }) {
            if (cell.CeilingTexture != INVALID_INDEX) { cellCeilTex = cell.CeilingTexture; }
        }
        if constexpr (requires { cell.Light; }) { cellLight = cell.Light; }
    }};

    u32* screenBuf {_screen.data()};

    auto const sample_and_draw {[&](i32 y, bool isFloor) {
        i32 const     effectiveY {isFloor ? y : (2 * screenCenterY) - y};
        f64 const     rowDist {effectiveY == screenCenterY ? std::numeric_limits<f64>::infinity()
                                                           : _projPlaneDist / ((2.0 * effectiveY) - (2.0 * screenCenterY))};
        f64 const     weight {std::min(rowDist * invPerpWallDist, 1.0)};
        point_d const currentFloor {(weight * floorWall.X) + ((1.0 - weight) * player.Position.X),
                                    (weight * floorWall.Y) + ((1.0 - weight) * player.Position.Y)};
        f64 const     floorDist {(currentFloor - player.Position).length()};

        f64 const fogFactor {std::max(1.0 - (floorDist * invFogDistance), level.Settings.FogMin)};

        i32 const texelX {static_cast<i32>(currentFloor.X * WALL_SIZE.Width) & (WALL_SIZE.Width - 1)};
        i32 const texelY {static_cast<i32>(currentFloor.Y * WALL_SIZE.Height) & (WALL_SIZE.Height - 1)};
        i32 const texelOffset {(texelX + (texelY * WALL_SIZE.Width)) * TEXTURE_BPP};

        point_i const floorCell {static_cast<i32>(currentFloor.X), static_cast<i32>(currentFloor.Y)};
        if (floorCell != lastFloorCell) {
            lastFloorCell = floorCell;
            cellFloorTex  = level.Settings.FloorTexture;
            cellCeilTex   = level.Settings.CeilingTexture;
            cellLight     = 0.0;
            if (map_t::Size.contains(floorCell)) {
                std::visit(query_cell, level.get_cell(floorCell));
            }
            cellFloorTexPtr = _cache.texture(cellFloorTex, 0);
            cellCeilTexPtr  = _cache.texture(cellCeilTex, 0);
        }

        f64 const cellFogFactor {fogFactor * (level.Settings.AmbientLight + cellLight)};

        if (isFloor) {
            copy_pixel(screenBuf, x + (y * _screenSize.Width), cellFloorTexPtr, texelOffset, cellFogFactor);
        } else if (level.Settings.IsSkybox) {
            i32 const skyTexY {static_cast<i32>(std::min(1.0 - (static_cast<f64>(y - fixedCenterY) / static_cast<f64>(_screenSize.Height - fixedCenterY)), 1.0) * texSize.Height) % texSize.Height};
            i32 const skyOffset {(skyTexX + (skyTexY * texSize.Width)) * TEXTURE_BPP};
            copy_pixel(screenBuf, x + (y * _screenSize.Width), skyTex, skyOffset, 1.0);
        } else {
            copy_pixel(screenBuf, x + (y * _screenSize.Width), cellCeilTexPtr, texelOffset, cellFogFactor);
        }
    }};

    for (i32 y {floorStart}; y < _screenSize.Height; y++) { sample_and_draw(y, true); }
    for (i32 y {0}; y < ceilingEnd; y++) { sample_and_draw(y, false); }
}

void raycaster::draw_sprites(level const& level, player const& player, f64 invFogDistance)
{
    static auto sprite_facing_index {[](degree_d spriteFacing, point_d spritePos, point_d cameraPos) -> i32 {
        auto const viewAngle {spritePos.angle_to(cameraPos)};
        auto const relativeAngle {(spriteFacing - viewAngle).as_normalized(angle_normalize::PositiveFullTurn)};

        constexpr f64 wedge {360.0 / NUM_FACINGS};
        return static_cast<i32>((relativeAngle.Value + (wedge / 2.0)) / wedge) % NUM_FACINGS;
    }};

    f64 const    invDet {1.0 / player.Plane.cross(player.Direction)};
    i32 const    screenCenterY {(_screenSize.Height / 2) + static_cast<i32>(player.BobAmount)};
    rect_i const screenRect {point_i {0, 0}, _screenSize};

    u32* screenBuf {_screen.data()};
    for (sprite const& spr : level.Sprites) {
        point_d const relPos {spr.Position - player.Position};

        f64 const transformX {invDet * relPos.cross(player.Direction)};
        f64 const transformY {invDet * player.Plane.cross(relPos)};
        if (transformY <= 0) { continue; }

        i32 const    spriteScreenX {static_cast<i32>((_screenSize.Width / 2.0) * (1.0 + (transformX / transformY)))};
        f64 const    scale {_projPlaneDist / transformY};
        i32 const    fullSpriteHeight {static_cast<i32>(std::abs(scale))};
        size_i const spriteSize {static_cast<size_i>(spr.Size * std::abs(scale))};

        i32 const spriteLeft {spriteScreenX - (spriteSize.Width / 2)};
        i32 const spriteTop {screenCenterY + (fullSpriteHeight / 2) - spriteSize.Height};

        rect_i const spriteRect {point_i {spriteLeft, spriteTop}, spriteSize};
        if (!screenRect.intersects(spriteRect, true)) { continue; }

        point_i const drawStart {std::max(spriteLeft, 0), std::max(spriteTop, 0)};
        point_i const drawEnd {std::min(spriteLeft + spriteSize.Width, _screenSize.Width),
                               std::min(spriteTop + spriteSize.Height, _screenSize.Height)};

        i32 const    facing {sprite_facing_index(spr.Facing, spr.Position, player.Position)};
        auto const*  tex {_cache.texture(spr.Texture, facing)};
        size_i const texSize {_cache.texture_size(spr.Texture, facing)};

        f64 const texStepY {1.0 * texSize.Height / spriteSize.Height};
        f64 const texPosYStart {(drawStart.Y - spriteTop) * texStepY};

        f64 const spriteLight {get_light(level, point_i {spr.Position})};
        f64 const spriteFogFactor {std::max(1.0 - (transformY * invFogDistance), level.Settings.FogMin) * (level.Settings.AmbientLight + spriteLight)};

        for (i32 stripe {drawStart.X}; stripe < drawEnd.X; ++stripe) {
            if (transformY >= _zBuffer[stripe]) { continue; }

            i32 const texX {std::clamp(((stripe - spriteLeft) * texSize.Width) / spriteSize.Width, 0, texSize.Width - 1)};
            f64       texPos {texPosYStart};

            for (i32 y {drawStart.Y}; y < drawEnd.Y; ++y) {
                i32 const texY {std::clamp(static_cast<i32>(texPos), 0, texSize.Height - 1)};
                texPos += texStepY;

                i32 const texOffset {(texX + (texY * texSize.Width)) * TEXTURE_BPP};
                if (is_magenta(tex, texOffset)) { continue; }

                isize const depthIndex {stripe + (static_cast<isize>(y) * _screenSize.Width)};
                if (transformY < _objectDepthBuffer[depthIndex]) {
                    _objectDepthBuffer[depthIndex] = transformY;
                    copy_pixel(screenBuf, stripe + (y * _screenSize.Width), tex, texOffset, spriteFogFactor);
                }
            }
        }
    }
}

void raycaster::draw_voxel_objects(level const& level, player const& player, f64 invFogDistance)
{
    static auto world_to_local {[](voxel_object const& obj, point_d worldXY, f64 worldZ, f64 cosYaw, f64 sinYaw) -> vec3_d {
        point_d const rel {worldXY - obj.Position};
        f64 const     localX {((rel.X * cosYaw) + (rel.Y * sinYaw)) / obj.Scale};
        f64 const     localY {((-rel.X * sinYaw) + (rel.Y * cosYaw)) / obj.Scale};
        return vec3_d {.X = localX + (obj.Grid->Size.X * 0.5),
                       .Y = localY + (obj.Grid->Size.Y * 0.5),
                       .Z = (worldZ - obj.BaseZ) / obj.Scale};
    }};

    static auto local_to_world {[](voxel_object const& obj, vec3_d local, f64 cosYaw, f64 sinYaw) -> std::pair<point_d, f64> {
        f64 const     cx {(local.X - (obj.Grid->Size.X * 0.5)) * obj.Scale};
        f64 const     cy {(local.Y - (obj.Grid->Size.Y * 0.5)) * obj.Scale};
        point_d const world {obj.Position.X + (cx * cosYaw) - (cy * sinYaw),
                             obj.Position.Y + (cx * sinYaw) + (cy * cosYaw)};
        return {world, obj.BaseZ + (local.Z * obj.Scale)};
    }};

    auto project {[&](i32 screenCenterY, point_d worldXY, f64 worldZ) -> std::tuple<f64, f64, f64> {
        f64 const     invDet {1.0 / player.Plane.cross(player.Direction)};
        point_d const relPos {worldXY - player.Position};

        f64 const transformX {invDet * relPos.cross(player.Direction)};
        f64 const transformY {invDet * player.Plane.cross(relPos)};

        if (transformY <= 1e-6) { return {0.0, 0.0, 0.0}; }

        f64 const screenX {(_screenSize.Width / 2.0) * (1.0 + (transformX / transformY))};
        f64 const scale {_projPlaneDist / transformY};
        f64 const screenY {screenCenterY - ((worldZ - EYE_HEIGHT) * scale)};

        return {screenX, screenY, transformY};
    }};

    i32 const    screenCenterY {(_screenSize.Height / 2) + static_cast<i32>(player.BobAmount)};
    rect_i const screenRect {point_i {0, 0}, _screenSize};
    u32*         screenBuf {_screen.data()};

    vec3_d const keyLight {voxel_light_from_heading(level.Settings.SunDirection, level.Settings.KeyAzimuthOffset, level.Settings.KeyElevation)};
    vec3_d const fillLight {voxel_light_from_heading(level.Settings.SunDirection, level.Settings.FillAzimuthOffset, level.Settings.FillElevation)};

    for (voxel_object const& obj : level.VoxelObjects) {
        if (!obj.Grid) { continue; }

        f64 const cosYaw {obj.Yaw.cos()};
        f64 const sinYaw {obj.Yaw.sin()};

        f64  bboxMinX {std::numeric_limits<f64>::infinity()}, bboxMaxX {-std::numeric_limits<f64>::infinity()};
        f64  bboxMinY {std::numeric_limits<f64>::infinity()}, bboxMaxY {-std::numeric_limits<f64>::infinity()};
        f64  bboxMinDepth {std::numeric_limits<f64>::infinity()};
        bool anyVisible {false};

        f64 const                   sizeX {static_cast<f64>(obj.Grid->Size.X)};
        f64 const                   sizeY {static_cast<f64>(obj.Grid->Size.Y)};
        f64 const                   sizeZ {static_cast<f64>(obj.Grid->Size.Z)};
        std::array<vec3_d, 8> const localCorners {{{.X = 0.0, .Y = 0.0, .Z = 0.0}, {.X = sizeX, .Y = 0.0, .Z = 0.0}, {.X = 0.0, .Y = sizeY, .Z = 0.0}, {.X = sizeX, .Y = sizeY, .Z = 0.0}, {.X = 0.0, .Y = 0.0, .Z = sizeZ}, {.X = sizeX, .Y = 0.0, .Z = sizeZ}, {.X = 0.0, .Y = sizeY, .Z = sizeZ}, {.X = sizeX, .Y = sizeY, .Z = sizeZ}}};
        for (vec3_d const& corner : localCorners) {
            auto const [worldXY, worldZ] {local_to_world(obj, corner, cosYaw, sinYaw)};
            auto const [sx, sy, depth] {project(screenCenterY, worldXY, worldZ)};
            if (depth <= 0.0) { continue; }

            anyVisible   = true;
            bboxMinX     = std::min(bboxMinX, sx);
            bboxMaxX     = std::max(bboxMaxX, sx);
            bboxMinY     = std::min(bboxMinY, sy);
            bboxMaxY     = std::max(bboxMaxY, sy);
            bboxMinDepth = std::min(bboxMinDepth, depth);
        }
        if (!anyVisible) { continue; }

        rect_i const bboxRect {point_i {static_cast<i32>(bboxMinX), static_cast<i32>(bboxMinY)},
                               size_i {static_cast<i32>(bboxMaxX - bboxMinX) + 1, static_cast<i32>(bboxMaxY - bboxMinY) + 1}};
        if (!screenRect.intersects(bboxRect, true)) { continue; }

        i32 const xStart {std::clamp(bboxRect.Position.X, 0, _screenSize.Width)};
        i32 const xEnd {std::clamp(bboxRect.Position.X + bboxRect.Size.Width, 0, _screenSize.Width)};
        i32 const yStart {std::clamp(bboxRect.Position.Y, 0, _screenSize.Height)};
        i32 const yEnd {std::clamp(bboxRect.Position.Y + bboxRect.Size.Height, 0, _screenSize.Height)};

        f64 const objWorldDiag {std::sqrt(static_cast<f64>((obj.Grid->Size.X * obj.Grid->Size.X)
                                                           + (obj.Grid->Size.Y * obj.Grid->Size.Y)
                                                           + (obj.Grid->Size.Z * obj.Grid->Size.Z)))
                                * obj.Scale};
        f64 const maxT {level.Settings.FogDistance + objWorldDiag};

        vec3_d const localOrigin {world_to_local(obj, player.Position, EYE_HEIGHT, cosYaw, sinYaw)};

        f64 maxWallDist {0.0};
        for (i32 x {xStart}; x < xEnd; ++x) { maxWallDist = std::max(maxWallDist, _zBuffer[x]); }
        if (bboxMinDepth >= maxWallDist) { continue; }

        constexpr i32 MAX_VOXEL_OBJECT_PIXELS {10000};
        i32 const     colCount {xEnd - xStart};
        i32 const     rowCount {yEnd - yStart};
        i32 const     bboxArea {colCount * rowCount};
        i32 const     stride {bboxArea > MAX_VOXEL_OBJECT_PIXELS ? static_cast<i32>(std::ceil(std::sqrt(static_cast<f64>(bboxArea) / MAX_VOXEL_OBJECT_PIXELS)))
                                                                 : 1};
        i32 const     strideCols {(colCount + stride - 1) / stride};
        f64 const     cellLight {get_light(level, point_i {static_cast<i32>(obj.Position.X), static_cast<i32>(obj.Position.Y)})};

        _taskManager.run_parallel([&](par_task const& ctx) {
            for (isize scol {static_cast<isize>(ctx.Start)}; scol < static_cast<isize>(ctx.End); ++scol) {
                i32 const x {xStart + (static_cast<i32>(scol) * stride)};
                if (x >= xEnd) { continue; }

                f64 const     cameraX {(2.0 * x / _screenSize.Width) - 1.0};
                point_d const rayDir2D {player.Direction + (player.Plane * cameraX)};
                i32 const     xBlockEnd {std::min(x + stride, xEnd)};

                for (i32 y {yStart}; y < yEnd; y += stride) {
                    f64 const    rayDirZ {(screenCenterY - y) / _projPlaneDist};
                    vec3_d const rayDir3D {vec3_d {.X = rayDir2D.X, .Y = rayDir2D.Y, .Z = rayDirZ}.normalized()};

                    vec3_d const localDir {.X = ((rayDir3D.X * cosYaw) + (rayDir3D.Y * sinYaw)) / obj.Scale,
                                           .Y = ((-rayDir3D.X * sinYaw) + (rayDir3D.Y * cosYaw)) / obj.Scale,
                                           .Z = rayDir3D.Z / obj.Scale};

                    auto const hit {obj.Grid->raycast(localOrigin, localDir, maxT)};
                    if (!hit.Hit) { continue; }

                    vec3_d const localHitPos {localOrigin + (localDir * hit.T)};
                    auto const [worldHitXY, worldHitZ] {local_to_world(obj, localHitPos, cosYaw, sinYaw)};
                    auto const [sx, sy, depth] {project(screenCenterY, worldHitXY, worldHitZ)};

                    if (depth <= 0.0) { continue; }

                    vec3_d const normal {voxel_face_normal(hit.FaceAxis, hit.FaceSign)};
                    f64 const    keyTerm {level.Settings.KeyDiffuse * std::max(0.0, normal.dot(keyLight))};
                    f64 const    fillTerm {level.Settings.FillDiffuse * std::max(0.0, normal.dot(fillLight))};
                    f64 const    ambientTerm {level.Settings.AmbientGround + ((level.Settings.AmbientSky - level.Settings.AmbientGround) * ((normal.Z * 0.5) + 0.5))};
                    f64 const    heightFactor {static_cast<f64>(hit.Cell.Z) / static_cast<f64>(std::max(1, obj.Grid->Size.Z - 1))};
                    f64 const    heightMultiplier {1.0 - level.Settings.HeightBandingStrength + (level.Settings.HeightBandingStrength * heightFactor)};

                    vec3_i const layer {.X = hit.Cell.X + (hit.FaceAxis == 0 ? hit.FaceSign : 0),
                                        .Y = hit.Cell.Y + (hit.FaceAxis == 1 ? hit.FaceSign : 0),
                                        .Z = hit.Cell.Z + (hit.FaceAxis == 2 ? hit.FaceSign : 0)};
                    i32 const    ao00 {obj.Grid->corner_ao(layer, hit.FaceAxis, -1, -1)};
                    i32 const    ao10 {obj.Grid->corner_ao(layer, hit.FaceAxis, +1, -1)};
                    i32 const    ao01 {obj.Grid->corner_ao(layer, hit.FaceAxis, -1, +1)};
                    i32 const    ao11 {obj.Grid->corner_ao(layer, hit.FaceAxis, +1, +1)};

                    f64 fu {}, fv {};
                    switch (hit.FaceAxis) {
                    case 0:
                        fu = localHitPos.Y - hit.Cell.Y;
                        fv = localHitPos.Z - hit.Cell.Z;
                        break;
                    case 1:
                        fu = localHitPos.X - hit.Cell.X;
                        fv = localHitPos.Z - hit.Cell.Z;
                        break;
                    default:
                        fu = localHitPos.X - hit.Cell.X;
                        fv = localHitPos.Y - hit.Cell.Y;
                        break;
                    }
                    fu = std::clamp(fu, 0.0, 1.0);
                    fv = std::clamp(fv, 0.0, 1.0);

                    f64 const aoInterp {(ao00 * (1.0 - fu) * (1.0 - fv)) + (ao10 * fu * (1.0 - fv)) + (ao01 * (1.0 - fu) * fv) + (ao11 * fu * fv)};
                    f64 const aoFactor {1.0 - (level.Settings.AmbientOcclusionStrength * (1.0 - (aoInterp / 3.0)))};

                    f64 const bakeShade {std::clamp((ambientTerm + keyTerm + fillTerm) * aoFactor * heightMultiplier, 0.0, 1.0)};

                    f64 const fogFactor {std::max(1.0 - (depth * invFogDistance), level.Settings.FogMin)};
                    f64 const shade {bakeShade * fogFactor * (level.Settings.AmbientLight + cellLight)};

                    u8 const  r {static_cast<u8>(std::min(hit.Color.R * shade, 255.0))};
                    u8 const  g {static_cast<u8>(std::min(hit.Color.G * shade, 255.0))};
                    u8 const  b {static_cast<u8>(std::min(hit.Color.B * shade, 255.0))};
                    u32 const packed {(0xFF000000u) | (static_cast<u32>(b) << 16) | (static_cast<u32>(g) << 8) | static_cast<u32>(r)};

                    i32 const yBlockEnd {std::min(y + stride, yEnd)};
                    for (i32 by {y}; by < yBlockEnd; ++by) {
                        for (i32 bx {x}; bx < xBlockEnd; ++bx) {
                            if (depth >= _zBuffer[bx]) { continue; }                   // occluded by a wall

                            isize const depthIndex {bx + (static_cast<isize>(by) * _screenSize.Width)};
                            if (depth >= _objectDepthBuffer[depthIndex]) { continue; } // occluded by a sprite/transparent wall

                            _objectDepthBuffer[depthIndex]           = depth;
                            screenBuf[bx + (by * _screenSize.Width)] = packed;
                        }
                    }
                }
            }
        },
                                  strideCols);
    }
}

void raycaster::draw_weapon(player const& player)
{
    auto* const tex {_cache.texture(handTexture, 0)};
    auto const  texSize {_cache.texture_size(handTexture, 0)};
    u32*        screenBuf {_screen.data()};

    f64 const     scale {_screenSize.Height / WEAPON_REFERENCE_HEIGHT};
    size_i const  drawSize {size_f {texSize} * scale};
    i32 const     bobOffsetY {static_cast<i32>(player.BobAmount * WEAPON_BOB_MULTIPLIER)};
    point_i const offset {static_cast<i32>((_screenSize.Width - drawSize.Width) * 0.75),
                          static_cast<i32>(_screenSize.Height - (drawSize.Height * 0.75)) + bobOffsetY};

    for (i32 y {0}; y < drawSize.Height; ++y) {
        i32 const screenY {y + offset.Y};
        i32 const texY {std::min(texSize.Height - 1, static_cast<i32>(y / scale))};
        for (i32 x {0}; x < drawSize.Width; ++x) {
            i32 const screenX {x + offset.X};
            if (!_screenSize.contains({screenX, screenY})) { continue; }

            i32 const texX {std::min(texSize.Width - 1, static_cast<i32>(x / scale))};
            i32 const texOffset {(texX + (texY * texSize.Width)) * TEXTURE_BPP};
            if (is_magenta(tex, texOffset)) { continue; }

            copy_pixel(screenBuf, screenX + (screenY * _screenSize.Width), tex, texOffset, 1.0);
        }
    }
}

void raycaster::draw_hud(player const&)
{
}

void raycaster::draw_message(level const& level)
{
    auto const& msg {level.get_message()};
    if (msg.empty()) { return; }

    auto* const tex {_cache.texture(fontTexture, 0)};
    auto const  texSize {_cache.texture_size(fontTexture, 0)};
    auto const  charSize {texSize.Height};
    u32*        screenBuf {_screen.data()};

    point_i const offset {5, 5};
    rect_i const  screenRect {point_i {0, 0}, _screenSize};

    for (i32 i {0}; i < std::ssize(msg); ++i) {
        char const c {msg[i]};
        if (std::isspace(static_cast<unsigned char>(c))) { continue; }

        i32 index {0};
        if (std::isdigit(static_cast<unsigned char>(c))) {
            index = c - '0';
        } else if (std::isalpha(static_cast<unsigned char>(c))) {
            index = 10 + (std::tolower(static_cast<unsigned char>(c)) - 'a');
        }

        for (i32 y {0}; y < charSize; ++y) {
            i32 const screenY {y + offset.Y};
            for (i32 x {0}; x < charSize; ++x) {
                i32 const screenX {x + offset.X + (i * (charSize + 1))};
                if (!screenRect.contains(point_i {screenX, screenY}, false)) { continue; }

                i32 const texX {static_cast<i32>(index * charSize) + x};
                i32 const texOffset {(texX + (y * texSize.Width)) * TEXTURE_BPP};
                if (is_magenta(tex, texOffset)) { continue; }

                copy_pixel(screenBuf, screenX + (screenY * _screenSize.Width), tex, texOffset, 1.0);
            }
        }
    }
}
