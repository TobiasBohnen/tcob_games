// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Raycaster.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>
#include <numbers>
#include <tuple>
#include <utility>

#include "Common.hpp"
#include "Level.hpp"
#include "Player.hpp"
#include "TextureCache.hpp"
#include "Walls.hpp"

constexpr f64 EYE_HEIGHT {0.5};                             // TODO: pull from player
constexpr f64 WALL_LIGHT_Z {0.5};                           // approximate wall column as a flat plane at mid-height for lighting
constexpr f64 SEEN_LIGHT_THRESHOLD {0.05};                  // minimum player-light strength for a cell to count as "seen"
constexpr f64 CELL_DIAGONAL_HALF {std::numbers::sqrt2 / 2}; // max distance from a cell's center to its farthest corner

static auto PixelIndex(size_i screenSize, isize x, isize y) -> isize
{
    return x + (y * screenSize.Width);
}

static auto IsMagenta(u8 const* tex, i32 offset) -> bool
{
    return tex[offset + 0] == 0x98 && tex[offset + 1] == 0x00 && tex[offset + 2] == 0x88;
}

static void CopyPixel(u32* dst, isize dstIdx, u8 const* src, isize srcIdx, vec3_d tint)
{
    u8 const r {static_cast<u8>(std::min(src[srcIdx + 0] * tint.X, 255.0))};
    u8 const g {static_cast<u8>(std::min(src[srcIdx + 1] * tint.Y, 255.0))};
    u8 const b {static_cast<u8>(std::min(src[srcIdx + 2] * tint.Z, 255.0))};
    dst[dstIdx] = (0xFF000000u) | (static_cast<u32>(b) << 16) | (static_cast<u32>(g) << 8) | static_cast<u32>(r);
}

static auto ComputeWallScreenExtent(f64 distance, i32 screenCenterY, f64 projPlaneDist) -> std::pair<i32, i32>
{
    f64 const lineHeight {projPlaneDist / distance};
    return {static_cast<i32>(std::round(screenCenterY - (lineHeight / 2.0))),
            static_cast<i32>(std::round(screenCenterY + (lineHeight / 2.0)))};
}

static auto ComputeCameraRayDir(isize x, i32 screenWidth, player const& player) -> point_d
{
    f64 const cameraX {(2.0 * x / screenWidth) - 1.0};
    return player.Direction + (player.Plane * cameraX);
}

static auto ComputeFalloff(f64 distSq, f64 rangeSq, f64 intensity) -> f64
{
    f64 const d {std::max(distSq, 0.01)};
    f64 const ratio {distSq / rangeSq};
    f64 const windowed {std::pow(std::clamp(1.0 - (ratio * ratio), 0.0, 1.0), 2.0)};
    return intensity * windowed / d;
}

static auto ToneMap(f64 x) -> f64
{
    return x / (1.0 + x);
}

static void AddLightContribution(point_d const& lightPos, f64 lightHeight, f64 range, f64 intensity, color lightColor,
                                 point_d const& surfacePos, f64 surfaceZ, vec3_d& total)
{
    point_d const toLightXY {lightPos - surfacePos};
    f64 const     dz {lightHeight - surfaceZ};
    f64 const     distSq {toLightXY.dot(toLightXY) + (dz * dz)};
    f64 const     rangeSq {range * range};
    if (distSq >= rangeSq) { return; }
    f64 const strength {ComputeFalloff(distSq, rangeSq, intensity)};
    total.X += (lightColor.R / 255.0) * strength;
    total.Y += (lightColor.G / 255.0) * strength;
    total.Z += (lightColor.B / 255.0) * strength;
}

static auto WorldToCell(point_d const& p) -> point_i
{
    return {static_cast<i32>(std::floor(p.X)), static_cast<i32>(std::floor(p.Y))};
}

static auto HasLineOfSight(level const& level, point_d const& from, point_d const& to) -> bool
{
    point_d const diff {to - from};
    f64 const     dist {diff.length()};
    if (dist < 1e-6) { return true; }

    point_d const rayDir {diff.X / dist, diff.Y / dist};
    point_i       map {static_cast<i32>(from.X), static_cast<i32>(from.Y)};

    point_d const deltaDist {(rayDir.X == 0) ? 1e30 : std::abs(1 / rayDir.X),
                             (rayDir.Y == 0) ? 1e30 : std::abs(1 / rayDir.Y)};
    point_i       step {rayDir.X < 0 ? -1 : 1, rayDir.Y < 0 ? -1 : 1};
    point_d       sideDist {
        (rayDir.X < 0) ? (from.X - map.X) * deltaDist.X : (map.X + 1.0 - from.X) * deltaDist.X,
        (rayDir.Y < 0) ? (from.Y - map.Y) * deltaDist.Y : (map.Y + 1.0 - from.Y) * deltaDist.Y};

    while (map_t::Size.contains(map)) {
        bool const stepX {sideDist.X < sideDist.Y};
        f64 const  traveled {stepX ? sideDist.X : sideDist.Y};
        if (traveled >= dist) { return true; } // reached the target cell before hitting anything

        if (stepX) {
            sideDist.X += deltaDist.X;
            map.X += step.X;
        } else {
            sideDist.Y += deltaDist.Y;
            map.Y += step.Y;
        }

        if (!map_t::Size.contains(map)) { break; }

        bool const blocked {std::visit([](auto&& c) -> bool {
            using T = std::decay_t<decltype(c)>;
            if constexpr (std::is_same_v<T, floor_cell>) {
                return false;
            } else if constexpr (requires { c.State; }) {
                return c.State != wall_state::Open;
            } // doors/push walls: open = passable
            else {
                return true;
            } // normal_wall, obstacle, diagonal_wall: always solid
        },
                                       level.get_cell(map))};
        if (blocked) { return false; }
    }
    return true;
}

static auto IsWithinPlayerLight(player const& player, point_i const& cell) -> bool
{
    point_d const cellCenter {cell.X + 0.5, cell.Y + 0.5};
    point_d const toCell {cellCenter - player.Position};
    f64 const     distSq {toCell.dot(toCell)};
    f64 const     rangeSq {player.Settings.LightRange * player.Settings.LightRange};
    if (distSq >= rangeSq) { return false; }
    f64 const strength {ComputeFalloff(distSq, rangeSq, player.Settings.LightIntensity)};
    return strength >= SEEN_LIGHT_THRESHOLD;
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
    _cellLights.resize(static_cast<usize>(MAP_WIDTH) * MAP_HEIGHT);
}

void raycaster::precompute_light_visibility(level const& level)
{
    for (auto& list : _cellLights) { list.clear(); }

    for (u32 li {0}; li < static_cast<u32>(level.DynamicLights.size()); ++li) {
        dynamic_light const& light {level.DynamicLights[li]};
        f64 const            sweepRadius {light.Range + CELL_DIAGONAL_HALF};

        i32 const cxMin {std::clamp(static_cast<i32>(std::floor(light.Position.X - sweepRadius)), 0, MAP_WIDTH - 1)};
        i32 const cxMax {std::clamp(static_cast<i32>(std::floor(light.Position.X + sweepRadius)), 0, MAP_WIDTH - 1)};
        i32 const cyMin {std::clamp(static_cast<i32>(std::floor(light.Position.Y - sweepRadius)), 0, MAP_HEIGHT - 1)};
        i32 const cyMax {std::clamp(static_cast<i32>(std::floor(light.Position.Y + sweepRadius)), 0, MAP_HEIGHT - 1)};

        for (i32 cy {cyMin}; cy <= cyMax; ++cy) {
            for (i32 cx {cxMin}; cx <= cxMax; ++cx) {
                point_d const cellCenter {cx + 0.5, cy + 0.5};
                point_d const toLightXY {light.Position - cellCenter};
                f64 const     dz {light.Height - WALL_LIGHT_Z};
                f64 const     distSq {toLightXY.dot(toLightXY) + (dz * dz)};
                if (distSq >= sweepRadius * sweepRadius) { continue; }

                if (HasLineOfSight(level, cellCenter, light.Position)) {
                    usize const cellIndex {static_cast<usize>((cy * MAP_WIDTH) + cx)};
                    _cellLights[cellIndex].push_back(li);
                }
            }
        }
    }
}

auto raycaster::accumulate_light(level const& level, player const& player, point_d const& surfacePos, f64 surfaceZ, point_i const& cell) const -> vec3_d
{
    vec3_d total {};

    AddLightContribution(player.Position, EYE_HEIGHT, player.Settings.LightRange, player.Settings.LightIntensity, color {255, 255, 255}, surfacePos, surfaceZ, total);

    if (map_t::Size.contains(cell)) {
        usize const cellIndex {static_cast<usize>((cell.Y * MAP_WIDTH) + cell.X)};
        for (u32 li : _cellLights[cellIndex]) {
            dynamic_light const& light {level.DynamicLights[li]};
            AddLightContribution(light.Position, light.Height, light.Range, light.Intensity, light.Color, surfacePos, surfaceZ, total);
        }
    }

    return vec3_d {.X = ToneMap(total.X), .Y = ToneMap(total.Y), .Z = ToneMap(total.Z)};
}

void raycaster::shade_and_write(u32* screenBuf, isize dstIdx, u8 const* tex, isize srcIdx,
                                level const& level, player const& player, point_d const& surfacePos, f64 surfaceZ) const
{
    point_i const cell {WorldToCell(surfacePos)};
    vec3_d const  tint {accumulate_light(level, player, surfacePos, surfaceZ, cell)};
    CopyPixel(screenBuf, dstIdx, tex, srcIdx, tint);
}

auto raycaster::draw(level& level, player const& player) -> u32 const*
{
    std::ranges::fill(_objectDepthBuffer, std::numeric_limits<f64>::infinity());

    precompute_light_visibility(level); // must complete before draw_columns' parallel dispatch reads it

    i32 const screenCenterY {(_screenSize.Height / 2) + static_cast<i32>(player.BobAmount)};

    _taskManager.run_parallel([&](par_task const& ctx) {
        draw_columns(level, player, screenCenterY, static_cast<i32>(ctx.Start), static_cast<i32>(ctx.End));
    },
                              _screenSize.Width);

    draw_sprites(level, player, screenCenterY);
    draw_voxel_objects(level, player, screenCenterY);

    draw_weapon(player);

    draw_screen_effect(level, player);

    draw_hud(player);
    draw_message(level);

    return _screen.data();
}

void raycaster::draw_columns(level& level, player const& player, i32 screenCenterY, i32 columnStart, i32 columnEnd)
{
    for (isize x {columnStart}; x < columnEnd; x++) {
        point_d const rayDir {ComputeCameraRayDir(x, _screenSize.Width, player)};

        point_i       map {player.Position};
        point_d const deltaDist {(rayDir.X == 0) ? 1e30 : std::abs(1 / rayDir.X),
                                 (rayDir.Y == 0) ? 1e30 : std::abs(1 / rayDir.Y)};

        point_i step {rayDir.X < 0 ? -1 : 1, rayDir.Y < 0 ? -1 : 1};
        point_d sideDist {
            (rayDir.X < 0) ? (player.Position.X - map.X) * deltaDist.X : (map.X + 1.0 - player.Position.X) * deltaDist.X,
            (rayDir.Y < 0) ? (player.Position.Y - map.Y) * deltaDist.Y : (map.Y + 1.0 - player.Position.Y) * deltaDist.Y};

        wall_hit hitResult {};
        point_i  hitCell {};

        // check player cell
        {
            auto const intersect {[&](auto&& c) { return c.intersect({map, player.Position, rayDir, sideDist.X < sideDist.Y, 0.0}); }};
            auto const wallHit {std::visit(intersect, level.get_cell(map))};
            if (wallHit.Hit) {
                hitResult = wallHit;
                hitCell   = map;
            }
            if (IsWithinPlayerLight(player, map)) { level.mark_seen(map, player.Position); }
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

                if (IsWithinPlayerLight(player, map)) { level.mark_seen(map, player.Position); }

                auto const wallHit {std::visit(intersect, level.get_cell(map))};
                if (wallHit.Hit) {
                    hitResult = wallHit;
                    hitCell   = map;
                    break;
                }
            }
        }

        if (!hitResult.Hit) { continue; }

        _zBuffer[x] = hitResult.Distance;

        auto const [wallTop, wallBottom] {ComputeWallScreenExtent(hitResult.Distance, screenCenterY, _projPlaneDist)};
        wall_extent const extent {.Top = wallTop, .Bottom = wallBottom, .ScreenCenterY = screenCenterY};

        draw_floor_ceiling_column(hitResult, level, player, x, rayDir, extent);

        draw_wall_column(hitResult, level, player, x, rayDir, hitCell, extent);
    }
}

void raycaster::draw_wall_column(wall_hit const& hit, level const& level, player const& player, isize x, point_d rayDir, point_i cell, wall_extent const& extent)
{
    i32 const drawStart {std::max(extent.Top, 0)};
    i32 const drawEnd {std::min(extent.Bottom, _screenSize.Height)};

    if (drawStart >= drawEnd) { return; }

    auto const* tex {_cache.texture(hit.Texture, 0)};
    i32 const   texX {std::clamp(static_cast<i32>((1.0 - hit.SegmentT) * static_cast<f64>(WALL_SIZE.Width)), 0, WALL_SIZE.Width - 1)};
    f64 const   texStep {1.0 * WALL_SIZE.Height / (extent.Bottom - extent.Top)};
    f64         texPos {(drawStart - extent.Top) * texStep};

    point_d const surfacePos {player.Position + (rayDir * hit.Distance)};
    vec3_d const  tint {accumulate_light(level, player, surfacePos, WALL_LIGHT_Z, cell)};

    u32* screenBuf {_screen.data()};
    for (i32 y {drawStart}; y < drawEnd; y++) {
        i32 const texY {((static_cast<i32>(texPos) % WALL_SIZE.Height) + WALL_SIZE.Height) % WALL_SIZE.Height};
        texPos += texStep;
        i32 const srcIdx {(texX + (texY * WALL_SIZE.Width)) * TEXTURE_BPP};

        CopyPixel(screenBuf, PixelIndex(_screenSize, x, y), tex, srcIdx, tint);
    }
}

void raycaster::draw_floor_ceiling_column(wall_hit const& hit, level const& level, player const& player, isize x, point_d rayDir, wall_extent const& extent)
{
    i32 const ceilingEnd {std::clamp(extent.Top, 0, _screenSize.Height)};
    i32 const floorStart {std::clamp(extent.Bottom, 0, _screenSize.Height)};

    point_d const floorWall {player.Position + (rayDir * hit.Distance)};
    f64 const     invPerpWallDist {1.0 / hit.Distance};

    point_i     lastFloorCell {-1, -1};
    i32         cellFloorTex {level.Settings.FloorTexture};
    i32         cellCeilTex {level.Settings.CeilingTexture};
    auto const* cellFloorTexPtr {_cache.texture(cellFloorTex, 0)};
    auto const* cellCeilTexPtr {_cache.texture(cellCeilTex, 0)};

    auto const query_cell {[&](auto&& cell) {
        if constexpr (requires { cell.FloorTexture; }) {
            if (cell.FloorTexture != INVALID_INDEX) { cellFloorTex = cell.FloorTexture; }
        }
        if constexpr (requires { cell.CeilingTexture; }) {
            if (cell.CeilingTexture != INVALID_INDEX) { cellCeilTex = cell.CeilingTexture; }
        }
    }};

    u32* screenBuf {_screen.data()};

    auto const sample_and_draw {[&](i32 y, bool isFloor) {
        i32 const     effectiveY {isFloor ? y : (2 * extent.ScreenCenterY) - y};
        f64 const     rowDist {effectiveY == extent.ScreenCenterY ? std::numeric_limits<f64>::infinity()
                                                                  : _projPlaneDist / ((2.0 * effectiveY) - (2.0 * extent.ScreenCenterY))};
        f64 const     weight {std::min(rowDist * invPerpWallDist, 1.0)};
        point_d const currentFloor {(weight * floorWall.X) + ((1.0 - weight) * player.Position.X),
                                    (weight * floorWall.Y) + ((1.0 - weight) * player.Position.Y)};

        i32 const texelX {static_cast<i32>(currentFloor.X * WALL_SIZE.Width) & (WALL_SIZE.Width - 1)};
        i32 const texelY {static_cast<i32>(currentFloor.Y * WALL_SIZE.Height) & (WALL_SIZE.Height - 1)};
        i32 const texelOffset {(texelX + (texelY * WALL_SIZE.Width)) * TEXTURE_BPP};

        point_i const floorCell {WorldToCell(currentFloor)};
        if (floorCell != lastFloorCell) {
            lastFloorCell = floorCell;
            cellFloorTex  = level.Settings.FloorTexture;
            cellCeilTex   = level.Settings.CeilingTexture;
            if (map_t::Size.contains(floorCell)) {
                std::visit(query_cell, level.get_cell(floorCell));
            }
            cellFloorTexPtr = _cache.texture(cellFloorTex, 0);
            cellCeilTexPtr  = _cache.texture(cellCeilTex, 0);
        }

        if (isFloor) {
            shade_and_write(screenBuf, PixelIndex(_screenSize, x, y), cellFloorTexPtr, texelOffset, level, player, currentFloor, 0.0);
        } else {
            shade_and_write(screenBuf, PixelIndex(_screenSize, x, y), cellCeilTexPtr, texelOffset, level, player, currentFloor, 1.0);
        }
    }};

    for (i32 y {floorStart}; y < _screenSize.Height; y++) { sample_and_draw(y, true); }
    for (i32 y {0}; y < ceilingEnd; y++) { sample_and_draw(y, false); }
}

void raycaster::draw_sprites(level const& level, player const& player, i32 screenCenterY)
{
    static auto sprite_facing_index {[](degree_d spriteFacing, point_d spritePos, point_d cameraPos) -> i32 {
        auto const viewAngle {spritePos.angle_to(cameraPos)};
        auto const relativeAngle {(spriteFacing - viewAngle).as_normalized(angle_normalize::PositiveFullTurn)};

        constexpr f64 wedge {360.0 / NUM_FACINGS};
        return static_cast<i32>((relativeAngle.Value + (wedge / 2.0)) / wedge) % NUM_FACINGS;
    }};

    f64 const    invDet {1.0 / player.Plane.cross(player.Direction)};
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

        point_i const spriteCell {WorldToCell(spr.Position)};
        vec3_d const  spriteTint {accumulate_light(level, player, spr.Position, 0.0, spriteCell)};

        for (i32 x {drawStart.X}; x < drawEnd.X; ++x) {
            if (transformY >= _zBuffer[x]) { continue; }

            i32 const texX {std::clamp(((x - spriteLeft) * texSize.Width) / spriteSize.Width, 0, texSize.Width - 1)};
            f64       texPos {texPosYStart};

            for (i32 y {drawStart.Y}; y < drawEnd.Y; ++y) {
                i32 const texY {std::clamp(static_cast<i32>(texPos), 0, texSize.Height - 1)};
                texPos += texStepY;

                i32 const texOffset {(texX + (texY * texSize.Width)) * TEXTURE_BPP};
                if (IsMagenta(tex, texOffset)) { continue; }

                isize const depthIndex {PixelIndex(_screenSize, x, y)};
                if (transformY < _objectDepthBuffer[depthIndex]) {
                    _objectDepthBuffer[depthIndex] = transformY;
                    CopyPixel(screenBuf, PixelIndex(_screenSize, x, y), tex, texOffset, spriteTint);
                }
            }
        }
    }
}

void raycaster::draw_voxel_objects(level const& level, player const& player, i32 screenCenterY)
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

    f64 const  invDet {1.0 / player.Plane.cross(player.Direction)};
    auto const project {[&](i32 screenCenterY, point_d worldXY, f64 worldZ) -> std::tuple<f64, f64, f64> {
        point_d const relPos {worldXY - player.Position};

        f64 const transformX {invDet * relPos.cross(player.Direction)};
        f64 const transformY {invDet * player.Plane.cross(relPos)};

        if (transformY <= 1e-6) { return {0.0, 0.0, 0.0}; }

        f64 const screenX {(_screenSize.Width / 2.0) * (1.0 + (transformX / transformY))};
        f64 const scale {_projPlaneDist / transformY};
        f64 const screenY {screenCenterY - ((worldZ - EYE_HEIGHT) * scale)};

        return {screenX, screenY, transformY};
    }};

    rect_i const screenRect {point_i {0, 0}, _screenSize};
    u32*         screenBuf {_screen.data()};

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

        vec3_d const localOrigin {world_to_local(obj, player.Position, EYE_HEIGHT, cosYaw, sinYaw)};

        f64 maxWallDist {0.0};
        for (i32 x {xStart}; x < xEnd; ++x) { maxWallDist = std::max(maxWallDist, _zBuffer[x]); }
        if (bboxMinDepth >= maxWallDist) { continue; }

        f64 const objWorldDiag {std::sqrt(static_cast<f64>((obj.Grid->Size.X * obj.Grid->Size.X)
                                                           + (obj.Grid->Size.Y * obj.Grid->Size.Y)
                                                           + (obj.Grid->Size.Z * obj.Grid->Size.Z)))
                                * obj.Scale};
        f64 const maxT {maxWallDist + objWorldDiag};

        constexpr i32 MAX_VOXEL_OBJECT_PIXELS {20000};
        i32 const     colCount {xEnd - xStart};
        i32 const     rowCount {yEnd - yStart};
        i32 const     bboxArea {colCount * rowCount};
        i32 const     stride {bboxArea > MAX_VOXEL_OBJECT_PIXELS ? static_cast<i32>(std::ceil(std::sqrt(static_cast<f64>(bboxArea) / MAX_VOXEL_OBJECT_PIXELS)))
                                                                 : 1};
        i32 const     strideCols {(colCount + stride - 1) / stride};

        _taskManager.run_parallel([&](par_task const& ctx) {
            for (isize scol {static_cast<isize>(ctx.Start)}; scol < static_cast<isize>(ctx.End); ++scol) {
                i32 const x {xStart + (static_cast<i32>(scol) * stride)};
                if (x >= xEnd) { continue; }

                point_d const rayDir2D {ComputeCameraRayDir(x, _screenSize.Width, player)};
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

                    point_i const hitCellForLight {WorldToCell(worldHitXY)};
                    vec3_i const  layer {.X = hit.Cell.X + (hit.FaceAxis == 0 ? hit.FaceSign : 0),
                                         .Y = hit.Cell.Y + (hit.FaceAxis == 1 ? hit.FaceSign : 0),
                                         .Z = hit.Cell.Z + (hit.FaceAxis == 2 ? hit.FaceSign : 0)};
                    i32 const     ao00 {obj.Grid->corner_ao(layer, hit.FaceAxis, -1, -1)};
                    i32 const     ao10 {obj.Grid->corner_ao(layer, hit.FaceAxis, +1, -1)};
                    i32 const     ao01 {obj.Grid->corner_ao(layer, hit.FaceAxis, -1, +1)};
                    i32 const     ao11 {obj.Grid->corner_ao(layer, hit.FaceAxis, +1, +1)};

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
                    f64 const aoFactor {aoInterp / 3.0};

                    vec3_d tint {accumulate_light(level, player, worldHitXY, worldHitZ, hitCellForLight)};
                    tint.X *= aoFactor;
                    tint.Y *= aoFactor;
                    tint.Z *= aoFactor;

                    u8 const  r {static_cast<u8>(std::min(hit.Color.R * tint.X, 255.0))};
                    u8 const  g {static_cast<u8>(std::min(hit.Color.G * tint.Y, 255.0))};
                    u8 const  b {static_cast<u8>(std::min(hit.Color.B * tint.Z, 255.0))};
                    u32 const packed {(0xFF000000u) | (static_cast<u32>(b) << 16) | (static_cast<u32>(g) << 8) | static_cast<u32>(r)};

                    i32 const yBlockEnd {std::min(y + stride, yEnd)};
                    for (i32 by {y}; by < yBlockEnd; ++by) {
                        for (i32 bx {x}; bx < xBlockEnd; ++bx) {
                            if (depth >= _zBuffer[bx]) { continue; }

                            isize const depthIndex {PixelIndex(_screenSize, bx, by)};
                            if (depth >= _objectDepthBuffer[depthIndex]) { continue; }

                            _objectDepthBuffer[depthIndex] = depth;
                            screenBuf[depthIndex]          = packed;
                        }
                    }
                }
            }
        },
                                  strideCols);
    }
}

void raycaster::draw_screen_effect(level const& level, player const& player)
{
    if (player.Cheater) {
        for (auto& col : _screen) {
            color c {color::FromABGR(col)};
            std::swap(c.R, c.B);
            col = (0xFF000000u) | (static_cast<u32>(c.B) << 16) | (static_cast<u32>(c.G) << 8) | static_cast<u32>(c.R);
        }
    }
}

void raycaster::draw_weapon(player const& player)
{
    auto* const tex {_cache.texture(handTexture, 0)};
    auto const  texSize {_cache.texture_size(handTexture, 0)};
    u32*        screenBuf {_screen.data()};

    f64 const     scale {_screenSize.Height / REFERENCE_HEIGHT};
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
            if (IsMagenta(tex, texOffset)) { continue; }

            CopyPixel(screenBuf, PixelIndex(_screenSize, screenX, screenY), tex, texOffset, vec3_d {.X = 1.0, .Y = 1.0, .Z = 1.0});
        }
    }
}

void raycaster::draw_hud(player const&)
{
    auto* const tex {_cache.texture(hudTexture, 0)};
    auto const  texSize {_cache.texture_size(hudTexture, 0)};
    u32*        screenBuf {_screen.data()};

    f64 const     scale {_screenSize.Height / REFERENCE_HEIGHT};
    size_i const  drawSize {size_f {texSize} * scale};
    point_i const offset {0, 0};

    for (i32 y {0}; y < drawSize.Height; ++y) {
        i32 const screenY {y + offset.Y};
        i32 const texY {std::min(texSize.Height - 1, static_cast<i32>(y / scale))};
        for (i32 x {0}; x < drawSize.Width; ++x) {
            i32 const screenX {x + offset.X};
            if (!_screenSize.contains({screenX, screenY})) { continue; }

            i32 const texX {std::min(texSize.Width - 1, static_cast<i32>(x / scale))};
            i32 const texOffset {(texX + (texY * texSize.Width)) * TEXTURE_BPP};
            if (IsMagenta(tex, texOffset)) { continue; }

            CopyPixel(screenBuf, PixelIndex(_screenSize, screenX, screenY), tex, texOffset, vec3_d {.X = 1.0, .Y = 1.0, .Z = 1.0});
        }
    }
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
                if (IsMagenta(tex, texOffset)) { continue; }

                CopyPixel(screenBuf, PixelIndex(_screenSize, screenX, screenY), tex, texOffset, vec3_d {.X = 1.0, .Y = 1.0, .Z = 1.0});
            }
        }
    }
}
