// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Raycaster.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <limits>
#include <numbers>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "Common.hpp"
#include "Level.hpp"
#include "Light.hpp"
#include "Player.hpp"
#include "TextureCache.hpp"
#include "Voxel.hpp"
#include "Walls.hpp"

constexpr f64 EYE_HEIGHT {0.5};                             // TODO: pull from player
constexpr f64 WALL_LIGHT_Z {0.5};                           // approximate wall column as a flat plane at mid-height for lighting
constexpr f64 SEEN_LIGHT_THRESHOLD {0.05};                  // minimum player-light strength for a cell to count as "seen"
constexpr f64 CELL_DIAGONAL_HALF {std::numbers::sqrt2 / 2}; // max distance from a cell's center to its farthest corner
constexpr f64 INFINITE_DIST {1e30};                         // stand-in for "ray never crosses this axis"
constexpr f64 INV_255 {1.0 / 255.0};
constexpr f64 CONTACT_SHADOW_WIDTH {0.3};
constexpr f64 CONTACT_SHADOW_STRENGTH {0.75};
constexpr f64 DITHERING_LIGHT_BANDS {8};

constexpr std::array<f64, 16> BAYER_4X4 {0.03125, 0.53125, 0.15625, 0.65625,
                                         0.78125, 0.28125, 0.90625, 0.40625,
                                         0.21875, 0.71875, 0.09375, 0.59375,
                                         0.96875, 0.46875, 0.84375, 0.34375};

static auto PixelIndex(size_i screenSize, isize x, isize y) -> isize { return x + (y * screenSize.Width); }
static auto IsMagenta(u8 const* tex, i32 offset) -> bool { return tex[offset + 0] == 0x98 && tex[offset + 1] == 0x00 && tex[offset + 2] == 0x88; }

static auto ApplyTint(u8 r, u8 g, u8 b, color_d tint) -> u32
{
    u8 const tr {static_cast<u8>(std::min(r * tint.R, 255.0))};
    u8 const tg {static_cast<u8>(std::min(g * tint.G, 255.0))};
    u8 const tb {static_cast<u8>(std::min(b * tint.B, 255.0))};
    return 0xFF000000u | (static_cast<u32>(tb) << 16) | (static_cast<u32>(tg) << 8) | static_cast<u32>(tr);
}

static void CopyPixel(u32* dst, isize dstIdx, u8 const* src, isize srcIdx, color_d tint) { dst[dstIdx] = ApplyTint(src[srcIdx + 0], src[srcIdx + 1], src[srcIdx + 2], tint); }

static auto BlendPixel(u32 a, u32 b, f64 t) -> u32
{
    u32 const w {static_cast<u32>(std::clamp(t, 0.0, 1.0) * 256.0)};
    u32 const inv {256 - w};
    u32 const rb {((((a & 0x00FF00FFu) * inv) + ((b & 0x00FF00FFu) * w)) >> 8) & 0x00FF00FFu};
    u32 const g {((((a & 0x0000FF00u) * inv) + ((b & 0x0000FF00u) * w)) >> 8) & 0x0000FF00u};
    return 0xFF000000u | rb | g;
}

static auto ScalePixel(u32 px, f64 factor) -> u32
{
    u32 const m {static_cast<u32>(std::clamp(factor, 0.0, 1.0) * 256.0)};
    u32 const rb {(((px & 0x00FF00FFu) * m) >> 8) & 0x00FF00FFu};
    u32 const g {(((px & 0x0000FF00u) * m) >> 8) & 0x0000FF00u};
    return 0xFF000000u | rb | g;
}

static void BlitMasked(u32* dst, size_i screenSize, u8 const* tex, i32 texPitch, point_i srcOrigin, size_i srcSize, size_i drawSize, point_i dstPos, f64 scale)
{
    i32 const xStart {std::max(0, -dstPos.X)};
    i32 const xEnd {std::min(drawSize.Width, screenSize.Width - dstPos.X)};
    i32 const yStart {std::max(0, -dstPos.Y)};
    i32 const yEnd {std::min(drawSize.Height, screenSize.Height - dstPos.Y)};

    color_d const white {.R = 1.0, .G = 1.0, .B = 1.0};

    for (i32 y {yStart}; y < yEnd; ++y) {
        i32 const screenY {y + dstPos.Y};
        i32 const texY {srcOrigin.Y + std::min(srcSize.Height - 1, static_cast<i32>(y / scale))};
        for (i32 x {xStart}; x < xEnd; ++x) {
            i32 const screenX {x + dstPos.X};
            i32 const texX {srcOrigin.X + std::min(srcSize.Width - 1, static_cast<i32>(x / scale))};
            i32 const texOffset {(texX + (texY * texPitch)) * TEXTURE_BPP};
            if (IsMagenta(tex, texOffset)) { continue; }

            CopyPixel(dst, PixelIndex(screenSize, screenX, screenY), tex, texOffset, white);
        }
    }
}

static auto ComputeCameraRayDir(isize x, i32 screenWidth, player const& player) -> point_d { return player.Direction + (player.Plane * ((2.0 * (static_cast<f64>(x) + 0.5) / screenWidth) - 1.0)); }

static auto ComputeFalloff(f64 distSq, f64 rangeSq, f64 intensity) -> f64
{
    f64 const d {std::max(distSq, 0.01)};
    f64 const ratio {distSq / rangeSq};
    f64 const base {std::clamp(1.0 - (ratio * ratio), 0.0, 1.0)};
    return intensity * (base * base) / d;
}

static auto WorldToCell(point_d const& p) -> point_i { return {static_cast<i32>(std::floor(p.X)), static_cast<i32>(std::floor(p.Y))}; }

static auto DitherTint(color_d tint, isize x, isize y, quality_settings const& quality) -> color_d
{
    if (!quality.Dithering) { return tint; }

    f64 const  invBands {1.0 / DITHERING_LIGHT_BANDS};
    f64 const  threshold {BAYER_4X4[static_cast<usize>(((y & 3) << 2) | (x & 3))]};
    auto const quantize {[&](f64 t) { return std::floor((t * DITHERING_LIGHT_BANDS) + threshold) * invBands; }};
    return color_d {.R = quantize(tint.R), .G = quantize(tint.G), .B = quantize(tint.B)};
}

////////////////////////////////////////////////////////////

raycaster::raycaster(texture_cache& cache)
    : _cache {cache}
    , _taskManager {locate_service<task_manager>()}
{
    _cellLights.resize(static_cast<usize>(MAP_WIDTH) * MAP_HEIGHT);
}

void raycaster::set_quality(quality_settings const& quality, f64 projPlaneDist)
{
    _quality = quality;

    _screenSize    = _quality.Resolution;
    _projPlaneDist = projPlaneDist;

    _screen.assign(static_cast<usize>(_screenSize.area()), 0);
    _zBuffer.assign(static_cast<usize>(_screenSize.Width), 0.0);
    _objectDepthBuffer.assign(static_cast<usize>(_screenSize.area()), 0.0);
}

void raycaster::precompute_light_visibility(level const& level)
{
    for (auto& list : _cellLights) { list.clear(); }

    _dynamicLights.clear();
    for (auto const& go : level.objects()) {
        if (go.light()) { _dynamicLights.push_back(*go.light()); }
    }
    if (_dynamicLights.empty()) { return; }

    usize const losSamples {static_cast<usize>(_quality.LightLosSamples)};

    auto const is_cell_blocking {[&level](point_i const& cell) -> bool {
        if (!map_t::Size.contains(cell)) { return true; }
        return std::visit([](auto&& c) -> bool {
            using T = std::decay_t<decltype(c)>;
            if constexpr (std::is_same_v<T, floor_cell>) {
                return false;
            } else if constexpr (requires { c.State; }) {
                return c.State != wall_state::Open; // doors/push walls: open = passable
            } else {
                return true;                        // normal_wall, obstacle, diagonal_wall: always solid
            }
        },
                          level.get_cell(cell));
    }};

    auto const cell_has_any_line_of_sight {[&](point_i const& cell, point_d const& lightPos) -> bool {
        auto const lightCell {WorldToCell(lightPos)};

        // ordered so that any prefix is spatially balanced: center first, then both diagonals
        std::array<point_d, 5> const samples {{
            {cell.X + 0.5, cell.Y + 0.5},
            {cell.X + 0.1, cell.Y + 0.1},
            {cell.X + 0.9, cell.Y + 0.9},
            {cell.X + 0.9, cell.Y + 0.1},
            {cell.X + 0.1, cell.Y + 0.9},
        }};

        for (usize sampleIndex {0}; sampleIndex < losSamples; ++sampleIndex) {
            auto const& from {samples[sampleIndex]};
            auto const  diff {lightPos - from};
            f64 const   dist {diff.length()};
            if (dist < 1e-6) { return true; }

            point_d const rayDir {diff.X / dist, diff.Y / dist};
            point_i       map {static_cast<i32>(from.X), static_cast<i32>(from.Y)};

            point_d const deltaDist {(rayDir.X == 0) ? INFINITE_DIST : std::abs(1 / rayDir.X),
                                     (rayDir.Y == 0) ? INFINITE_DIST : std::abs(1 / rayDir.Y)};
            point_i       step {rayDir.X < 0 ? -1 : 1, rayDir.Y < 0 ? -1 : 1};
            point_d       sideDist {(rayDir.X < 0) ? (from.X - map.X) * deltaDist.X : (map.X + 1.0 - from.X) * deltaDist.X,
                                    (rayDir.Y < 0) ? (from.Y - map.Y) * deltaDist.Y : (map.Y + 1.0 - from.Y) * deltaDist.Y};

            bool visible {true};
            for (;;) {
                bool const stepX {sideDist.X < sideDist.Y};
                f64 const  traveled {stepX ? sideDist.X : sideDist.Y};
                if (traveled >= dist) { break; } // reached the light before hitting anything

                if (stepX) {
                    sideDist.X += deltaDist.X;
                    map.X += step.X;
                } else {
                    sideDist.Y += deltaDist.Y;
                    map.Y += step.Y;
                }

                if (!map_t::Size.contains(map)) { break; }
                if (map == lightCell) { break; }

                if (is_cell_blocking(map)) {
                    visible = false;
                    break;
                }
            }

            if (visible) { return true; }
        }
        return false;
    }};

    struct light_sweep {
        bool Active {false};
        i32  CxMin {0};
        i32  CxMax {-1};
        i32  CyMin {0};
        i32  CyMax {-1};
        f64  RadiusSq {0.0};
    };

    std::vector<light_sweep> sweeps(_dynamicLights.size());
    for (usize li {0}; li < _dynamicLights.size(); ++li) {
        auto const& light {_dynamicLights[li]};
        if (!map_t::Size.contains(WorldToCell(light.Position))) { continue; }

        f64 const sweepRadius {light.Range + CELL_DIAGONAL_HALF};

        auto& sweep {sweeps[li]};
        sweep.Active   = true;
        sweep.RadiusSq = sweepRadius * sweepRadius;
        sweep.CxMin    = std::clamp(static_cast<i32>(std::floor(light.Position.X - sweepRadius)), 0, MAP_WIDTH - 1);
        sweep.CxMax    = std::clamp(static_cast<i32>(std::floor(light.Position.X + sweepRadius)), 0, MAP_WIDTH - 1);
        sweep.CyMin    = std::clamp(static_cast<i32>(std::floor(light.Position.Y - sweepRadius)), 0, MAP_HEIGHT - 1);
        sweep.CyMax    = std::clamp(static_cast<i32>(std::floor(light.Position.Y + sweepRadius)), 0, MAP_HEIGHT - 1);
    }

    _taskManager.run_parallel([&](par_task const& ctx) {
        for (i32 cy {static_cast<i32>(ctx.Start)}; cy < static_cast<i32>(ctx.End); ++cy) {
            for (usize li {0}; li < sweeps.size(); ++li) {
                auto const& sweep {sweeps[li]};
                if (!sweep.Active || cy < sweep.CyMin || cy > sweep.CyMax) { continue; }

                auto const& light {_dynamicLights[li]};
                f64 const   dz {light.Z - WALL_LIGHT_Z};

                for (i32 cx {sweep.CxMin}; cx <= sweep.CxMax; ++cx) {
                    point_d const cellCenter {cx + 0.5, cy + 0.5};
                    point_d const toLightXY {light.Position - cellCenter};
                    f64 const     distSq {toLightXY.dot(toLightXY) + (dz * dz)};
                    if (distSq >= sweep.RadiusSq) { continue; }

                    if (cell_has_any_line_of_sight(point_i {cx, cy}, light.Position)) {
                        usize const cellIndex {static_cast<usize>((cy * MAP_WIDTH) + cx)};
                        _cellLights[cellIndex].push_back(static_cast<u32>(li));
                    }
                }
            }
        }
    },
                              MAP_HEIGHT);
}

auto raycaster::accumulate_light(player const& player, point_d const& surfacePos, f64 surfaceZ, point_i const& cell, u32 lightMask) const -> color_d
{
    static auto tone_map {[](f64 x) -> f64 { return x / (1.0 + x); }};

    static auto add_light_contribution {[](point_d const& lightPos, f64 lightHeight, f64 range, f64 intensity, color lightColor, point_d const& surPos, f64 surZ, color_d& total) {
        auto const toLightXY {lightPos - surPos};
        f64 const  dz {lightHeight - surZ};
        f64 const  distSq {toLightXY.dot(toLightXY) + (dz * dz)};
        f64 const  rangeSq {range * range};
        if (distSq >= rangeSq) { return; }
        f64 const strength {ComputeFalloff(distSq, rangeSq, intensity)};
        total.R += lightColor.R * INV_255 * strength;
        total.G += lightColor.G * INV_255 * strength;
        total.B += lightColor.B * INV_255 * strength;
    }};

    color_d total {};

    if ((lightMask & LIGHT_LAYER_WORLD) != 0) {
        add_light_contribution(player.Position, EYE_HEIGHT, player.Settings.LightRange, player.Settings.LightIntensity, color {255, 255, 255}, surfacePos, surfaceZ, total);
    }

    if (map_t::Size.contains(cell)) {
        usize const cellIndex {static_cast<usize>((cell.Y * MAP_WIDTH) + cell.X)};
        for (u32 li : _cellLights[cellIndex]) {
            auto const& light {_dynamicLights[li]};
            if ((light.Layers & lightMask) == 0) { continue; }
            add_light_contribution(light.Position, light.Z, light.Range, light.Intensity, light.Color, surfacePos, surfaceZ, total);
        }
    }

    return {.R = tone_map(total.R), .G = tone_map(total.G), .B = tone_map(total.B)};
}

void raycaster::shade_and_write(u32* screenBuf, isize x, isize y, u8 const* tex, isize srcIdx, player const& player, point_d const& surfacePos, f64 surfaceZ) const
{
    point_i const cell {WorldToCell(surfacePos)};
    color_d const tint {accumulate_light(player, surfacePos, surfaceZ, cell)};
    CopyPixel(screenBuf, PixelIndex(_screenSize, x, y), tex, srcIdx, DitherTint(tint, x, y, _quality));
}

auto raycaster::draw(level& level, player const& player) -> u32 const*
{
    std::ranges::fill(_objectDepthBuffer, std::numeric_limits<f64>::infinity());

    precompute_light_visibility(level);

    i32 const screenCenterY {(_screenSize.Height / 2) + static_cast<i32>(player.BobAmount)};

    _taskManager.run_parallel([&](par_task const& ctx) {
        draw_columns(level, player, screenCenterY, static_cast<i32>(ctx.Start), static_cast<i32>(ctx.End));
    },
                              _screenSize.Width);

    draw_sprites(level, player, screenCenterY);
    draw_voxel_objects(level, player, screenCenterY);

    draw_weapon(player);

    draw_screen_effect();

    draw_hud(player);
    draw_message(level);

    return _screen.data();
}

void raycaster::draw_columns(level& level, player const& player, i32 screenCenterY, i32 columnStart, i32 columnEnd)
{
    static auto compute_wall_screen_extent {[](f64 distance, i32 centerY, f64 projDist) -> std::pair<i32, i32> {
        f64 const lineHeight {projDist / distance};
        return {static_cast<i32>(std::round(centerY - (lineHeight / 2.0))), static_cast<i32>(std::round(centerY + (lineHeight / 2.0)))};
    }};

    // largest squared 2D distance at which the player light still reaches SEEN_LIGHT_THRESHOLD
    auto const compute_seen_range_sq {[&]() -> f64 {
        f64 const range {player.Settings.LightRange};
        f64 const rangeSq {range * range};
        f64 const intensity {player.Settings.LightIntensity};

        if (ComputeFalloff(0.0, rangeSq, intensity) < SEEN_LIGHT_THRESHOLD) { return -1.0; }

        f64 lo {0.0};
        f64 hi {rangeSq};
        for (i32 i {0}; i < 40; ++i) {
            f64 const mid {0.5 * (lo + hi)};
            if (ComputeFalloff(mid, rangeSq, intensity) >= SEEN_LIGHT_THRESHOLD) {
                lo = mid;
            } else {
                hi = mid;
            }
        }
        return lo;
    }};

    f64 const seenRangeSq {compute_seen_range_sq()};

    auto const is_within_player_light {[&](point_i const& cell) -> bool {
        point_d const cellCenter {cell.X + 0.5, cell.Y + 0.5};
        point_d const toCell {cellCenter - player.Position};
        return toCell.dot(toCell) <= seenRangeSq; // negative seenRangeSq never matches
    }};

    for (isize x {columnStart}; x < columnEnd; x++) {
        point_d const rayDir {ComputeCameraRayDir(x, _screenSize.Width, player)};

        point_i       map {player.Position};
        point_d const deltaDist {(rayDir.X == 0) ? INFINITE_DIST : std::abs(1 / rayDir.X),
                                 (rayDir.Y == 0) ? INFINITE_DIST : std::abs(1 / rayDir.Y)};

        point_i step {rayDir.X < 0 ? -1 : 1, rayDir.Y < 0 ? -1 : 1};
        point_d sideDist {
            (rayDir.X < 0) ? (player.Position.X - map.X) * deltaDist.X : (map.X + 1.0 - player.Position.X) * deltaDist.X,
            (rayDir.Y < 0) ? (player.Position.Y - map.Y) * deltaDist.Y : (map.Y + 1.0 - player.Position.Y) * deltaDist.Y};

        std::optional<wall_hit> hitResult {std::nullopt};

        // check player cell
        {
            auto const intersect {[&](auto&& c) { return c.intersect({map, player.Position, rayDir, sideDist.X < sideDist.Y, 0.0}); }};

            if (auto const wallHit {std::visit(intersect, level.get_cell(map))}) {
                hitResult = wallHit;
            }
            if (is_within_player_light(map)) { level.mark_seen(map); }
        }

        // DDA
        if (!hitResult) {
            bool       side {false};
            auto const intersect {[&](auto&& c) -> std::optional<wall_hit> { return c.intersect({map, player.Position, rayDir, side, !side ? sideDist.X - deltaDist.X : sideDist.Y - deltaDist.Y}); }};

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

                if (is_within_player_light(map)) { level.mark_seen(map); }

                auto const wallHit {std::visit(intersect, level.get_cell(map))};
                if (wallHit) {
                    hitResult = wallHit;
                    break;
                }
            }
        }

        if (!hitResult) { continue; }

        _zBuffer[x] = hitResult->Distance;

        auto const [wallTop, wallBottom] {compute_wall_screen_extent(hitResult->Distance, screenCenterY, _projPlaneDist)};
        wall_extent const extent {.Top = wallTop, .Bottom = wallBottom, .ScreenCenterY = screenCenterY};

        draw_floor_ceiling_column(*hitResult, level, player, x, rayDir, extent);

        draw_wall_column(*hitResult, player, x, rayDir, extent);
    }
}

void raycaster::draw_wall_column(wall_hit const& hit, player const& player, isize x, point_d rayDir, wall_extent const& extent)
{
    i32 const drawStart {std::max(extent.Top, 0)};
    i32 const drawEnd {std::min(extent.Bottom, _screenSize.Height)};

    if (drawStart >= drawEnd) { return; }

    auto const* tex {_cache.texture(hit.Texture, 0)};
    i32 const   texX {std::clamp(static_cast<i32>((1.0 - hit.SegmentT) * static_cast<f64>(WALL_SIZE.Width)), 0, WALL_SIZE.Width - 1)};
    f64 const   texStep {static_cast<f64>(WALL_SIZE.Height) / (extent.Bottom - extent.Top)};
    f64         texPos {(drawStart - extent.Top) * texStep};

    point_d const surfacePos {player.Position + (rayDir * hit.Distance)};
    f64 const     wallHeight {static_cast<f64>(extent.Bottom - extent.Top)};

    u32* screenBuf {_screen.data()};
    for (i32 y {drawStart}; y < drawEnd; y++) {
        i32 const texY {((static_cast<i32>(texPos) % WALL_SIZE.Height) + WALL_SIZE.Height) % WALL_SIZE.Height};
        texPos += texStep;
        i32 const srcIdx {(texX + (texY * WALL_SIZE.Width)) * TEXTURE_BPP};

        f64 const worldZ {std::clamp((extent.Bottom - y) / wallHeight, 0.0, 1.0)};
        shade_and_write(screenBuf, x, y, tex, srcIdx, player, surfacePos, worldZ);
    }
}

void raycaster::draw_floor_ceiling_column(wall_hit const& hit, level const& level, player const& player, isize x, point_d rayDir, wall_extent const& extent)
{
    i32 const ceilingEnd {std::clamp(extent.Top, 0, _screenSize.Height)};
    i32 const floorStart {std::clamp(extent.Bottom, 0, _screenSize.Height)};

    point_d const floorWall {player.Position + (rayDir * hit.Distance)};
    f64 const     invPerpWallDist {1.0 / hit.Distance};

    point_i   lastFloorCell {-1, -1};
    i32       cellFloorTex {INVALID_INDEX};
    i32       cellCeilTex {INVALID_INDEX};
    u8 const* cellFloorTexPtr {nullptr};
    u8 const* cellCeilTexPtr {nullptr};
    f64       cellReflect {0.0};
    u8 const* wallTexPtr {nullptr};

    auto const query_cell {[&](auto&& cell) {
        if constexpr (requires { cell.FloorTexture; }) {
            if (cell.FloorTexture != INVALID_INDEX) { cellFloorTex = cell.FloorTexture; }
        }
        if constexpr (requires { cell.CeilingTexture; }) {
            if (cell.CeilingTexture != INVALID_INDEX) { cellCeilTex = cell.CeilingTexture; }
        }
        if constexpr (requires { cell.FloorReflectivity; }) {
            cellReflect = cell.FloorReflectivity;
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
            cellFloorTex  = INVALID_INDEX;
            cellCeilTex   = INVALID_INDEX;
            cellReflect   = 0.0;
            if (map_t::Size.contains(floorCell)) {
                std::visit(query_cell, level.get_cell(floorCell));
            }
            cellFloorTexPtr = cellFloorTex != INVALID_INDEX ? _cache.texture(cellFloorTex, 0) : nullptr;
            cellCeilTexPtr  = cellCeilTex != INVALID_INDEX ? _cache.texture(cellCeilTex, 0) : nullptr;
        }

        u8 const* const tex {isFloor ? cellFloorTexPtr : cellCeilTexPtr};
        if (tex == nullptr) {
            screenBuf[PixelIndex(_screenSize, x, y)] = 0xFF000000u; // this cell has no texture for this surface
            return;
        }

        shade_and_write(screenBuf, x, y, tex, texelOffset, player, currentFloor, isFloor ? 0.0 : 1.0);

        f64 const wallGap {std::max(hit.Distance - rowDist, 0.0)};
        if (_quality.ContactShadows && std::isfinite(rowDist) && wallGap < CONTACT_SHADOW_WIDTH) {
            f64 const t {1.0 - (wallGap / CONTACT_SHADOW_WIDTH)};
            u32&      pixel {screenBuf[PixelIndex(_screenSize, x, y)]};
            pixel = ScalePixel(pixel, 1.0 - (CONTACT_SHADOW_STRENGTH * t * t));
        }

        if (isFloor && cellReflect > 0.0) {
            f64 const z {std::max(EYE_HEIGHT * ((hit.Distance / rowDist) - 1.0), 0.0)}; // height on the wall that the bounced ray reaches
            if (z < 1.0) {
                if (wallTexPtr == nullptr) { wallTexPtr = _cache.texture(hit.Texture, 0); }

                i32 const wallTexX {std::clamp(static_cast<i32>((1.0 - hit.SegmentT) * static_cast<f64>(WALL_SIZE.Width)), 0, WALL_SIZE.Width - 1)};
                i32 const wallTexY {std::clamp(static_cast<i32>((1.0 - z) * static_cast<f64>(WALL_SIZE.Height)), 0, WALL_SIZE.Height - 1)};
                i32 const wallSrc {(wallTexX + (wallTexY * WALL_SIZE.Width)) * TEXTURE_BPP};

                auto const wallTint {accumulate_light(player, floorWall, z, WorldToCell(floorWall))};
                u32 const  reflected {ApplyTint(wallTexPtr[wallSrc + 0], wallTexPtr[wallSrc + 1], wallTexPtr[wallSrc + 2], DitherTint(wallTint, x, y, _quality))};

                u32& pixel {screenBuf[PixelIndex(_screenSize, x, y)]};
                pixel = BlendPixel(pixel, reflected, cellReflect * (1.0 - z)); // fades toward the top of the wall
            }
        }
    }};

    for (i32 y {floorStart}; y < _screenSize.Height; y++) { sample_and_draw(y, true); }
    for (i32 y {0}; y < ceilingEnd; y++) { sample_and_draw(y, false); }
}

void raycaster::draw_sprites(level const& level, player const& player, i32 screenCenterY)
{
    f64 const    invDet {1.0 / player.Plane.cross(player.Direction)};
    rect_i const screenRect {point_i {0, 0}, _screenSize};

    u32* screenBuf {_screen.data()};
    for (auto const& go : level.objects()) {
        auto const* spr {go.sprite()};
        if (!spr) { continue; }

        auto const relPos {spr->Position - player.Position};

        f64 const transformX {invDet * relPos.cross(player.Direction)};
        f64 const transformY {invDet * player.Plane.cross(relPos)};
        if (transformY <= 0) { continue; }

        i32 const  spriteScreenX {static_cast<i32>((_screenSize.Width / 2.0) * (1.0 + (transformX / transformY)))};
        f64 const  scale {_projPlaneDist / transformY};
        auto const spriteSize {spr->screen_size(scale)};

        i32 const spriteLeft {spriteScreenX - (spriteSize.Width / 2)};
        i32 const spriteBottom {spr->screen_bottom(scale, screenCenterY, EYE_HEIGHT)};
        i32 const spriteTop {spriteBottom - spriteSize.Height};

        rect_i const spriteRect {point_i {spriteLeft, spriteTop}, spriteSize};
        if (!screenRect.intersects(spriteRect, true)) { continue; }

        point_i const drawStart {std::max(spriteLeft, 0), std::max(spriteTop, 0)};
        point_i const drawEnd {std::min(spriteLeft + spriteSize.Width, _screenSize.Width),
                               std::min(spriteTop + spriteSize.Height, _screenSize.Height)};

        i32 const   facing {spr->facing_index(player.Position)};
        auto const* tex {_cache.texture(spr->Texture, facing)};
        auto const  texSize {_cache.texture_size(spr->Texture, facing)};

        f64 const texStepY {static_cast<f64>(texSize.Height) / spriteSize.Height};
        f64 const texPosYStart {(drawStart.Y - spriteTop) * texStepY};

        auto const spriteCell {WorldToCell(spr->Position)};
        auto const spriteTint {accumulate_light(player, spr->Position, spr->BaseZ, spriteCell, spr->LightMask)};

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
                    CopyPixel(screenBuf, PixelIndex(_screenSize, x, y), tex, texOffset, DitherTint(spriteTint, x, y, _quality));
                }
            }
        }
    }
}

void raycaster::draw_voxel_objects(level& level, player const& player, i32 screenCenterY)
{
    f64 const  invDet {1.0 / player.Plane.cross(player.Direction)};
    auto const project {[&](point3_d const& world) -> std::tuple<f64, f64, f64> {
        point_d const relPos {world.X - player.Position.X, world.Y - player.Position.Y};

        f64 const transformX {invDet * relPos.cross(player.Direction)};
        f64 const transformY {invDet * player.Plane.cross(relPos)};

        if (transformY <= 1e-6) { return {0.0, 0.0, 0.0}; }

        f64 const screenX {(_screenSize.Width / 2.0) * (1.0 + (transformX / transformY))};
        f64 const scale {_projPlaneDist / transformY};
        f64 const screenY {screenCenterY - ((world.Z - EYE_HEIGHT) * scale)};

        return {screenX, screenY, transformY};
    }};

    rect_i const screenRect {point_i {0, 0}, _screenSize};
    u32*         screenBuf {_screen.data()};

    for (auto& go : level.objects()) {
        auto* const vo {go.voxel()};
        if (!vo) { continue; }

        vo->Seen = false;
        if (!vo->Grid) { continue; }

        auto const xf {vo->make_transform()};

        f64  bboxMinX {std::numeric_limits<f64>::infinity()}, bboxMaxX {-std::numeric_limits<f64>::infinity()};
        f64  bboxMinY {std::numeric_limits<f64>::infinity()}, bboxMaxY {-std::numeric_limits<f64>::infinity()};
        f64  bboxMinDepth {std::numeric_limits<f64>::infinity()};
        bool anyVisible {false};

        for (auto const& corner : vo->Grid->corners()) {
            auto const [sx, sy, depth] {project(xf.to_world(corner))};
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

        auto const localOrigin {xf.to_local(point3_d {.X = player.Position.X, .Y = player.Position.Y, .Z = EYE_HEIGHT})};

        f64 maxWallDist {0.0};
        for (i32 x {xStart}; x < xEnd; ++x) { maxWallDist = std::max(maxWallDist, _zBuffer[x]); }
        if (bboxMinDepth >= maxWallDist) { continue; }

        vo->Seen = true;

        f64 const maxT {maxWallDist + vo->world_diagonal()};

        i32 const colCount {xEnd - xStart};
        i32 const rowCount {yEnd - yStart};
        i32 const bboxArea {colCount * rowCount};
        i32 const stride {bboxArea > _quality.VoxelPixelBudget ? static_cast<i32>(std::ceil(std::sqrt(static_cast<f64>(bboxArea) / _quality.VoxelPixelBudget)))
                                                               : 1};
        i32 const strideCols {(colCount + stride - 1) / stride};

        _taskManager.run_parallel([&](par_task const& ctx) {
            for (isize scol {static_cast<isize>(ctx.Start)}; scol < static_cast<isize>(ctx.End); ++scol) {
                i32 const x {xStart + (static_cast<i32>(scol) * stride)};
                if (x >= xEnd) { continue; }

                auto const rayDir2D {ComputeCameraRayDir(x, _screenSize.Width, player)};
                i32 const  xBlockEnd {std::min(x + stride, xEnd)};

                for (i32 y {yStart}; y < yEnd; y += stride) {
                    f64 const  rayDirZ {(screenCenterY - y) / _projPlaneDist};
                    auto const rayDir3D {point3_d {.X = rayDir2D.X, .Y = rayDir2D.Y, .Z = rayDirZ}.normalized()};

                    auto const localDir {xf.dir_to_local(rayDir3D)};

                    auto const hit {vo->Grid->raycast(localOrigin, localDir, maxT)};
                    if (!hit.Hit) { continue; }

                    auto const    localHitPos {localOrigin + (localDir * hit.T)};
                    auto const    worldHit {xf.to_world(localHitPos)};
                    point_d const worldHitXY {worldHit.X, worldHit.Y};
                    auto const [sx, sy, depth] {project(worldHit)};

                    if (depth <= 0.0) { continue; }
                    if (depth >= _zBuffer[x]) { continue; }

                    auto const hitCellForLight {WorldToCell(worldHitXY)};
                    f64 const  aoFactor {_quality.VoxelAo ? vo->Grid->face_ao(hit, localHitPos) : 1.0};

                    auto tint {accumulate_light(player, worldHitXY, worldHit.Z, hitCellForLight)};
                    tint.R *= aoFactor;
                    tint.G *= aoFactor;
                    tint.B *= aoFactor;

                    i32 const yBlockEnd {std::min(y + stride, yEnd)};
                    for (i32 by {y}; by < yBlockEnd; ++by) {
                        for (i32 bx {x}; bx < xBlockEnd; ++bx) {
                            if (depth >= _zBuffer[bx]) { continue; }

                            isize const depthIndex {PixelIndex(_screenSize, bx, by)};
                            if (depth >= _objectDepthBuffer[depthIndex]) { continue; }

                            _objectDepthBuffer[depthIndex] = depth;
                            screenBuf[depthIndex]          = ApplyTint(hit.Color.R, hit.Color.G, hit.Color.B, DitherTint(tint, bx, by, _quality));
                        }
                    }
                }
            }
        },
                                  strideCols);
    }
}

void raycaster::draw_screen_effect()
{
    if (ScreenEffect) {
        ScreenEffect(_screen);
    }
}

void raycaster::draw_weapon(player const& player)
{
    auto* const tex {_cache.texture(handTexture, 0)};
    auto const  texSize {_cache.texture_size(handTexture, 0)};

    f64 const     scale {_screenSize.Height / REFERENCE_HEIGHT};
    size_i const  drawSize {size_f {texSize} * scale};
    i32 const     bobOffsetY {static_cast<i32>(player.BobAmount * WEAPON_BOB_MULTIPLIER)};
    point_i const offset {static_cast<i32>((_screenSize.Width - drawSize.Width) * 0.75),
                          static_cast<i32>(_screenSize.Height - (drawSize.Height * 0.75)) + bobOffsetY};

    BlitMasked(_screen.data(), _screenSize, tex, texSize.Width, point_i {0, 0}, texSize, drawSize, offset, scale);
}

void raycaster::draw_hud(player const&)
{
    auto* const tex {_cache.texture(hudTexture, 0)};
    auto const  texSize {_cache.texture_size(hudTexture, 0)};

    f64 const    scale {_screenSize.Height / REFERENCE_HEIGHT};
    size_i const drawSize {size_f {texSize} * scale};

    BlitMasked(_screen.data(), _screenSize, tex, texSize.Width, point_i {0, 0}, texSize, drawSize, point_i {0, 0}, scale);
}

void raycaster::draw_message(level const& level)
{
    auto const& msg {level.get_message()};
    if (msg.empty()) { return; }

    auto* const tex {_cache.texture(fontTexture, 0)};
    auto const  texSize {_cache.texture_size(fontTexture, 0)};
    auto const  charSize {texSize.Height};

    point_i const offset {5, 5};
    size_i const  glyphSize {charSize, charSize};

    for (i32 i {0}; i < std::ssize(msg); ++i) {
        char const c {msg[i]};
        if (std::isspace(static_cast<unsigned char>(c))) { continue; }

        i32 index {0};
        if (std::isdigit(static_cast<unsigned char>(c))) {
            index = c - '0';
        } else if (std::isalpha(static_cast<unsigned char>(c))) {
            index = 10 + (std::tolower(static_cast<unsigned char>(c)) - 'a');
        }

        BlitMasked(_screen.data(), _screenSize, tex, texSize.Width, point_i {index * charSize, 0}, glyphSize, glyphSize, point_i {offset.X + (i * (charSize + 1)), offset.Y}, 1.0);
    }
}
