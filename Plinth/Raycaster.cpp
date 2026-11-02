// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Raycaster.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <limits>
#include <optional>
#include <span>
#include <tuple>
#include <variant>
#include <vector>

#include "Common.hpp"
#include "Level.hpp"
#include "Light.hpp"
#include "Player.hpp"
#include "TextureCache.hpp"
#include "Voxel.hpp"
#include "Walls.hpp"

constexpr f64 INFINITE_DIST {1e30};   // stand-in for "ray never crosses this axis"
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

static auto ComputeCameraRayDir(isize x, i32 screenWidth, player const& player) -> point_d { return player.Direction + (player.Plane * ((2.0 * (static_cast<f64>(x) + 0.5) / screenWidth) - 1.0)); }

static auto ComputeFalloff(f64 distSq, f64 rangeSq, f64 intensity) -> f64
{
    f64 const d {std::max(distSq, 0.01)};
    f64 const ratio {distSq / rangeSq};
    f64 const base {std::clamp(1.0 - (ratio * ratio), 0.0, 1.0)};
    return intensity * (base * base) / d;
}

static auto FloorHeightAt(slope const& slope, point_i const& cell, point_d const& pos) -> f64
{
    f64 const dx {std::clamp(pos.X - cell.X, 0.0, 1.0)};
    f64 const dy {std::clamp(pos.Y - cell.Y, 0.0, 1.0)};
    return (slope.TL * (1.0 - dx) * (1.0 - dy)) + (slope.TR * dx * (1.0 - dy)) + (slope.BL * (1.0 - dx) * dy) + (slope.BR * dx * dy);
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

static auto IsWithinPlayerLight(player const& player, point_i const& cell) -> bool
{
    f64 const     range {player.Settings.LightRange};
    point_d const toCell {point_d {cell.X + 0.5, cell.Y + 0.5} - player.Position};
    return toCell.dot(toCell) <= range * range;
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
    _zBuffer.assign(static_cast<usize>(_screenSize.Width), 0.0);
    _objectDepthBuffer.assign(static_cast<usize>(_screenSize.area()), 0.0);
}

auto raycaster::accumulate_light(point_d const& surfacePos, f64 surfaceZ, point_i const& cell, u32 lightMask) const -> color_d
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
    CopyPixel(_screen.data(), PixelIndex(_screenSize, x, y), tex, srcIdx, close ? tint : DitherTint(tint, x, y, _quality));
}

auto raycaster::draw(level& level, player& player) -> std::span<u32>
{
    constexpr static f64 EYE_HEIGHT {0.5}; // eye height above the floor under the player

    // valid for the duration of this call only
    _level  = &level;
    _player = &player;

    std::ranges::fill(_objectDepthBuffer, std::numeric_limits<f64>::infinity());

    level.update_light_visibility(static_cast<usize>(_quality.LightLosSamples));

    auto const playerCell {WorldToCell(player.Position)};
    player.EyeZ = FloorHeightAt(level.slope_at(playerCell), playerCell, player.Position) + EYE_HEIGHT;

    i32 const screenCenterY {(_screenSize.Height / 2) + static_cast<i32>(player.BobAmount)};

    _taskManager.run_parallel([&](par_task const& ctx) {
        draw_columns(screenCenterY, static_cast<i32>(ctx.Start), static_cast<i32>(ctx.End));
    },
                              _screenSize.Width);

    draw_sprites(screenCenterY);
    draw_voxel_objects(screenCenterY);

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

        std::optional<wall_hit> hitResult {std::nullopt};
        bool                    side {false};

        crossed.clear();
        crossed.push_back({.Cell = map, .Dist = 0.0});

        // check player cell
        {
            auto const intersect {[&](auto&& c) { return c.intersect({map, player.Position, rayDir, sideDist.X < sideDist.Y, 0.0}); }};
            if (auto const wallHit {std::visit(intersect, level.get_cell(map))}) { hitResult = wallHit; }
        }

        bool const hitInPlayerCell {hitResult.has_value()};

        // DDA
        if (!hitResult) {
            auto const intersect {[&](auto&& c) { return c.intersect({map, player.Position, rayDir, side, !side ? sideDist.X - deltaDist.X : sideDist.Y - deltaDist.Y}); }};

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

                crossed.push_back({.Cell = map, .Dist = !side ? sideDist.X - deltaDist.X : sideDist.Y - deltaDist.Y});

                if (auto const wallHit {std::visit(intersect, level.get_cell(map))}) {
                    hitResult = wallHit;
                    break;
                }
            }
        }

        if (!hitResult) {
            _zBuffer[x] = std::numeric_limits<f64>::infinity();
            continue;
        }

        _zBuffer[x] = hitResult->Distance;

        f64 const  entryDist {crossed.back().Dist};
        auto const prevCell {crossed.size() > 1 ? crossed[crossed.size() - 2].Cell : map}; // open cell the ray came from
        bool const recessed {hitInPlayerCell || hitResult->Distance - entryDist > WALL_RECESS_EPS};
        auto const baseCell {recessed ? map : prevCell};
        f64 const  wallBase {FloorHeightAt(level.slope_at(baseCell), baseCell, player.Position + (rayDir * hitResult->Distance))};

        auto const toOpen {hitInPlayerCell ? point_i::Zero : (side ? point_i {0, -step.Y} : point_i {-step.X, 0})};

        column_hit col {.X        = x,
                        .RayDir   = rayDir,
                        .Hit      = *hitResult,
                        .Cell     = map,
                        .ToOpen   = toOpen,
                        .CenterY  = screenCenterY,
                        .WallBase = wallBase,
                        .Top      = 0,
                        .Bottom   = _screenSize.Height,
                        .Crossed  = crossed};

        draw_floor_ceiling_column(col);
        draw_wall_column(col);
    }
}

void raycaster::draw_floor_ceiling_column(column_hit& col)
{
    constexpr static f64 CONTACT_SHADOW_WIDTH {0.3};
    constexpr static f64 CONTACT_SHADOW_STRENGTH {0.75};

    auto&       level {*_level};
    auto const& player {*_player};

    f64 const   proj {_projPlaneDist};
    f64 const   eyeZ {player.EyeZ};
    isize const x {col.X};
    auto const  rayDir {col.RayDir};
    f64 const   hitDist {col.Hit.Distance};
    u32*        screenBuf {_screen.data()};
    u8 const*   wallTexPtr {nullptr};

    auto const wall_texture {[&]() -> u8 const* {
        if (wallTexPtr == nullptr) { wallTexPtr = _cache.texture(col.Hit.Texture, 0); }
        return wallTexPtr;
    }};

    auto const draw_flat {[&](bool isFloor, cell_surface const& surface, point_i const& cell, f64 tIn, f64 tOut) {
        auto const slope {level.slope_at(cell)};
        f64 const  lift {isFloor ? 0.0 : 1.0};

        auto const p {player.Position + (rayDir * tOut)};
        i32 const  rowFar {static_cast<i32>(std::clamp(std::round(col.CenterY + ((eyeZ - (FloorHeightAt(slope, cell, p) + lift)) * proj / std::max(tOut, MIN_RAY_DIST))), -1.0, static_cast<f64>(_screenSize.Height) + 1.0))};
        i32 const  from {isFloor ? std::max(rowFar, col.Top) : col.Top};
        i32 const  to {isFloor ? col.Bottom : std::min(rowFar, col.Bottom)};
        if (from >= to) { return; }

        auto const dp {player.Position - cell};
        auto const [vx, vy] {rayDir};
        f64 const D {slope.BR - slope.BL - slope.TR + slope.TL};
        f64 const A {D * vx * vy};
        f64 const B {((slope.TR - slope.TL) * vx) + ((slope.BL - slope.TL) * vy) + (D * ((dp.X * vy) + (dp.Y * vx)))};
        f64 const C {slope.TL + ((slope.TR - slope.TL) * dp.X) + ((slope.BL - slope.TL) * dp.Y) + (D * dp.X * dp.Y) + lift};

        i32 const texId {isFloor ? surface.FloorTexture : surface.CeilingTexture};
        u8 const* tex {texId != INVALID_INDEX ? _cache.texture(texId, 0) : nullptr};
        if (tex == nullptr) { tex = wall_texture(); }

        for (i32 y {from}; y < to; ++y) {
            f64 const s {(static_cast<f64>(y) - col.CenterY) / proj};
            f64 const term {s + B};
            f64       t {tOut};

            if (std::abs(A) < 1e-12) {
                if (std::abs(term) > 1e-9) {
                    t = std::clamp((eyeZ - C) / term, tIn, tOut);
                }
            } else {
                f64 const discriminant {(term * term) - (4.0 * A * (C - eyeZ))};
                if (discriminant >= 0.0) {
                    f64 const sqrtDisc {std::sqrt(discriminant)};
                    f64 const t1 {(-term - sqrtDisc) / (2.0 * A)};
                    f64 const t2 {(-term + sqrtDisc) / (2.0 * A)};
                    if (t1 >= tIn && t1 <= tOut) {
                        t = t1;
                    } else if (t2 >= tIn && t2 <= tOut) {
                        t = t2;
                    } else {
                        t = std::clamp(t1, tIn, tOut);
                    }
                }
            }

            auto const pos {player.Position + (rayDir * t)};
            f64 const  z {FloorHeightAt(slope, cell, pos) + lift};

            i32 const texelX {static_cast<i32>(pos.X * WALL_SIZE.Width) & (WALL_SIZE.Width - 1)};
            i32 const texelY {static_cast<i32>(pos.Y * WALL_SIZE.Height) & (WALL_SIZE.Height - 1)};
            i32 const texelOffset {(texelX + (texelY * WALL_SIZE.Width)) * TEXTURE_BPP};

            draw_surface_pixel(x, y, tex, texelOffset, pos, z, 1.0);
            _objectDepthBuffer[PixelIndex(_screenSize, x, y)] = t;

            f64 const wallGap {std::max(hitDist - t, 0.0)};
            if (_quality.ContactShadows && wallGap < CONTACT_SHADOW_WIDTH) {
                f64 const c {1.0 - (wallGap / CONTACT_SHADOW_WIDTH)};
                u32&      pixel {screenBuf[PixelIndex(_screenSize, x, y)]};
                pixel = ScalePixel(pixel, 1.0 - (CONTACT_SHADOW_STRENGTH * c * c));
            }

            if (isFloor && surface.FloorReflectivity > 0.0) {
                f64 const mirrored {z - col.WallBase + ((eyeZ - z) * ((hitDist / std::max(t, MIN_RAY_DIST)) - 1.0))};
                f64 const zRel {std::max(mirrored, 0.0)};
                if (zRel < 1.0) {
                    u8 const* wallTex {wall_texture()};

                    i32 const wallTexX {std::clamp(static_cast<i32>((1.0 - col.Hit.SegmentT) * static_cast<f64>(WALL_SIZE.Width)), 0, WALL_SIZE.Width - 1)};
                    i32 const wallTexY {std::clamp(static_cast<i32>((1.0 - zRel) * static_cast<f64>(WALL_SIZE.Height)), 0, WALL_SIZE.Height - 1)};
                    i32 const wallSrc {(wallTexX + (wallTexY * WALL_SIZE.Width)) * TEXTURE_BPP};

                    point_d const wallPoint {player.Position + (rayDir * hitDist)};
                    auto const    wallTint {accumulate_light(wallPoint, col.WallBase + zRel, WorldToCell(wallPoint), LIGHT_LAYERS_ALL)};
                    u32 const     reflected {ApplyTint(wallTex[wallSrc + 0], wallTex[wallSrc + 1], wallTex[wallSrc + 2], DitherTint(wallTint, x, y, _quality))};

                    u32& pixel {screenBuf[PixelIndex(_screenSize, x, y)]};
                    pixel = BlendPixel(pixel, reflected, surface.FloorReflectivity * (1.0 - zRel));
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

        if (isLast && hitDist - t0 <= WALL_RECESS_EPS) { break; } // wall on the cell face: it covers the rest of the column

        f64 const t1 {isLast ? hitDist : col.Crossed[i + 1].Dist};
        if (t1 <= t0) { continue; }

        auto const cell {col.Crossed[i].Cell};
        auto const surface {level.surface(cell)};

        i32 const bottomBefore {col.Bottom};
        i32 const topBefore {col.Top};

        draw_flat(true, surface, cell, t0, t1);
        draw_flat(false, surface, cell, t0, t1);

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
    f64 const wallTopRow {std::round(col.CenterY + ((player.EyeZ - (col.WallBase + 1.0)) * _projPlaneDist / dist))};
    f64 const wallBottomRow {std::round(col.CenterY + ((player.EyeZ - col.WallBase) * _projPlaneDist / dist))};
    f64 const wallHeight {std::max(wallBottomRow - wallTopRow, 1.0)};

    auto const* tex {_cache.texture(hit.Texture, 0)};
    i32 const   texX {std::clamp(static_cast<i32>((1.0 - hit.SegmentT) * static_cast<f64>(WALL_SIZE.Width)), 0, WALL_SIZE.Width - 1)};
    f64 const   texStep {static_cast<f64>(WALL_SIZE.Height) / wallHeight};
    f64         texPos {(static_cast<f64>(col.Top) - wallTopRow) * texStep};

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
        i32 const texY {((static_cast<i32>(texPos) % WALL_SIZE.Height) + WALL_SIZE.Height) % WALL_SIZE.Height};
        texPos += texStep;
        i32 const srcIdx {(texX + (texY * WALL_SIZE.Width)) * TEXTURE_BPP};

        f64 const worldZ {col.WallBase + std::clamp((wallBottomRow - y) / wallHeight, 0.0, 1.0)};
        draw_surface_pixel(col.X, y, tex, srcIdx, surfacePos, worldZ, cornerShade);
        _objectDepthBuffer[PixelIndex(_screenSize, col.X, y)] = hit.Distance;
    }
}

void raycaster::draw_sprites(i32 screenCenterY)
{
    auto&       level {*_level};
    auto const& player {*_player};

    f64 const    invDet {1.0 / player.Plane.cross(player.Direction)};
    rect_i const screenRect {point_i::Zero, _screenSize};

    u32* screenBuf {_screen.data()};
    for (auto& go : level.objects()) {
        auto* spr {go.sprite()};
        if (!spr) { continue; }

        spr->Seen = false;

        auto const relPos {spr->Position - player.Position};

        f64 const transformX {invDet * relPos.cross(player.Direction)};
        f64 const transformY {invDet * player.Plane.cross(relPos)};
        if (transformY <= 0) { continue; }

        i32 const  spriteScreenX {static_cast<i32>((_screenSize.Width / 2.0) * (1.0 + (transformX / transformY)))};
        f64 const  scale {_projPlaneDist / transformY};
        auto const spriteSize {spr->screen_size(scale)};

        i32 const spriteLeft {spriteScreenX - (spriteSize.Width / 2)};
        i32 const spriteBottom {spr->screen_bottom(scale, screenCenterY, player.EyeZ)};
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
        auto const spriteTint {accumulate_light(spr->Position, spr->BaseZ, spriteCell, spr->LightMask)};
        bool const inLight {IsWithinPlayerLight(player, spriteCell)};

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
                    if (inLight) { spr->Seen = true; }
                    CopyPixel(screenBuf, PixelIndex(_screenSize, x, y), tex, texOffset, DitherTint(spriteTint, x, y, _quality));
                }
            }
        }
    }
}

void raycaster::draw_voxel_objects(i32 screenCenterY)
{
    level&        level {*_level};
    player const& player {*_player};

    f64 const  invDet {1.0 / player.Plane.cross(player.Direction)};
    auto const project {[&](point3_d const& world) -> std::tuple<f64, f64, f64> {
        point_d const relPos {world.X - player.Position.X, world.Y - player.Position.Y};

        f64 const transformX {invDet * relPos.cross(player.Direction)};
        f64 const transformY {invDet * player.Plane.cross(relPos)};

        if (transformY <= 1e-6) { return {0.0, 0.0, 0.0}; }

        f64 const screenX {(_screenSize.Width / 2.0) * (1.0 + (transformX / transformY))};
        f64 const scale {_projPlaneDist / transformY};
        f64 const screenY {screenCenterY - ((world.Z - player.EyeZ) * scale)};

        return {screenX, screenY, transformY};
    }};

    rect_i const screenRect {point_i::Zero, _screenSize};
    u32*         screenBuf {_screen.data()};

    for (auto& go : level.objects()) {
        auto* const vo {go.voxel()};
        if (!vo) { continue; }

        vo->Seen = false;
        if (!vo->Grid) { continue; }

        auto const voCell {WorldToCell(vo->Position)};
        f64 const  floorZ {FloorHeightAt(level.slope_at(voCell), voCell, vo->Position)};

        auto const xf {vo->make_transform()};

        f64  bboxMinX {std::numeric_limits<f64>::infinity()}, bboxMaxX {-std::numeric_limits<f64>::infinity()};
        f64  bboxMinY {std::numeric_limits<f64>::infinity()}, bboxMaxY {-std::numeric_limits<f64>::infinity()};
        f64  bboxMinDepth {std::numeric_limits<f64>::infinity()};
        bool anyVisible {false};

        for (auto const& corner : vo->Grid->corners()) {
            auto const     worldCorner {xf.to_world(corner)};
            point3_d const adjustedCorner {.X = worldCorner.X, .Y = worldCorner.Y, .Z = worldCorner.Z + floorZ};
            auto const [sx, sy, depth] {project(adjustedCorner)};
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

        auto const localOrigin {xf.to_local(point3_d {.X = player.Position.X, .Y = player.Position.Y, .Z = player.EyeZ - floorZ})};

        f64 maxWallDist {0.0};
        for (i32 x {xStart}; x < xEnd; ++x) { maxWallDist = std::max(maxWallDist, _zBuffer[x]); }
        if (bboxMinDepth >= maxWallDist) { continue; }

        f64 const maxT {maxWallDist + vo->world_diagonal()};

        i32 const colCount {xEnd - xStart};
        i32 const rowCount {yEnd - yStart};
        i32 const bboxArea {colCount * rowCount};
        i32 const stride {bboxArea > _quality.VoxelPixelBudget ? static_cast<i32>(std::ceil(std::sqrt(static_cast<f64>(bboxArea) / _quality.VoxelPixelBudget)))
                                                               : 1};
        i32 const strideCols {(colCount + stride - 1) / stride};

        std::atomic_bool drawn {false};

        _taskManager.run_parallel([&](par_task const& ctx) {
            bool wrote {false};

            for (isize scol {static_cast<isize>(ctx.Start)}; scol < static_cast<isize>(ctx.End); ++scol) {
                i32 const x {xStart + (static_cast<i32>(scol) * stride)};
                if (x >= xEnd) { continue; }

                auto const rayDir2D {ComputeCameraRayDir(x + ((stride - 1) / 2), _screenSize.Width, player)};
                i32 const  xBlockEnd {std::min(x + stride, xEnd)};

                for (i32 y {yStart}; y < yEnd; y += stride) {
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
                    if (depth >= _zBuffer[x]) { continue; }

                    auto const hitCellForLight {WorldToCell(worldHitXY)};
                    f64 const  aoFactor {_quality.VoxelAo ? vo->Grid->face_ao(hit, localHitPos) : 1.0};

                    auto tint {accumulate_light(worldHitXY, adjustedWorldHit.Z, hitCellForLight, LIGHT_LAYERS_ALL)};
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
