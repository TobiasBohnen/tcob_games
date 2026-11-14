// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Raycaster.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <optional>
#include <span>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include "Common.hpp"
#include "Level.hpp"
#include "Player.hpp"
#include "TextureCache.hpp"
#include "Voxel.hpp"
#include "Walls.hpp"

// Stands in for infinity: a distance or depth nothing reaches ("no wall", "nothing drawn here yet").
// Finite on purpose, so 0 * INFINITE_DIST is 0 and not NaN when a ray starts exactly on a cell edge.
constexpr f64 INFINITE_DIST {1e30};
constexpr f64 INV_255 {1.0 / 255.0};
constexpr f64 WALL_RECESS_EPS {1e-6}; // a wall hit further into its cell than this is recessed (door, diagonal)
constexpr f64 MIN_RAY_DIST {1e-4};    // keeps row projection finite for surfaces at the player

static auto ApplyTint(u8 r, u8 g, u8 b, color_d tint) -> u32
{
    u8 const tr {static_cast<u8>(std::min(r * tint.R, 255.0))};
    u8 const tg {static_cast<u8>(std::min(g * tint.G, 255.0))};
    u8 const tb {static_cast<u8>(std::min(b * tint.B, 255.0))};
    return 0xFF000000u | (static_cast<u32>(tb) << 16) | (static_cast<u32>(tg) << 8) | static_cast<u32>(tr);
}

static void CopyPixel(u32* dst, usize dstIdx, u8 const* src, usize srcIdx, color_d tint) { dst[dstIdx] = ApplyTint(src[srcIdx + 0], src[srcIdx + 1], src[srcIdx + 2], tint); }

// Computes ray direction vector using the camera plane for screen column x:
// Dir = player.Direction + player.Plane * ((2 * x / width) - 1)
static auto ComputeCameraRayDir(isize x, i32 screenWidth, player const& player) -> point_d { return player.Direction + (player.Plane * ((2.0 * (static_cast<f64>(x) + 0.5) / screenWidth) - 1.0)); }

// Calculates light attenuation using inverse-square distance law combined with
// a smooth quartic falloff envelope: intensity * (1 - (dist^4 / range^4))^2 / dist^2
static auto ComputeFalloff(f64 distSq, f64 rangeSq, f64 intensity) -> f64
{
    f64 const d {std::max(distSq, 0.01)};
    f64 const ratio {distSq / rangeSq};
    f64 const base {std::clamp(1.0 - (ratio * ratio), 0.0, 1.0)};
    return intensity * (base * base) / d;
}

static auto DitherTint(color_d tint, isize x, isize y, quality_settings const& quality) -> color_d
{
    constexpr static f64 DITHERING_LIGHT_BANDS {8};

    constexpr static std::array<f64, 16> BAYER_4X4 {0.03125, 0.53125, 0.15625, 0.65625,
                                                    0.78125, 0.28125, 0.90625, 0.40625,
                                                    0.21875, 0.71875, 0.09375, 0.59375,
                                                    0.96875, 0.46875, 0.84375, 0.34375};

    if (!quality.Dithering) { return tint; }

    f64 const  invBands {1.0 / DITHERING_LIGHT_BANDS};
    f64 const  threshold {BAYER_4X4[static_cast<usize>(((y & 3) << 2) | (x & 3))]};
    auto const quantize {[&](f64 t) { return std::floor((t * DITHERING_LIGHT_BANDS) + threshold) * invBands; }};
    return color_d {.R = quantize(tint.R), .G = quantize(tint.G), .B = quantize(tint.B)};
}

static auto IsWithinPlayerLight(player const& player, point_i cell) -> bool
{
    f64 const     range {player.Settings.LightRange};
    point_d const toCell {point_d {cell.X + 0.5, cell.Y + 0.5} - player.Position};
    return toCell.dot(toCell) <= range * range;
}

static auto HashUnit(i32 gx, i32 gy, u32 index, u32 salt) -> f64
{
    u32 h {(static_cast<u32>(gx) * 73856093u) ^ (static_cast<u32>(gy) * 19349663u) ^ (index * 83492791u) ^ (salt * 2654435761u)};
    h ^= h >> 16;
    h *= 0x7feb352du;
    h ^= h >> 15;
    h *= 0x846ca68bu;
    h ^= h >> 16;
    return static_cast<f64>(h) / 4294967296.0;
}

static auto BlendPixel(u32 a, u32 b, f64 t) -> u32
{
    u32 const w {static_cast<u32>(std::clamp(t, 0.0, 1.0) * 256.0)};
    u32 const inv {256 - w};
    u32 const rb {((((a & 0x00FF00FFu) * inv) + ((b & 0x00FF00FFu) * w)) >> 8) & 0x00FF00FFu};
    u32 const g {((((a & 0x0000FF00u) * inv) + ((b & 0x0000FF00u) * w)) >> 8) & 0x0000FF00u};
    return 0xFF000000u | rb | g;
}

////////////////////////////////////////////////////////////

struct raycaster::column_hit {
    struct entry {
        point_i Cell;
        f64     Dist {}; // distance at which the ray entered the cell
    };

    isize    X {};
    point_d  RayDir;
    wall_hit Hit;
    point_i  Cell;                  // cell that was hit
    point_i  ToOpen;                // offset from Cell to the open cell the ray came from; zero when the hit is in the player's own cell
    i32      CenterY {};
    f64      WallBase {};           // world z of the bottom of the wall
    f64      WallTop {};            // world z of the top of the wall
    i32      Top {};                // rows [Top, Bottom) are still undrawn; the spans shrink this window, the wall fills what is left
    i32      Bottom {};

    std::span<entry const> Crossed; // every cell the ray passed through, in order; the last one is the hit cell
};

raycaster::raycaster(texture_cache& cache)
    : _cache {cache}
    , _taskManager {locate_service<task_manager>()}
{
}

void raycaster::set_quality(quality_settings const& quality, f64 projPlaneDist)
{
    _quality = quality;

    _screenSize    = _quality.Resolution;
    _projPlaneDist = projPlaneDist;

    _screen.assign(static_cast<usize>(_screenSize.area()), 0);
    _wallDepthBuffer.assign(static_cast<usize>(_screenSize.Width), 0.0);
    _objectDepthBuffer.assign(static_cast<usize>(_screenSize.area()), 0.0);
}

auto raycaster::accumulate_light(point_d const& surfacePos, f64 surfaceZ, point_i cell, u32 lightMask) const -> color_d
{
    auto const& level {*_level};
    auto const& player {*_player};

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
        add_light_contribution(player.Position, player.EyeZ, player.Settings.LightRange, player.Settings.LightIntensity, color {255, 255, 255}, surfacePos, surfaceZ, total);
    }

    auto const lights {level.lights()};
    for (u32 li : level.lights_at(cell)) {
        auto const& lt {lights[li]};
        if ((lt.Layers & lightMask) == 0) { continue; }
        add_light_contribution(lt.Position, lt.Z, lt.Range, lt.Intensity, lt.Color, surfacePos, surfaceZ, total);
    }

    return {.R = tone_map(total.R), .G = tone_map(total.G), .B = tone_map(total.B)};
}

void raycaster::draw_surface_pixel(isize x, isize y, u8 const* tex, isize srcIdx, point_d const& surfacePos, f64 surfaceZ, f64 shade)
{
    constexpr static f64 DITHER_MIN_DIST_SQ {0.5}; // closer than this, surfaces are drawn smooth

    auto const& player {*_player};

    auto const cell {WorldToCell(surfacePos)};
    auto       tint {accumulate_light(surfacePos, surfaceZ, cell, LIGHT_LAYERS_ALL)};
    tint.R *= shade;
    tint.G *= shade;
    tint.B *= shade;

    auto const toSurface {surfacePos - player.Position};
    bool const close {toSurface.dot(toSurface) < DITHER_MIN_DIST_SQ};
    u32*       screenBuf {_screen.data()};
    CopyPixel(screenBuf, PixelIndex(_screenSize.Width, x, y), tex, srcIdx, close ? tint : DitherTint(tint, x, y, _quality));
}

auto raycaster::draw(level& level, player& player) -> std::span<u32>
{
    constexpr static f64 EYE_HEIGHT {0.5}; // eye height above the floor under the player

    _level  = &level;
    _player = &player;

    std::ranges::fill(_objectDepthBuffer, INFINITE_DIST);

    level.update_light_visibility(static_cast<usize>(_quality.LightLosSamples));

    auto const playerCell {WorldToCell(player.Position)};
    player.EyeZ = HeightAt(level.slopes_at(playerCell).Floor, playerCell, player.Position) + EYE_HEIGHT;

    i32 const screenCenterY {(_screenSize.Height / 2) + static_cast<i32>(player.bob_amount())};

    _camera = {player, _screenSize.Width, _projPlaneDist, screenCenterY};

    _taskManager.run_parallel([&](par_task const& ctx) {
        draw_columns(screenCenterY, static_cast<i32>(ctx.Start), static_cast<i32>(ctx.End));
    },
                              _screenSize.Width);

    draw_sprites(screenCenterY);
    draw_voxel_objects(screenCenterY);
    draw_motes();

    _level  = nullptr;
    _player = nullptr;

    return _screen;
}

void raycaster::draw_columns(i32 screenCenterY, i32 columnStart, i32 columnEnd)
{
    auto const& level {*_level};
    auto const& player {*_player};

    std::vector<column_hit::entry> crossed;
    crossed.reserve(static_cast<usize>(MAP_WIDTH) + MAP_HEIGHT + 2); // a ray cannot cross more cells than this

    for (isize x {columnStart}; x < columnEnd; x++) {
        auto const rayDir {ComputeCameraRayDir(x, _screenSize.Width, player)};

        point_i       map {player.Position};
        point_d const deltaDist {(rayDir.X == 0) ? INFINITE_DIST : std::abs(1 / rayDir.X),
                                 (rayDir.Y == 0) ? INFINITE_DIST : std::abs(1 / rayDir.Y)};

        point_i step {rayDir.X < 0 ? -1 : 1, rayDir.Y < 0 ? -1 : 1};
        point_d sideDist {(rayDir.X < 0) ? (player.Position.X - map.X) * deltaDist.X : (map.X + 1.0 - player.Position.X) * deltaDist.X,
                          (rayDir.Y < 0) ? (player.Position.Y - map.Y) * deltaDist.Y : (map.Y + 1.0 - player.Position.Y) * deltaDist.Y};

        // Walk the cells in order. The first one is the player's own: the ray starts inside it, so its
        // "entry" is distance 0 and the side it is left through. For every other cell it is where the ray came in.
        bool                    side {sideDist.X < sideDist.Y};
        f64                     entryDist {0.0};
        std::optional<wall_hit> hitResult;

        auto const intersect {[&](auto&& c) { return c.intersect({map, player.Position, rayDir, side, entryDist}); }};

        crossed.clear();
        for (;;) {
            crossed.push_back({.Cell = map, .Dist = entryDist});

            hitResult = std::visit(intersect, level.get_cell(map));
            if (hitResult) { break; }

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

            entryDist = !side ? sideDist.X - deltaDist.X : sideDist.Y - deltaDist.Y;
        }

        if (!hitResult) {
            _wallDepthBuffer[x] = INFINITE_DIST;
            continue;
        }

        _wallDepthBuffer[x] = hitResult->Distance;

        bool const hitInPlayerCell {crossed.size() == 1};
        auto const prevCell {hitInPlayerCell ? map : crossed[crossed.size() - 2].Cell}; // open cell the ray came from
        bool const recessed {hitInPlayerCell || ((hitResult->Distance - crossed.back().Dist) > WALL_RECESS_EPS)};
        auto const baseCell {recessed ? map : prevCell};
        auto const baseSlope {level.slopes_at(baseCell)};
        auto const hitPoint {player.Position + (rayDir * hitResult->Distance)};

        column_hit col {.X        = x,
                        .RayDir   = rayDir,
                        .Hit      = *hitResult,
                        .Cell     = map,
                        .ToOpen   = hitInPlayerCell ? point_i::Zero : (side ? point_i {0, -step.Y} : point_i {-step.X, 0}),
                        .CenterY  = screenCenterY,
                        .WallBase = HeightAt(baseSlope.Floor, baseCell, hitPoint),
                        .WallTop  = HeightAt(baseSlope.Ceiling, baseCell, hitPoint),
                        .Top      = 0,
                        .Bottom   = _screenSize.Height,
                        .Crossed  = crossed};

        draw_floor_ceiling_column(col);
        draw_wall_column(col);
    }
}

void raycaster::draw_floor_ceiling_column(column_hit& col)
{
    static auto scale_pixel {[](u32 px, f64 factor) -> u32 {
        u32 const m {static_cast<u32>(std::clamp(factor, 0.0, 1.0) * 256.0)};
        u32 const rb {(((px & 0x00FF00FFu) * m) >> 8) & 0x00FF00FFu};
        u32 const g {(((px & 0x0000FF00u) * m) >> 8) & 0x0000FF00u};
        return 0xFF000000u | rb | g;
    }};

    constexpr static f64 CONTACT_SHADOW_WIDTH {0.3};
    constexpr static f64 CONTACT_SHADOW_STRENGTH {0.75};
    constexpr static f64 REFLECTION_HEIGHT {1.0}; // floor reflections fade out this far up a wall

    auto&       level {*_level};
    auto const& player {*_player};

    u32*      screenBuf {_screen.data()};
    u8 const* wallTexPtr {nullptr};

    auto const wall_texture {[&]() -> u8 const* {
        if (wallTexPtr == nullptr) { wallTexPtr = _cache.texture(col.Hit.Texture, 0); }
        return wallTexPtr;
    }};

    constexpr static f64 MIN_RISER_HEIGHT {1e-4};
    constexpr static f64 RISER_LIGHT_BIAS {0.001}; // lights the face from the open side

    // vertical face where a surface does not meet the one of the cell before it
    auto const draw_riser {[&](bool isFloor, point_i prevCell, point_i cell, auto const& prevSlopes, auto const& slopes, cell_surface const& surface, f64 t) {
        i32 const texId {isFloor ? surface.FloorTexture : surface.CeilingTexture};
        if (texId == SKY_TEXTURE) { return; }

        auto const edge {player.Position + (col.RayDir * t)};
        f64 const  prevZ {HeightAt(isFloor ? prevSlopes.Floor : prevSlopes.Ceiling, prevCell, edge)};
        f64 const  hereZ {HeightAt(isFloor ? slopes.Floor : slopes.Ceiling, cell, edge)};

        // only faces turned toward the player: a floor that steps up, a ceiling that steps down
        if ((isFloor ? hereZ - prevZ : prevZ - hereZ) <= MIN_RISER_HEIGHT) { return; }

        f64 const  rowsPerUnit {_projPlaneDist / std::max(t, MIN_RAY_DIST)};
        auto const row {[&](f64 z) { return static_cast<i32>(std::clamp(std::round(col.CenterY + ((player.EyeZ - z) * rowsPerUnit)), -1.0, static_cast<f64>(_screenSize.Height) + 1.0)); }};
        i32 const  from {std::max(row(std::max(hereZ, prevZ)), col.Top)};
        i32 const  to {std::min(row(std::min(hereZ, prevZ)), col.Bottom)};
        if (from >= to) { return; }

        u8 const* tex {texId != INVALID_INDEX ? _cache.texture(texId, 0) : nullptr};
        if (tex == nullptr) { tex = wall_texture(); }
        if (tex == nullptr) { return; }

        // u runs along the edge: if the cells differ in x, the edge runs along y
        f64 const     along {cell.X != prevCell.X ? edge.Y : edge.X};
        i32 const     texelX {static_cast<i32>(along * WALL_SIZE.Width) & (WALL_SIZE.Width - 1)};
        point_d const litPos {player.Position + (col.RayDir * std::max(t - RISER_LIGHT_BIAS, 0.0))};

        for (i32 y {from}; y < to; ++y) {
            f64 const z {player.EyeZ - (((static_cast<f64>(y) - col.CenterY) / _projPlaneDist) * t)};
            i32 const texelY {(WALL_SIZE.Height - 1) - (static_cast<i32>(std::floor(z * WALL_SIZE.Height)) & (WALL_SIZE.Height - 1))};
            i32 const texelOffset {(texelX + (texelY * WALL_SIZE.Width)) * TEXTURE_BPP};

            draw_surface_pixel(col.X, y, tex, texelOffset, litPos, z, 1.0);
            _objectDepthBuffer[PixelIndex(_screenSize.Width, col.X, y)] = t;
        }

        if (isFloor) {
            col.Bottom = std::min(col.Bottom, from);
        } else {
            col.Top = std::max(col.Top, to);
        }
    }};

    auto const draw_flat {[&](bool isFloor, cell_surface const& surface, point_i cell, auto const& slopes, f64 tIn, f64 tOut) {
        auto const& slope {isFloor ? slopes.Floor : slopes.Ceiling};

        auto const p {player.Position + (col.RayDir * tOut)};
        i32 const  rowFar {static_cast<i32>(std::clamp(std::round(col.CenterY + ((player.EyeZ - HeightAt(slope, cell, p)) * _projPlaneDist / std::max(tOut, MIN_RAY_DIST))), -1.0, static_cast<f64>(_screenSize.Height) + 1.0))};
        i32 const  from {isFloor ? std::max(rowFar, col.Top) : col.Top};
        i32 const  to {isFloor ? col.Bottom : std::min(rowFar, col.Bottom)};
        if (from >= to) { return; }

        if (!isFloor && surface.CeilingTexture == SKY_TEXTURE) {
            for (i32 y {from}; y < to; ++y) { screenBuf[PixelIndex(_screenSize.Width, col.X, y)] = sky_pixel(y, col.RayDir); }
            col.Top = std::max(col.Top, to);
            return;
        }

        auto const dp {player.Position - cell};
        auto const [vx, vy] {col.RayDir};

        // Plane equation coefficients (A, B, C) for non-flat slopes.
        // Substituting ray equation P = pos + t * rayDir into the bilinear slope equation
        // yields a quadratic equation in t: A*t^2 + term*t + (C - eyeZ) = 0.
        f64 const D {slope.BR - slope.BL - slope.TR + slope.TL};
        f64 const A {D * vx * vy};
        f64 const B {((slope.TR - slope.TL) * vx) + ((slope.BL - slope.TL) * vy) + (D * ((dp.X * vy) + (dp.Y * vx)))};
        f64 const C {slope.TL + ((slope.TR - slope.TL) * dp.X) + ((slope.BL - slope.TL) * dp.Y) + (D * dp.X * dp.Y)};

        i32 const texId {isFloor ? surface.FloorTexture : surface.CeilingTexture};
        u8 const* tex {texId != INVALID_INDEX ? _cache.texture(texId, 0) : nullptr};
        if (tex == nullptr) { tex = wall_texture(); }
        if (tex == nullptr) { return; }

        bool const linear {std::abs(A) < 1e-12};

        for (i32 y {from}; y < to; ++y) {
            f64 const s {(static_cast<f64>(y) - col.CenterY) / _projPlaneDist};
            f64 const term {s + B};
            f64       t {tOut};

            if (linear) {
                // Linear case when cross-term coefficient A is near zero: t = (eyeZ - C) / term
                if (std::abs(term) > 1e-9) {
                    t = std::clamp((player.EyeZ - C) / term, tIn, tOut);
                }
            } else {
                // Quadratic formula root finding: (-term ± sqrt(discriminant)) / (2 * A)
                f64 const discriminant {(term * term) - (4.0 * A * (C - player.EyeZ))};
                if (discriminant >= 0.0) {
                    f64 const sqrtDisc {std::sqrt(discriminant)};
                    f64 const t1 {(-term - sqrtDisc) / (2.0 * A)};
                    f64 const t2 {(-term + sqrtDisc) / (2.0 * A)};

                    // take the root closest to this cell's span, a row on the far edge can land just outside it
                    auto const gap {[&](f64 r) { return std::max({tIn - r, r - tOut, 0.0}); }};
                    t = std::clamp(gap(t1) <= gap(t2) ? t1 : t2, tIn, tOut);
                }
            }

            auto const pos {player.Position + (col.RayDir * t)};
            f64 const  z {HeightAt(slope, cell, pos)};

            i32 const texelX {static_cast<i32>(pos.X * WALL_SIZE.Width) & (WALL_SIZE.Width - 1)};
            i32 const texelY {static_cast<i32>(pos.Y * WALL_SIZE.Height) & (WALL_SIZE.Height - 1)};
            i32 const texelOffset {(texelX + (texelY * WALL_SIZE.Width)) * TEXTURE_BPP};

            draw_surface_pixel(col.X, y, tex, texelOffset, pos, z, 1.0);
            _objectDepthBuffer[PixelIndex(_screenSize.Width, col.X, y)] = t;

            f64 const wallGap {std::max(col.Hit.Distance - t, 0.0)};
            if (_quality.ContactShadows && wallGap < CONTACT_SHADOW_WIDTH) {
                f64 const c {1.0 - (wallGap / CONTACT_SHADOW_WIDTH)};
                u32&      pixel {screenBuf[PixelIndex(_screenSize.Width, col.X, y)]};
                pixel = scale_pixel(pixel, 1.0 - (CONTACT_SHADOW_STRENGTH * c * c));
            }

            if (isFloor && surface.FloorReflectivity > 0.0) {
                f64 const mirrored {z - col.WallBase + ((player.EyeZ - z) * ((col.Hit.Distance / std::max(t, MIN_RAY_DIST)) - 1.0))};
                f64 const height {std::max(mirrored, 0.0)}; // height above the wall base that the bounced ray reaches
                if (height < std::min(REFLECTION_HEIGHT, col.WallTop - col.WallBase) && wall_texture() != nullptr) {
                    u8 const* wallTex {wall_texture()};

                    i32 const wallTexX {std::clamp(static_cast<i32>((1.0 - col.Hit.SegmentT) * static_cast<f64>(WALL_SIZE.Width)), 0, WALL_SIZE.Width - 1)};
                    i32 const wallTexY {std::clamp(static_cast<i32>((1.0 - (height - std::floor(height))) * static_cast<f64>(WALL_SIZE.Height)), 0, WALL_SIZE.Height - 1)};
                    i32 const wallSrc {(wallTexX + (wallTexY * WALL_SIZE.Width)) * TEXTURE_BPP};

                    point_d const wallPoint {player.Position + (col.RayDir * col.Hit.Distance)};
                    auto const    wallTint {accumulate_light(wallPoint, col.WallBase + height, WorldToCell(wallPoint), LIGHT_LAYERS_ALL)};
                    u32 const     reflected {ApplyTint(wallTex[wallSrc + 0], wallTex[wallSrc + 1], wallTex[wallSrc + 2], DitherTint(wallTint, col.X, y, _quality))};

                    u32& pixel {screenBuf[PixelIndex(_screenSize.Width, col.X, y)]};
                    pixel = BlendPixel(pixel, reflected, surface.FloorReflectivity * (1.0 - (height / REFLECTION_HEIGHT)));
                }
            }
        }

        if (isFloor) {
            col.Bottom = std::min(col.Bottom, from);
        } else {
            col.Top = std::max(col.Top, to);
        }
    }};

    usize const count {col.Crossed.size()};
    for (usize i {0}; i < count; ++i) {
        f64 const  t0 {col.Crossed[i].Dist};
        bool const isLast {i + 1 == count};

        if (isLast && col.Hit.Distance - t0 <= WALL_RECESS_EPS) { break; } // wall on the cell face: it covers the rest of the column

        f64 const t1 {isLast ? col.Hit.Distance : col.Crossed[i + 1].Dist};
        if (t1 <= t0) { continue; }

        auto const cell {col.Crossed[i].Cell};
        auto const surface {level.surface_at(cell)};
        auto const slopes {level.slopes_at(cell)};

        i32 const bottomBefore {col.Bottom};
        i32 const topBefore {col.Top};

        if (i > 0) {
            auto const prevCell {col.Crossed[i - 1].Cell};
            auto const prevSlopes {level.slopes_at(prevCell)};
            draw_riser(true, prevCell, cell, prevSlopes, slopes, surface, t0);
            draw_riser(false, prevCell, cell, prevSlopes, slopes, surface, t0);
        }

        draw_flat(true, surface, cell, slopes, t0, t1);
        draw_flat(false, surface, cell, slopes, t0, t1);

        if ((col.Bottom < bottomBefore || col.Top > topBefore) && IsWithinPlayerLight(player, cell)) { level.mark_visible(cell); }
    }

    if (col.Top < col.Bottom && IsWithinPlayerLight(player, col.Cell)) { level.mark_visible(col.Cell); }
}

void raycaster::draw_wall_column(column_hit const& col)
{
    constexpr static f64 CORNER_WIDTH {0.25}; // world units from an inside corner
    constexpr static f64 CORNER_STRENGTH {0.6};
    constexpr static f64 LIGHT_BIAS {0.001};  // lights a wall from the open side so the cell lookup never lands inside the wall

    auto const& level {*_level};
    auto const& player {*_player};

    auto const& hit {col.Hit};

    if (col.Top >= col.Bottom) { return; }

    f64 const dist {std::max(hit.Distance, MIN_RAY_DIST)};
    f64 const rowsPerUnit {_projPlaneDist / dist};
    f64 const wallBottomRow {std::round(col.CenterY + ((player.EyeZ - col.WallBase) * rowsPerUnit))};

    auto const* tex {_cache.texture(hit.Texture, 0)};
    if (tex == nullptr) { return; }
    i32 const texX {std::clamp(static_cast<i32>((1.0 - hit.SegmentT) * static_cast<f64>(WALL_SIZE.Width)), 0, WALL_SIZE.Width - 1)};
    f64 const texStep {static_cast<f64>(WALL_SIZE.Height) / rowsPerUnit};
    f64       texPos {static_cast<f64>(WALL_SIZE.Height) + ((static_cast<f64>(col.Top) - wallBottomRow) * texStep)};

    auto const surfacePos {player.Position + (col.RayDir * std::max(hit.Distance - LIGHT_BIAS, 0.0))};

    // darken near inside corners of normal walls: where the neighbouring cell along the wall is solid
    f64 cornerShade {1.0};
    if (_quality.ContactShadows && col.ToOpen != point_i::Zero && std::holds_alternative<normal_wall>(level.get_cell(col.Cell))) {
        bool const    xFace {col.ToOpen.X != 0};
        auto const    hitPoint {player.Position + (col.RayDir * hit.Distance)};
        f64 const     frac {std::clamp((xFace ? hitPoint.Y - col.Cell.Y : hitPoint.X - col.Cell.X), 0.0, 1.0)};
        point_i const open {col.Cell.X + col.ToOpen.X, col.Cell.Y + col.ToOpen.Y};
        auto const    lowStep {xFace ? point_i {0, -1} : point_i {-1, 0}};

        for (i32 sign : {1, -1}) {
            f64 const edgeDist {sign == 1 ? frac : 1.0 - frac};
            if (edgeDist >= CORNER_WIDTH) { continue; }

            point_i const neighbor {open.X + (lowStep.X * sign), open.Y + (lowStep.Y * sign)};
            if (!map_t::Size.contains(neighbor) || !std::holds_alternative<normal_wall>(level.get_cell(neighbor))) { continue; }

            f64 const t {1.0 - (edgeDist / CORNER_WIDTH)};
            cornerShade *= 1.0 - (CORNER_STRENGTH * t * t);
        }
    }

    for (i32 y {col.Top}; y < col.Bottom; y++) {
        i32 const texY {((static_cast<i32>(std::floor(texPos)) % WALL_SIZE.Height) + WALL_SIZE.Height) % WALL_SIZE.Height};
        texPos += texStep;
        i32 const srcIdx {(texX + (texY * WALL_SIZE.Width)) * TEXTURE_BPP};

        f64 const worldZ {std::max(col.WallBase, std::min(col.WallBase + ((wallBottomRow - y) / rowsPerUnit), col.WallTop))};
        draw_surface_pixel(col.X, y, tex, srcIdx, surfacePos, worldZ, cornerShade);
        _objectDepthBuffer[PixelIndex(_screenSize.Width, col.X, y)] = hit.Distance;
    }
}

void raycaster::draw_sprites(i32 screenCenterY)
{
    auto&       level {*_level};
    auto const& player {*_player};

    rect_i const screenRect {point_i::Zero, _screenSize};

    u32* screenBuf {_screen.data()};
    for (auto& go : level.objects()) {
        auto* spr {go.sprite()};
        if (!spr) { continue; }

        spr->Seen = false;

        auto const p {_camera.project(spr->Position)};
        if (p.Depth <= 0) { continue; }
        f64 const depth {p.Depth};

        i32 const    spriteScreenX {static_cast<i32>(p.ScreenX)};
        auto const   spriteSize {spr->screen_size(p.Scale)};
        rect_i const spriteRect {point_i {spriteScreenX - (spriteSize.Width / 2),
                                          spr->screen_bottom(p.Scale, screenCenterY, player.EyeZ) - spriteSize.Height},
                                 spriteSize};
        if (!screenRect.intersects(spriteRect, true)) { continue; }

        point_i const drawStart {std::max(spriteRect.left(), 0), std::max(spriteRect.top(), 0)};
        point_i const drawEnd {std::min(spriteRect.right(), _screenSize.Width), std::min(spriteRect.bottom(), _screenSize.Height)};

        i32 const   facing {spr->facing_index(player.Position)};
        auto const* tex {_cache.texture(spr->Texture, facing)};
        auto const  texSize {_cache.texture_size(spr->Texture, facing)};

        f64 const texStepY {static_cast<f64>(texSize.Height) / spriteSize.Height};
        f64 const texPosYStart {(drawStart.Y - spriteRect.top()) * texStepY};

        auto const spriteCell {WorldToCell(spr->Position)};
        auto const spriteTint {accumulate_light(spr->Position, spr->BaseZ, spriteCell, spr->LightMask)};
        bool const inLight {IsWithinPlayerLight(player, spriteCell)};

        for (i32 x {drawStart.X}; x < drawEnd.X; ++x) {
            if (depth >= _wallDepthBuffer[x]) { continue; }

            i32 const texX {std::clamp(((x - spriteRect.left()) * texSize.Width) / spriteSize.Width, 0, texSize.Width - 1)};
            f64       texPos {texPosYStart};

            for (i32 y {drawStart.Y}; y < drawEnd.Y; ++y) {
                i32 const texY {std::clamp(static_cast<i32>(texPos), 0, texSize.Height - 1)};
                texPos += texStepY;

                i32 const texOffset {(texX + (texY * texSize.Width)) * TEXTURE_BPP};
                if (IsMagenta(tex, texOffset)) { continue; }

                usize const idx {PixelIndex(_screenSize.Width, x, y)};
                if (depth < _objectDepthBuffer[idx]) {
                    _objectDepthBuffer[idx] = depth;
                    if (inLight) { spr->Seen = true; }
                    CopyPixel(screenBuf, idx, tex, texOffset, DitherTint(spriteTint, x, y, _quality));
                }
            }
        }
    }
}

void raycaster::draw_voxel_objects(i32 screenCenterY)
{
    struct screen_bounds {
        i32 XStart {};
        i32 XEnd {};
        i32 YStart {};
        i32 YEnd {};
        f64 Nearest {};     // nothing of the object is closer than this
        f64 MaxWallDist {}; // farthest wall behind its columns

        auto area() const -> i32 { return (XEnd - XStart) * (YEnd - YStart); }
    };

    level&        level {*_level};
    player const& player {*_player};

    // screen x, screen y and depth of a world point; depth is 0 for anything not safely in front of the camera
    auto const project {[&](point3_d const& world) -> std::tuple<f64, f64, f64> {
        auto const p {_camera.project({world.X, world.Y})};
        if (p.Depth <= 1e-6) { return {0.0, 0.0, 0.0}; }

        return {p.ScreenX, _camera.screen_y(world.Z, p), p.Depth};
    }};

    rect_i const screenRect {point_i::Zero, _screenSize};
    u32*         screenBuf {_screen.data()};

    // where an object lands on screen, nothing if it is off screen or hidden behind walls
    auto const measure {[&](auto* vo) -> std::optional<screen_bounds> {
        auto const voCell {WorldToCell(vo->Position)};
        f64 const  floorZ {HeightAt(level.slopes_at(voCell).Floor, voCell, vo->Position)};
        auto const xf {vo->make_transform()};

        f64  bboxMinX {INFINITE_DIST}, bboxMaxX {-INFINITE_DIST};
        f64  bboxMinY {INFINITE_DIST}, bboxMaxY {-INFINITE_DIST};
        f64  bboxMinDepth {INFINITE_DIST};
        bool anyVisible {false};
        bool behindCamera {false};

        for (auto const& corner : vo->Grid->corners()) {
            auto const     worldCorner {xf.to_world(corner)};
            point3_d const adjustedCorner {.X = worldCorner.X, .Y = worldCorner.Y, .Z = worldCorner.Z + floorZ};
            auto const [sx, sy, depth] {project(adjustedCorner)};
            if (depth <= 0.0) {
                behindCamera = true;
                continue;
            }

            anyVisible   = true;
            bboxMinX     = std::min(bboxMinX, sx);
            bboxMaxX     = std::max(bboxMaxX, sx);
            bboxMinY     = std::min(bboxMinY, sy);
            bboxMaxY     = std::max(bboxMaxY, sy);
            bboxMinDepth = std::min(bboxMinDepth, depth);
        }
        if (!anyVisible) { return std::nullopt; }

        rect_i const bboxRect {point_i {static_cast<i32>(bboxMinX), static_cast<i32>(bboxMinY)},
                               size_i {static_cast<i32>(bboxMaxX - bboxMinX) + 1, static_cast<i32>(bboxMaxY - bboxMinY) + 1}};
        if (!screenRect.intersects(bboxRect, true)) { return std::nullopt; }

        screen_bounds b {.XStart  = std::clamp(bboxRect.Position.X, 0, _screenSize.Width),
                         .XEnd    = std::clamp(bboxRect.Position.X + bboxRect.Size.Width, 0, _screenSize.Width),
                         .YStart  = std::clamp(bboxRect.Position.Y, 0, _screenSize.Height),
                         .YEnd    = std::clamp(bboxRect.Position.Y + bboxRect.Size.Height, 0, _screenSize.Height),
                         .Nearest = behindCamera ? 0.0 : bboxMinDepth};

        for (i32 x {b.XStart}; x < b.XEnd; ++x) { b.MaxWallDist = std::max(b.MaxWallDist, _wallDepthBuffer[x]); }
        if (b.Nearest >= b.MaxWallDist) { return std::nullopt; }

        return b;
    }};

    // nearest first, so pixels that are already covered reject the objects behind them
    _voxelOrder.clear();
    for (auto& go : level.objects()) {
        auto* const vo {go.voxel()};
        if (!vo) { continue; }

        vo->Seen = false;
        point_d const toObject {vo->Position - player.Position};
        _voxelOrder.emplace_back(toObject.dot(toObject), &go);
    }
    std::ranges::sort(_voxelOrder, [](auto const& a, auto const& b) { return a.first < b.first; });

    std::vector<std::optional<screen_bounds>> measured;
    measured.reserve(_voxelOrder.size());

    f64 totalArea {0.0};
    for (auto const& entry : _voxelOrder) {
        auto* const vo {entry.second->voxel()};
        measured.push_back(vo->Grid ? measure(vo) : std::nullopt);
        if (measured.back()) { totalArea += measured.back()->area(); }
    }
    i32 const stride {totalArea > _quality.VoxelPixelBudget ? static_cast<i32>(std::ceil(std::sqrt(totalArea / _quality.VoxelPixelBudget))) : 1};

    for (usize oi {0}; oi < _voxelOrder.size(); ++oi) {
        auto* const vo {_voxelOrder[oi].second->voxel()};

        auto const& bounds {measured[oi]};
        if (!bounds) { continue; }

        auto const [xStart, xEnd, yStart, yEnd, nearest, maxWallDist] {*bounds};
        auto const voCell {WorldToCell(vo->Position)};
        f64 const  floorZ {HeightAt(level.slopes_at(voCell).Floor, voCell, vo->Position)};
        auto const xf {vo->make_transform()};

        auto const localOrigin {xf.to_local(point3_d {.X = player.Position.X, .Y = player.Position.Y, .Z = player.EyeZ - floorZ})};

        auto const edge_len_sq {[&](i32 col) {
            auto const d {ComputeCameraRayDir(col, _screenSize.Width, player)};
            return d.dot(d);
        }};
        f64 const  maxLenSq {std::max(edge_len_sq(bounds->XStart), edge_len_sq(xEnd - 1))};
        f64 const  maxSlope {std::max(std::abs(screenCenterY - yStart), std::abs(yEnd - screenCenterY)) / _projPlaneDist};
        f64 const  maxT {(maxWallDist * std::sqrt(maxLenSq + (maxSlope * maxSlope))) + vo->world_diagonal()};

        i32 const strideCols {((xEnd - bounds->XStart) + stride - 1) / stride};

        std::atomic_bool drawn {false};

        _taskManager.run_parallel([&](par_task const& ctx) {
            bool wrote {false};

            for (isize scol {ctx.Start}; scol < ctx.End; ++scol) {
                i32 const x {static_cast<i32>(PixelIndex(stride, xStart, scol))};
                if (x >= xEnd) { continue; }

                i32 const xBlockEnd {std::min(x + stride, xEnd)};

                // the whole object is behind the walls in these columns
                f64 blockWall {0.0};
                for (i32 bx {x}; bx < xBlockEnd; ++bx) { blockWall = std::max(blockWall, _wallDepthBuffer[bx]); }
                if (nearest >= blockWall) { continue; }

                auto const rayDir2D {ComputeCameraRayDir(x + ((stride - 1) / 2), _screenSize.Width, player)};

                for (i32 y {yStart}; y < yEnd; y += stride) {
                    // already covered by something nearer than the whole object
                    if (stride == 1 && nearest >= _objectDepthBuffer[PixelIndex(_screenSize.Width, x, y)]) { continue; }

                    f64 const  rayDirZ {(screenCenterY - (y + (0.5 * stride))) / _projPlaneDist};
                    auto const rayDir3D {point3_d {.X = rayDir2D.X, .Y = rayDir2D.Y, .Z = rayDirZ}.normalized()};

                    auto const localDir {xf.dir_to_local(rayDir3D)};

                    auto const hit {vo->Grid->raycast(localOrigin, localDir, maxT)};
                    if (!hit.Hit) { continue; }

                    auto const     localHitPos {localOrigin + (localDir * hit.T)};
                    auto const     worldHit {xf.to_world(localHitPos)};
                    point3_d const adjustedWorldHit {.X = worldHit.X, .Y = worldHit.Y, .Z = worldHit.Z + floorZ};
                    point_d const  worldHitXY {worldHit.X, worldHit.Y};
                    auto const [sx, sy, depth] {project(adjustedWorldHit)};

                    if (depth <= 0.0) { continue; }
                    if (depth >= _wallDepthBuffer[x]) { continue; }

                    auto const hitCellForLight {WorldToCell(worldHitXY)};
                    f64 const  aoFactor {_quality.VoxelAo ? vo->Grid->face_ao(hit, localHitPos) : 1.0};

                    auto tint {accumulate_light(worldHitXY, adjustedWorldHit.Z, hitCellForLight, LIGHT_LAYERS_ALL)};
                    tint.R *= aoFactor;
                    tint.G *= aoFactor;
                    tint.B *= aoFactor;

                    i32 const yBlockEnd {std::min(y + stride, yEnd)};
                    for (i32 by {y}; by < yBlockEnd; ++by) {
                        for (i32 bx {x}; bx < xBlockEnd; ++bx) {
                            if (depth >= _wallDepthBuffer[bx]) { continue; }

                            usize const depthIndex {PixelIndex(_screenSize.Width, bx, by)};
                            if (depth >= _objectDepthBuffer[depthIndex]) { continue; }

                            _objectDepthBuffer[depthIndex] = depth;
                            screenBuf[depthIndex]          = ApplyTint(hit.Color.R, hit.Color.G, hit.Color.B, DitherTint(tint, bx, by, _quality));
                            wrote                          = true;
                        }
                    }
                }
            }

            if (wrote) { drawn.store(true, std::memory_order_relaxed); }
        },
                                  strideCols);

        vo->Seen = drawn.load(std::memory_order_relaxed) && IsWithinPlayerLight(player, voCell);
    }
}

void raycaster::draw_motes()
{
    constexpr static f64 MIN_LIGHT {0.04}; // motes in less light than this are skipped

    auto const& level {*_level};

    static auto add_pixel {[](u32 px, u32 add, f64 t) -> u32 {
        u32 const w {static_cast<u32>(std::clamp(t, 0.0, 1.0) * 256.0)};
        u32 const r {std::min((px & 0xFFu) + (((add & 0xFFu) * w) >> 8), 255u)};
        u32 const g {std::min(((px >> 8) & 0xFFu) + ((((add >> 8) & 0xFFu) * w) >> 8), 255u)};
        u32 const b {std::min(((px >> 16) & 0xFFu) + ((((add >> 16) & 0xFFu) * w) >> 8), 255u)};
        return 0xFF000000u | (b << 16) | (g << 8) | r;
    }};

    static auto frac {[](f64 v) -> f64 { return v - std::floor(v); }};

    auto const& player {*_player};

    f64 const t {static_cast<f64>(level.time().count()) / 1000.0};
    f64 const lightRange {player.Settings.LightRange};

    i32 const     reach {static_cast<i32>(std::ceil(lightRange))};
    point_i const center {WorldToCell(player.Position)};
    u32*          screenBuf {_screen.data()};

    for (i32 cy {center.Y - reach}; cy <= center.Y + reach; ++cy) {
        for (i32 cx {center.X - reach}; cx <= center.X + reach; ++cx) {
            point_i const cell {cx, cy};
            if (!map_t::Size.contains(cell) || std::holds_alternative<normal_wall>(level.get_cell(cell)) || !IsWithinPlayerLight(player, cell)) { continue; }

            auto const& motes {level.motes_at(cell)};
            if (motes.Count == 0) { continue; }

            // skip cells that are entirely behind the player or outside the view cone
            {
                auto const c {_camera.project(point_d {cx + 0.5, cy + 0.5})};
                f64 const  slack {(0.7072 + motes.Drift + motes.Size) * _camera.max_shift_per_unit()}; // cell half-diagonal, drift and mote size
                if (c.Depth < -slack || std::abs(c.Lateral) > c.Depth + (2.0 * slack)) { continue; }
            }

            auto const slopes {level.slopes_at(cell)};

            for (u32 i {0}; i < motes.Count; ++i) {
                point_d const pos {cx + frac(HashUnit(cx, cy, i, 3) + (motes.Wind.X * t)) + (motes.Drift * std::sin((t * motes.DriftSpeed) + (HashUnit(cx, cy, i, 5) * TAU))),
                                   cy + frac(HashUnit(cx, cy, i, 4) + (motes.Wind.Y * t)) + (motes.Drift * std::sin((t * motes.DriftSpeed * 0.8) + (HashUnit(cx, cy, i, 6) * TAU)))};

                // falls (or rises) and wraps inside the height band
                f64 const fall {HashUnit(cx, cy, i, 7) - (t * motes.FallSpeed * (1.0 + (motes.SpeedJitter * ((2.0 * HashUnit(cx, cy, i, 8)) - 1.0))))};
                f64 const floorZ {HeightAt(slopes.Floor, cell, pos)};
                f64 const ceilZ {HeightAt(slopes.Ceiling, cell, pos)};
                f64 const z {std::min(floorZ + motes.MinHeight + (frac(fall) * (motes.MaxHeight - motes.MinHeight)), ceilZ)};

                auto const p {_camera.project(pos)};
                f64 const  depth {p.Depth};
                if (depth <= 0.05) { continue; }

                i32 const sx {static_cast<i32>(std::floor(p.ScreenX))};
                i32 const sy {static_cast<i32>(std::floor(_camera.screen_y(z, p)))};

                i32 const widthPx {std::max(1, static_cast<i32>(std::round(motes.Size * p.Scale)))};
                i32 const lengthPx {std::max(widthPx, static_cast<i32>(std::round(motes.Length * p.Scale)))};
                i32 const left {sx - (widthPx / 2)};
                if (left >= _screenSize.Width || left + widthPx <= 0 || sy - lengthPx >= _screenSize.Height || sy < 0) { continue; }

                auto tint {accumulate_light(pos, z, cell, LIGHT_LAYERS_ALL)};
                tint.R += (1.0 - tint.R) * motes.Emissive;
                tint.G += (1.0 - tint.G) * motes.Emissive;
                tint.B += (1.0 - tint.B) * motes.Emissive;
                if (std::max({tint.R, tint.G, tint.B}) < MIN_LIGHT) { continue; }

                f64 twinkle {1.0};
                if (motes.Twinkle > 0.0) {
                    f64 const pulse {0.5 + (0.5 * std::sin((t * motes.TwinkleSpeed * TAU * (0.5 + HashUnit(cx, cy, i, 9))) + (HashUnit(cx, cy, i, 10) * TAU)))};
                    twinkle = 1.0 - (motes.Twinkle * pulse);
                }

                color_d base {.R = 1.0, .G = 1.0, .B = 1.0};
                if (!motes.Colors.empty()) {
                    f64 const   pick {HashUnit(cx, cy, i, 11) * static_cast<f64>(motes.Colors.size() - 1)};
                    usize const lo {static_cast<usize>(pick)};
                    usize const hi {std::min(lo + 1, motes.Colors.size() - 1)};
                    f64 const   mix {pick - static_cast<f64>(lo)};
                    auto const& c0 {motes.Colors[lo]};
                    auto const& c1 {motes.Colors[hi]};
                    base = c0.lerp(c1, mix);
                }

                f64 const fade {1.0 - std::min(depth / lightRange, 1.0)};
                u32 const moteColor {ApplyTint(255, 255, 255, {.R = base.R * tint.R, .G = base.G * tint.G, .B = base.B * tint.B})};

                for (i32 py {std::max(sy - lengthPx + 1, 0)}; py <= std::min(sy, _screenSize.Height - 1); ++py) {
                    f64 const trail {lengthPx > widthPx ? 1.0 - (static_cast<f64>(sy - py) / lengthPx) : 1.0};
                    f64 const alpha {motes.Alpha * fade * twinkle * trail};

                    for (i32 px {std::max(left, 0)}; px < std::min(left + widthPx, _screenSize.Width); ++px) {
                        if (depth >= _wallDepthBuffer[px]) { continue; }

                        usize const idx {PixelIndex(_screenSize.Width, px, py)};
                        if (depth >= _objectDepthBuffer[idx]) { continue; }

                        screenBuf[idx] = motes.Additive ? add_pixel(screenBuf[idx], moteColor, alpha) : BlendPixel(screenBuf[idx], moteColor, alpha);
                    }
                }
            }
        }
    }
}

auto raycaster::sky_pixel(i32 y, point_d const& rayDir) const -> u32
{
    static auto dot {[](point3_d const& a, point3_d const& b) { return (a.X * b.X) + (a.Y * b.Y) + (a.Z * b.Z); }};

    static auto smooth {[](f64 e0, f64 e1, f64 x) -> f64 {
        f64 const u {std::clamp((x - e0) / (e1 - e0), 0.0, 1.0)};
        return u * u * (3.0 - (2.0 * u));
    }};

    static auto fbm {[](auto const& noise, f64 nx, f64 ny, i32 octaves) -> f64 {
        f64 sum {0.0};
        f64 amp {0.5};
        f64 norm {0.0};
        for (i32 o {0}; o < octaves; ++o) {
            sum += noise({static_cast<f32>(nx), static_cast<f32>(ny)}) * amp;
            norm += amp;
            amp *= 0.5;
            nx = (nx * 2.0) + 7.31;
            ny = (ny * 2.0) + 3.17;
        }
        return sum / norm;
    }};

    auto const& sky {_level->sky()};
    f64 const   azimuth {sky.Stars ? std::atan2(rayDir.Y, rayDir.X) : 0.0};

    // view direction; the horizon is the unbobbed screen center
    f64 const      zRow {((_screenSize.Height / 2.0) - y) / _projPlaneDist};
    f64 const      invNorm {1.0 / std::sqrt(rayDir.dot(rayDir) + (zRow * zRow))};
    point3_d const dir {.X = rayDir.X * invNorm, .Y = rayDir.Y * invNorm, .Z = zRow * invNorm};

    // gradient
    f64 const h {std::clamp(dir.Z, 0.0, 1.0)};
    color_d   color {sky.Horizon.lerp(sky.Zenith, h)};

    // dusk glow, sun halo and sun disc
    f64 const sunDot {dot(dir, sky.Sun)};
    f64 const s {std::max(sunDot, 0.0)};
    f64 const s2 {s * s};
    f64 const s8 {s2 * s2 * s2 * s2};
    f64 const s32 {s8 * s8 * s8 * s8};

    f64 const duskGlow {sky.Dusk * s8 * (1.0 - (0.7 * h))};
    f64 const haloGlow {0.35 * s32 * (0.3 + (0.7 * sky.Day))};
    color.R += (sky.DuskColor.R * duskGlow) + (sky.SunColor.R * haloGlow);
    color.G += (sky.DuskColor.G * duskGlow) + (sky.SunColor.G * haloGlow);
    color.B += (sky.DuskColor.B * duskGlow) + (sky.SunColor.B * haloGlow);
    if (sunDot >= sky.CosSun) { color = sky.SunColor; }

    // stars: cells of StarSize screen pixels in (azimuth, sin elevation)
    if (sky.Stars) {
        f64 const cellsPerUnit {_projPlaneDist / std::max(sky.StarSize, 1.0)};
        f64 const coverage {std::min(sky.StarSize, 1.0) * std::min(sky.StarSize, 1.0)};
        i32 const gx {static_cast<i32>(std::floor((azimuth + (TAU * 0.5)) * cellsPerUnit))};
        i32 const gy {static_cast<i32>(std::floor(dir.Z * cellsPerUnit))};
        if (HashUnit(gx, gy, 0, 21) < sky.StarDensity) {
            f64 const pulse {0.5 + (0.5 * std::sin((sky.Time * (1.0 + (2.0 * HashUnit(gx, gy, 0, 22)))) + (HashUnit(gx, gy, 0, 23) * TAU)))};
            f64 const bright {coverage * sky.Night * (1.0 - (sky.StarTwinkle * pulse)) * (0.4 + (0.6 * HashUnit(gx, gy, 0, 24)))};
            color.R += bright;
            color.G += bright;
            color.B += bright;
        }
    }

    // moons and planets
    for (auto const& body : sky.Bodies) {
        f64 const d {dot(dir, body.Dir)};
        if (d < body.CosR) { continue; }

        f64 const rho2 {std::max(1.0 - (d * d), 0.0) / (body.SinR * body.SinR)};
        f64 const nz {std::sqrt(std::max(1.0 - rho2, 0.0))};

        color_d surface {body.Color};
        if (body.Noise > 0.0) {
            // disc coordinates, u to the right and v up, from the body direction and world up
            f64 const c {std::max(std::sqrt(std::max(1.0 - (body.Dir.Z * body.Dir.Z), 0.0)), 1e-6)};
            f64 const u {((dir.X * body.Dir.Y) - (dir.Y * body.Dir.X)) / (c * body.SinR)};
            f64 const v {((c * dir.Z) - (body.Dir.Z * ((dir.X * body.Dir.X) + (dir.Y * body.Dir.Y)) / c)) / body.SinR};

            // features compress toward the limb, which makes the disc look round
            f64 const warp {body.NoiseScale / (0.4 + (0.6 * nz))};
            f64 const n {fbm(_noise, (u * warp) + body.NoiseSeed, v * warp, 2)};
            surface = surface.lerp(body.NoiseColor, n * body.Noise);
        }

        if (body.Lit) {
            // phase: surface normal against the direction of the sun
            f64 const nDotL {((sunDot - (d * body.SunDot)) / body.SinR) - (body.SunDot * nz)};
            f64 const light {(body.Ambient + ((1.0 - body.Ambient) * std::clamp(nDotL, 0.0, 1.0)))};
            surface.R *= light;
            surface.G *= light;
            surface.B *= light;
            color = surface.lerp(color, sky.Day);
        } else {
            color = surface;
        }
    }

    // clouds: a flat layer above the viewer, projected along the view direction
    if (sky.CloudCoverage > 0.0 && dir.Z > 0.03) {
        f64 const edge {1.0 - sky.CloudCoverage};
        f64 const lo {edge - sky.CloudSoftness};
        f64 const hi {edge + sky.CloudSoftness};

        f64 const invZ {1.0 / dir.Z};
        f64 const cx {(dir.X * invZ * sky.CloudScale) + (sky.CloudWind.X * sky.Time)};
        f64 const cy {(dir.Y * invZ * sky.CloudScale) + (sky.CloudWind.Y * sky.Time)};

        // the first octave carries the shape; if the second octave at its maximum still stays under the edge, it is clear sky
        f64 const n0 {_noise({static_cast<f32>(cx), static_cast<f32>(cy)})};
        if (((0.5 * n0) + 0.25) / 0.75 > lo) {
            f64 const n1 {_noise({static_cast<f32>((cx * 2.0) + 7.31), static_cast<f32>((cy * 2.0) + 3.17)})};
            f64 const density {((0.5 * n0) + (0.25 * n1)) / 0.75};
            f64 const alpha {smooth(lo, hi, density) * smooth(0.03, 0.3, dir.Z)}; // fades out toward the horizon
            if (alpha > 0.0) {
                // the side facing the sun is thinner, so a cloud is lit where it is thicker than its sun-side neighbor
                f64 const toward {_noise({static_cast<f32>(cx + (sky.Sun.X * 0.3)), static_cast<f32>(cy + (sky.Sun.Y * 0.3))})};
                f64 const lit {std::clamp(0.5 + ((n0 - toward) * 3.0), 0.0, 1.0)};

                f64 const brightness {(0.12 + (0.88 * sky.Day)) * (0.55 + (0.45 * lit))};
                color_d   cloud {sky.CloudColor.lerp(sky.DuskColor, sky.Dusk * s2)};
                cloud.R *= brightness;
                cloud.G *= brightness;
                cloud.B *= brightness;
                color = color.lerp(cloud, alpha);
            }
        }
    }

    return ApplyTint(255, 255, 255, color);
}
