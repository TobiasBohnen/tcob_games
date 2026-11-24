// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Raycaster.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <numbers>
#include <optional>
#include <span>
#include <utility>
#include <variant>
#include <vector>

#include "CellWalk.hpp"
#include "Common.hpp"
#include "Level.hpp"
#include "LevelObject.hpp"
#include "Map.hpp"
#include "Player.hpp"
#include "TextureCache.hpp"
#include "Voxel.hpp"
#include "Walls.hpp"

constexpr f64 INV_255 {1.0 / 255.0};
constexpr f64 WALL_RECESS_EPS {1e-6};     // a wall hit further into its cell than this is recessed (door, diagonal)
constexpr f64 MIN_RAY_DIST {1e-4};        // keeps row projection finite for surfaces at the player
constexpr f64 SEEN_MIN_LIGHT {0.02};      // a surface lit dimmer than this (brightest channel) does not count as seen
constexpr f64 SURFACE_LIGHT_BIAS {0.001}; // lights a surface from the open side so the cell lookup never lands inside the wall

static auto ApplyTint(u8 r, u8 g, u8 b, color_d tint) -> u32
{
    u8 const tr {static_cast<u8>(std::min(r * tint.R, 255.0))};
    u8 const tg {static_cast<u8>(std::min(g * tint.G, 255.0))};
    u8 const tb {static_cast<u8>(std::min(b * tint.B, 255.0))};
    return 0xFF000000u | (static_cast<u32>(tb) << 16) | (static_cast<u32>(tg) << 8) | static_cast<u32>(tr);
}

static void CopyPixel(u32* dst, usize dstIdx, u8 const* src, usize srcIdx, color_d tint) { dst[dstIdx] = ApplyTint(src[srcIdx + 0], src[srcIdx + 1], src[srcIdx + 2], tint); }

// byte offset of a texel in a texture that is `width` texels wide
static auto TexelOffset(i32 x, i32 y, i32 width) -> i32 { return (x + (y * width)) * TEXTURE_BPP; }

// texel of a world coordinate on a texture that repeats once per world unit
static auto WrapTexel(f64 coord, i32 size) -> i32 { return static_cast<i32>(coord * size) & (size - 1); }

// column of the wall texture that a wall hit lands on
static auto WallTexelX(wall_hit const& hit) -> i32 { return std::clamp(static_cast<i32>((1.0 - hit.SegmentT) * static_cast<f64>(WALL_SIZE.Width)), 0, WALL_SIZE.Width - 1); }

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

static auto DitherTint(color_d tint, isize x, isize y) -> color_d
{
    constexpr static f64 DITHERING_LIGHT_BANDS {8};

    constexpr static std::array<f64, 16> BAYER_4X4 {0.03125, 0.53125, 0.15625, 0.65625,
                                                    0.78125, 0.28125, 0.90625, 0.40625,
                                                    0.21875, 0.71875, 0.09375, 0.59375,
                                                    0.96875, 0.46875, 0.84375, 0.34375};

    f64 const  invBands {1.0 / DITHERING_LIGHT_BANDS};
    f64 const  threshold {BAYER_4X4[static_cast<usize>(((y & 3) << 2) | (x & 3))]};
    auto const quantize {[&](f64 t) { return std::floor((t * DITHERING_LIGHT_BANDS) + threshold) * invBands; }};
    return color_d {.R = quantize(tint.R), .G = quantize(tint.G), .B = quantize(tint.B)};
}

static auto IsWithinPlayerLight(player const& player, point_i cell) -> bool
{
    f64 const     range {player.Settings.LightRange};
    point_d const toCell {point_d {cell.X + 0.5, cell.Y + 0.5} - player.Position};
    return toCell.length_squared() <= range * range;
}

// whether a surface lit with this tint is bright enough to be made out on screen
static auto IsVisiblyLit(color_d tint, f64 minLight = SEEN_MIN_LIGHT) -> bool { return std::max({tint.R, tint.G, tint.B}) >= minLight; }

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
    point_d  Origin;                // where the ray starts: the player's position
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

    auto point_at(f64 t) const -> point_d { return Origin + (RayDir * t); }

    // the point at distance t, nudged back toward the open side so that lighting it never looks inside a wall
    auto lit_point_at(f64 t) const -> point_d { return point_at(std::max(t - SURFACE_LIGHT_BIAS, 0.0)); }
};

raycaster::raycaster(texture_cache& cache)
    : _cache {cache}
    , _taskManager {locate_service<task_manager>()}
{
}

auto raycaster::draw(level& level, player& player) -> std::span<u32>
{
    constexpr static f64 EYE_HEIGHT {0.5}; // eye height above the floor under the player

    std::ranges::fill(_objectDepthBuffer, INFINITE_DIST);

    player.EyeZ = level.floor_z_at(player.Position) + EYE_HEIGHT;

    i32 const screenCenterY {(_screenSize.Height / 2) + static_cast<i32>(player.bob_amount())};

    frame frame {.Level   = level,
                 .Player  = player,
                 .Camera  = {player, _screenSize.Width, _projPlaneDist, screenCenterY},
                 .CenterY = screenCenterY};

    _taskManager.run_parallel([&](par_task const& ctx) {
        draw_columns(frame, static_cast<i32>(ctx.Start), static_cast<i32>(ctx.End));
    },
                              _screenSize.Width);

    draw_sprites(frame);
    draw_voxel_objects(frame);
    draw_motes(frame);

    return _screen;
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

auto raycaster::accumulate_light(frame const& frame, point_d const& surfacePos, f64 surfaceZ, point_i cell, u32 lightMask) const -> color_d
{
    auto const& level {frame.Level};
    auto const& player {frame.Player};

    static auto tone_map {[](f64 x) -> f64 { return x / (1.0 + x); }};

    static auto add_light_contribution {[](point_d const& lightPos, f64 lightHeight, f64 range, f64 intensity, color lightColor, point_d const& surPos, f64 surZ, color_d& total) {
        auto const toLightXY {lightPos - surPos};
        f64 const  dz {lightHeight - surZ};
        f64 const  distSq {toLightXY.length_squared() + (dz * dz)};
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

    auto const& lighting {level.lighting()};
    auto const  lights {lighting.lights()};
    for (u32 li : lighting.lights_at(cell)) {
        auto const& lt {lights[li]};
        if ((lt.Layers & lightMask) == 0) { continue; }
        add_light_contribution(lt.Position.xy(), lt.Position.Z, lt.Range, lt.Intensity, lt.Color, surfacePos, surfaceZ, total);
    }

    return {.R = tone_map(total.R), .G = tone_map(total.G), .B = tone_map(total.B)};
}

void raycaster::draw_surface_pixel(frame const& frame, isize x, isize y, u8 const* tex, isize srcIdx, point_d const& surfacePos, f64 surfaceZ, f64 shade)
{
    constexpr static f64 DITHER_MIN_DIST_SQ {0.5}; // closer than this, surfaces are drawn smooth

    auto const& player {frame.Player};

    auto const cell {WorldToCell(surfacePos)};
    auto const tint {accumulate_light(frame, surfacePos, surfaceZ, cell, LIGHT_LAYERS_ALL) * shade};

    auto const toSurface {surfacePos - player.Position};
    bool const close {toSurface.length_squared() < DITHER_MIN_DIST_SQ};
    auto const ditheredTint {close || !_quality.Dithering ? tint : DitherTint(tint, x, y)};

    u32* screenBuf {_screen.data()};
    CopyPixel(screenBuf, PixelIndex(_screenSize.Width, x, y), tex, srcIdx, ditheredTint);
}

void raycaster::draw_columns(frame const& frame, i32 columnStart, i32 columnEnd)
{
    auto const& level {frame.Level};
    auto const& player {frame.Player};
    auto const& map {level.map()};

    std::vector<column_hit::entry> crossed;
    crossed.reserve(static_cast<usize>(MAP_WIDTH) + MAP_HEIGHT + 2); // a ray cannot cross more cells than this

    for (isize x {columnStart}; x < columnEnd; x++) {
        auto const rayDir {ComputeCameraRayDir(x, _screenSize.Width, player)};

        // Walk the cells in order. The first one is the player's own: the ray starts inside it, so its
        // "entry" is distance 0. For every other cell it is where the ray came in.
        cell_walk               walk {player.Position, rayDir};
        std::optional<wall_hit> hitResult;

        auto const intersect {[&](auto&& c) { return c.intersect({walk.cell(), player.Position, rayDir, walk.side(), walk.entry()}); }};

        crossed.clear();
        for (;;) {
            crossed.push_back({.Cell = walk.cell(), .Dist = walk.entry()});

            hitResult = std::visit(intersect, map.get_cell(walk.cell()));
            if (hitResult) { break; }

            walk.advance();
            if (!map_grid::Size.contains(walk.cell())) { break; }
        }

        if (!hitResult) {
            _wallDepthBuffer[x] = INFINITE_DIST;
            continue;
        }

        _wallDepthBuffer[x] = hitResult->Distance;

        bool const hitInPlayerCell {crossed.size() == 1};
        auto const prevCell {hitInPlayerCell ? walk.cell() : crossed[crossed.size() - 2].Cell}; // open cell the ray came from
        bool const recessed {hitInPlayerCell || ((hitResult->Distance - crossed.back().Dist) > WALL_RECESS_EPS)};
        auto const baseCell {recessed ? walk.cell() : prevCell};
        auto const baseSlope {map.slopes_at(baseCell)};
        auto const hitPoint {player.Position + (rayDir * hitResult->Distance)};

        column_hit col {.X        = x,
                        .Origin   = player.Position,
                        .RayDir   = rayDir,
                        .Hit      = *hitResult,
                        .Cell     = walk.cell(),
                        .ToOpen   = walk.to_previous(),
                        .CenterY  = frame.CenterY,
                        .WallBase = HeightAt(baseSlope.Floor, baseCell, hitPoint),
                        .WallTop  = HeightAt(baseSlope.Ceiling, baseCell, hitPoint),
                        .Top      = 0,
                        .Bottom   = _screenSize.Height,
                        .Crossed  = crossed};

        draw_floor_ceiling_column(frame, col);
        draw_wall_column(frame, col);
    }
}

void raycaster::draw_floor_ceiling_column(frame const& frame, column_hit& col)
{
    static auto scale_pixel {[](u32 px, f64 factor) -> u32 {
        u32 const m {static_cast<u32>(std::clamp(factor, 0.0, 1.0) * 256.0)};
        u32 const rb {(((px & 0x00FF00FFu) * m) >> 8) & 0x00FF00FFu};
        u32 const g {(((px & 0x0000FF00u) * m) >> 8) & 0x0000FF00u};
        return 0xFF000000u | rb | g;
    }};

    constexpr static f64 CONTACT_SHADOW_WIDTH {0.2};
    constexpr static f64 CONTACT_SHADOW_STRENGTH {0.45};
    constexpr static f64 REFLECTION_HEIGHT {1.0}; // floor reflections fade out this far up a wall
    constexpr static f64 MIN_RISER_HEIGHT {1e-4};

    // solid neighbours of a cell
    constexpr static u32 WALL_N {1u << 0}, WALL_S {1u << 1}, WALL_W {1u << 2}, WALL_E {1u << 3};
    constexpr static u32 WALL_NW {1u << 4}, WALL_NE {1u << 5}, WALL_SW {1u << 6}, WALL_SE {1u << 7};

    auto&       map {frame.Level.map()};
    auto const& player {frame.Player};

    u32* screenBuf {_screen.data()};

    // index of this column's pixel in row y, in the screen and the object depth buffer
    auto const pixel_index {[&](i32 y) { return PixelIndex(_screenSize.Width, col.X, y); }};

    // the light a surface at this point receives, from every light
    auto const light_at {[&](point_d const& pos, f64 z) { return accumulate_light(frame, pos, z, WorldToCell(pos), LIGHT_LAYERS_ALL); }};

    // screen row of world height z, seen at ray distance t
    auto const row_at {[&](f64 z, f64 t) {
        f64 const rowsPerUnit {_projPlaneDist / std::max(t, MIN_RAY_DIST)};
        return static_cast<i32>(std::clamp(std::round(col.CenterY + ((player.EyeZ - z) * rowsPerUnit)), -1.0, static_cast<f64>(_screenSize.Height) + 1.0));
    }};

    // rows [from, to) are drawn: floors take them off the bottom of the undrawn window, ceilings off the top
    auto const claim {[&](bool isFloor, i32 from, i32 to) {
        if (isFloor) {
            col.Bottom = std::min(col.Bottom, from);
        } else {
            col.Top = std::max(col.Top, to);
        }
    }};

    // vertical face where a surface does not meet the one of the cell before it
    auto const draw_riser {[&](bool isFloor, point_i prevCell, point_i cell, auto const& prevSlopes, auto const& slopes, cell_surface const& surface, f64 t) {
        i32 const texId {isFloor ? surface.FloorTexture : surface.CeilingTexture};
        if (texId == SKY_TEXTURE) { return; }
        u8 const* tex {_cache.texture(texId, 0)};
        if (tex == nullptr) { return; }

        auto const edge {col.point_at(t)};
        f64 const  prevZ {HeightAt(isFloor ? prevSlopes.Floor : prevSlopes.Ceiling, prevCell, edge)};
        f64 const  hereZ {HeightAt(isFloor ? slopes.Floor : slopes.Ceiling, cell, edge)};

        // only faces turned toward the player: a floor that steps up, a ceiling that steps down
        if ((isFloor ? hereZ - prevZ : prevZ - hereZ) <= MIN_RISER_HEIGHT) { return; }

        i32 const from {std::max(row_at(std::max(hereZ, prevZ), t), col.Top)};
        i32 const to {std::min(row_at(std::min(hereZ, prevZ), t), col.Bottom)};
        if (from >= to) { return; }

        // u runs along the edge: if the cells differ in x, the edge runs along y
        f64 const     along {cell.X != prevCell.X ? edge.Y : edge.X};
        i32 const     texelX {WrapTexel(along, WALL_SIZE.Width)};
        point_d const litPos {col.lit_point_at(t)};

        for (i32 y {from}; y < to; ++y) {
            f64 const z {player.EyeZ - (((static_cast<f64>(y) - col.CenterY) / _projPlaneDist) * t)};
            i32 const texelY {(WALL_SIZE.Height - 1) - (static_cast<i32>(std::floor(z * WALL_SIZE.Height)) & (WALL_SIZE.Height - 1))};

            draw_surface_pixel(frame, col.X, y, tex, TexelOffset(texelX, texelY, WALL_SIZE.Width), litPos, z, 1.0);
            _objectDepthBuffer[pixel_index(y)] = t;
        }

        claim(isFloor, from, to);
    }};

    auto const draw_flat {[&](bool isFloor, cell_surface const& surface, point_i cell, auto const& slopes, f64 tIn, f64 tOut, u32 walls) {
        auto const& slope {isFloor ? slopes.Floor : slopes.Ceiling};

        i32 const rowFar {row_at(HeightAt(slope, cell, col.point_at(tOut)), tOut)};
        i32 const from {isFloor ? std::max(rowFar, col.Top) : col.Top};
        i32 const to {isFloor ? col.Bottom : std::min(rowFar, col.Bottom)};
        if (from >= to) { return; }

        if (!isFloor && surface.CeilingTexture == SKY_TEXTURE) {
            for (i32 y {from}; y < to; ++y) { screenBuf[pixel_index(y)] = sky_pixel(frame, y, col.RayDir); }
            claim(false, from, to);
            return;
        }

        i32 const texId {isFloor ? surface.FloorTexture : surface.CeilingTexture};
        u8 const* tex {_cache.texture(texId, 0)};
        if (tex == nullptr) { return; }

        auto const dp {player.Position - cell};
        auto const [vx, vy] {col.RayDir};

        // Plane equation coefficients (A, B, C) for non-flat slopes.
        // Substituting ray equation P = pos + t * rayDir into the bilinear slope equation
        // yields a quadratic equation in t: A*t^2 + term*t + (C - eyeZ) = 0.
        f64 const D {slope.BR - slope.BL - slope.TR + slope.TL};
        f64 const A {D * vx * vy};
        f64 const B {((slope.TR - slope.TL) * vx) + ((slope.BL - slope.TL) * vy) + (D * ((dp.X * vy) + (dp.Y * vx)))};
        f64 const C {slope.TL + ((slope.TR - slope.TL) * dp.X) + ((slope.BL - slope.TL) * dp.Y) + (D * dp.X * dp.Y)};

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

            auto const pos {col.point_at(t)};
            f64 const  z {HeightAt(slope, cell, pos)};

            i32 const texelX {WrapTexel(pos.X, WALL_SIZE.Width)};
            i32 const texelY {WrapTexel(pos.Y, WALL_SIZE.Height)};

            draw_surface_pixel(frame, col.X, y, tex, TexelOffset(texelX, texelY, WALL_SIZE.Width), pos, z, 1.0);
            _objectDepthBuffer[pixel_index(y)] = t;

            u32& pixel {screenBuf[pixel_index(y)]};

            // distance to the nearest wall: along the ray for the wall that was hit (doors, diagonals), and to the solid
            // neighbour cells, so the shadow keeps its width at an angle and follows corners
            if (_quality.ContactShadows && (walls != 0 || col.Hit.Distance - t < CONTACT_SHADOW_WIDTH)) {
                f64 const fx {pos.X - cell.X};
                f64 const fy {pos.Y - cell.Y};

                f64 dist {std::max(col.Hit.Distance - t, 0.0)};
                if ((walls & WALL_W) != 0) { dist = std::min(dist, fx); }
                if ((walls & WALL_E) != 0) { dist = std::min(dist, 1.0 - fx); }
                if ((walls & WALL_N) != 0) { dist = std::min(dist, fy); }
                if ((walls & WALL_S) != 0) { dist = std::min(dist, 1.0 - fy); }

                // a wall that only touches the cell at a corner
                auto const corner {[&](u32 bit, u32 sideA, u32 sideB, f64 cx, f64 cy) {
                    if ((walls & bit) != 0 && (walls & (sideA | sideB)) == 0) { dist = std::min(dist, std::hypot(fx - cx, fy - cy)); }
                }};
                corner(WALL_NW, WALL_N, WALL_W, 0.0, 0.0);
                corner(WALL_NE, WALL_N, WALL_E, 1.0, 0.0);
                corner(WALL_SW, WALL_S, WALL_W, 0.0, 1.0);
                corner(WALL_SE, WALL_S, WALL_E, 1.0, 1.0);

                if (dist < CONTACT_SHADOW_WIDTH) {
                    f64 const c {1.0 - (dist / CONTACT_SHADOW_WIDTH)};
                    pixel = scale_pixel(pixel, 1.0 - (CONTACT_SHADOW_STRENGTH * c * c));
                }
            }

            if (isFloor && surface.FloorReflectivity > 0.0) {
                f64 const mirrored {z - col.WallBase + ((player.EyeZ - z) * ((col.Hit.Distance / std::max(t, MIN_RAY_DIST)) - 1.0))};
                f64 const height {std::max(mirrored, 0.0)}; // height above the wall base that the bounced ray reaches
                u8 const* wallTexPtr {_cache.texture(col.Hit.Texture, 0)};
                if (height < std::min(REFLECTION_HEIGHT, col.WallTop - col.WallBase) && wallTexPtr != nullptr) {
                    i32 const wallTexX {WallTexelX(col.Hit)};
                    i32 const wallTexY {std::clamp(static_cast<i32>((1.0 - (height - std::floor(height))) * static_cast<f64>(WALL_SIZE.Height)), 0, WALL_SIZE.Height - 1)};
                    i32 const wallSrc {TexelOffset(wallTexX, wallTexY, WALL_SIZE.Width)};

                    auto const wallTint {light_at(col.point_at(col.Hit.Distance), col.WallBase + height)};
                    auto const ditheredTint {_quality.Dithering ? DitherTint(wallTint, col.X, y) : wallTint};
                    u32 const  reflected {ApplyTint(wallTexPtr[wallSrc + 0], wallTexPtr[wallSrc + 1], wallTexPtr[wallSrc + 2], ditheredTint)};

                    pixel = BlendPixel(pixel, reflected, surface.FloorReflectivity * (1.0 - (height / REFLECTION_HEIGHT)));
                }
            }
        }

        claim(isFloor, from, to);
    }};

    usize const count {col.Crossed.size()};

    for (usize i {0}; i < count; ++i) {
        f64 const  t0 {col.Crossed[i].Dist};
        bool const isLast {i + 1 == count};

        if (isLast && col.Hit.Distance - t0 <= WALL_RECESS_EPS) { break; } // wall on the cell face: it covers the rest of the column

        f64 const t1 {isLast ? col.Hit.Distance : col.Crossed[i + 1].Dist};
        if (t1 <= t0) { continue; }

        auto const cell {col.Crossed[i].Cell};
        auto const surface {map.surface_at(cell)};
        auto const slopes {map.slopes_at(cell)};

        i32 const bottomBefore {col.Bottom};
        i32 const topBefore {col.Top};

        if (i > 0) {
            auto const prevCell {col.Crossed[i - 1].Cell};
            auto const prevSlopes {map.slopes_at(prevCell)};
            draw_riser(true, prevCell, cell, prevSlopes, slopes, surface, t0);
            draw_riser(false, prevCell, cell, prevSlopes, slopes, surface, t0);
        }

        // which of the eight neighbours are solid walls: what can shadow this cell
        u32 walls {0};
        if (_quality.ContactShadows) {
            auto const is_wall {[&](i32 ox, i32 oy) {
                point_i const near {cell.X + ox, cell.Y + oy};
                return map_grid::Size.contains(near) && std::holds_alternative<normal_wall>(map.get_cell(near));
            }};
            if (is_wall(0, -1)) { walls |= WALL_N; }
            if (is_wall(0, 1)) { walls |= WALL_S; }
            if (is_wall(-1, 0)) { walls |= WALL_W; }
            if (is_wall(1, 0)) { walls |= WALL_E; }
            if (is_wall(-1, -1)) { walls |= WALL_NW; }
            if (is_wall(1, -1)) { walls |= WALL_NE; }
            if (is_wall(-1, 1)) { walls |= WALL_SW; }
            if (is_wall(1, 1)) { walls |= WALL_SE; }
        }

        draw_flat(true, surface, cell, slopes, t0, t1, walls);
        draw_flat(false, surface, cell, slopes, t0, t1, walls);

        if (col.Bottom < bottomBefore || col.Top > topBefore) {
            point_d const center {cell.X + 0.5, cell.Y + 0.5};
            f64 const     midZ {(HeightAt(slopes.Floor, cell, center) + HeightAt(slopes.Ceiling, cell, center)) * 0.5};
            if (IsVisiblyLit(light_at(center, midZ))) { map.mark_visible(cell); }
        }
    }

    if (col.Top < col.Bottom && IsVisiblyLit(light_at(col.lit_point_at(col.Hit.Distance), (col.WallBase + col.WallTop) * 0.5))) {
        map.mark_visible(col.Cell);
    }
}

void raycaster::draw_wall_column(frame const& frame, column_hit const& col)
{
    constexpr static f64 CORNER_WIDTH {0.25}; // world units from an inside corner
    constexpr static f64 CORNER_STRENGTH {0.6};

    auto const& hit {col.Hit};
    auto const* tex {_cache.texture(hit.Texture, 0)};
    if (tex == nullptr) { return; }

    auto&       map {frame.Level.map()};
    auto const& player {frame.Player};

    if (col.Top >= col.Bottom) { return; }

    f64 const dist {std::max(hit.Distance, MIN_RAY_DIST)};
    f64 const rowsPerUnit {_projPlaneDist / dist};
    f64 const wallBottomRow {std::round(col.CenterY + ((player.EyeZ - col.WallBase) * rowsPerUnit))};

    i32 const texX {WallTexelX(hit)};
    f64 const texStep {static_cast<f64>(WALL_SIZE.Height) / rowsPerUnit};
    f64       texPos {static_cast<f64>(WALL_SIZE.Height) + ((static_cast<f64>(col.Top) - wallBottomRow) * texStep)};

    auto const surfacePos {col.lit_point_at(hit.Distance)};

    // darken near inside corners of normal walls: where the neighbouring cell along the wall is solid
    f64 cornerShade {1.0};
    if (_quality.ContactShadows && col.ToOpen != point_i::Zero && std::holds_alternative<normal_wall>(map.get_cell(col.Cell))) {
        bool const xFace {col.ToOpen.X != 0};
        auto const hitPoint {col.point_at(hit.Distance)};
        f64 const  frac {std::clamp((xFace ? hitPoint.Y - col.Cell.Y : hitPoint.X - col.Cell.X), 0.0, 1.0)};
        auto const open {col.Cell + col.ToOpen};
        auto const lowStep {xFace ? point_i {0, -1} : point_i {-1, 0}};

        for (i32 sign : {1, -1}) {
            f64 const edgeDist {sign == 1 ? frac : 1.0 - frac};
            if (edgeDist >= CORNER_WIDTH) { continue; }

            auto const neighbor {open + (lowStep * sign)};
            if (!map_grid::Size.contains(neighbor) || !std::holds_alternative<normal_wall>(map.get_cell(neighbor))) { continue; }

            f64 const t {1.0 - (edgeDist / CORNER_WIDTH)};
            cornerShade *= 1.0 - (CORNER_STRENGTH * t * t);
        }
    }

    for (i32 y {col.Top}; y < col.Bottom; y++) {
        i32 const texY {((static_cast<i32>(std::floor(texPos)) % WALL_SIZE.Height) + WALL_SIZE.Height) % WALL_SIZE.Height};
        texPos += texStep;

        f64 const worldZ {std::max(col.WallBase, std::min(col.WallBase + ((wallBottomRow - y) / rowsPerUnit), col.WallTop))};
        draw_surface_pixel(frame, col.X, y, tex, TexelOffset(texX, texY, WALL_SIZE.Width), surfacePos, worldZ, cornerShade);
        _objectDepthBuffer[PixelIndex(_screenSize.Width, col.X, y)] = hit.Distance;
    }
}

void raycaster::draw_sprites(frame const& frame)
{
    auto&       level {frame.Level};
    auto const& player {frame.Player};

    rect_i const screenRect {point_i::Zero, _screenSize};

    u32* screenBuf {_screen.data()};
    for (auto& o : level.objects()) {
        auto&       go {base_of(o)};
        auto const* spr {go.sprite()};
        if (!spr) { continue; }

        go.set_seen(false);
        auto const goXY {go.position().xy()};

        auto const p {frame.Camera.project(goXY)};
        if (p.Depth <= 0) { continue; }
        f64 const depth {p.Depth};

        i32 const    spriteScreenX {static_cast<i32>(p.ScreenX)};
        auto const   spriteSize {spr->screen_size(p.Scale)};
        rect_i const spriteRect {point_i {spriteScreenX - (spriteSize.Width / 2),
                                          spr->screen_bottom(p.Scale, frame.CenterY, player.EyeZ - go.world_z()) - spriteSize.Height},
                                 spriteSize};
        if (!screenRect.intersects(spriteRect, true)) { continue; }

        point_i const drawStart {std::max(spriteRect.left(), 0), std::max(spriteRect.top(), 0)};
        point_i const drawEnd {std::min(spriteRect.right(), _screenSize.Width), std::min(spriteRect.bottom(), _screenSize.Height)};

        i32 const   facing {spr->facing_index(goXY, player.Position)};
        auto const* tex {_cache.texture(spr->Texture, facing)};
        auto const  texSize {_cache.texture_size(spr->Texture, facing)};

        f64 const texStepY {static_cast<f64>(texSize.Height) / spriteSize.Height};
        f64 const texPosYStart {(drawStart.Y - spriteRect.top()) * texStepY};

        auto const spriteCell {WorldToCell(goXY)};
        auto const spriteTint {accumulate_light(frame, goXY, go.world_z(), spriteCell, spr->LightMask)};
        bool const inLight {IsVisiblyLit(spriteTint)};

        for (i32 x {drawStart.X}; x < drawEnd.X; ++x) {
            if (depth >= _wallDepthBuffer[x]) { continue; }

            i32 const texX {std::clamp(((x - spriteRect.left()) * texSize.Width) / spriteSize.Width, 0, texSize.Width - 1)};
            f64       texPos {texPosYStart};

            for (i32 y {drawStart.Y}; y < drawEnd.Y; ++y) {
                i32 const texY {std::clamp(static_cast<i32>(texPos), 0, texSize.Height - 1)};
                texPos += texStepY;

                usize const idx {PixelIndex(_screenSize.Width, x, y)};
                if (depth >= _objectDepthBuffer[idx]) { continue; }

                i32 const texOffset {TexelOffset(texX, texY, texSize.Width)};
                if (IsMagenta(tex, texOffset)) { continue; }

                _objectDepthBuffer[idx] = depth;
                if (inLight) { go.set_seen(true); }
                CopyPixel(screenBuf, idx, tex, texOffset, spriteTint);
            }
        }
    }
}

void raycaster::draw_voxel_objects(frame const& frame)
{
    level&        level {frame.Level};
    player const& player {frame.Player};
    auto const&   camera {frame.Camera};

    // screen x, screen y and depth of a world point; depth is 0 for anything not safely in front of the camera
    auto const project {[&](point3_d const& world) -> point3_d {
        auto const p {camera.project(world.xy())};
        if (p.Depth <= 1e-6) { return {}; }

        return {.X = p.ScreenX, .Y = camera.screen_y(world.Z, p), .Z = p.Depth};
    }};

    rect_i const screenRect {point_i::Zero, _screenSize};
    u32*         screenBuf {_screen.data()};

    // where an object lands on screen, nothing if it is off screen or hidden behind walls
    auto const measure {[&](object_base const& go) -> std::optional<screen_bounds> {
        auto const* vo {go.voxel()};
        if (!vo->Grid) { return std::nullopt; }

        auto const xf {vo->make_transform(go.world_position())};

        f64  minX {INFINITE_DIST}, maxX {-INFINITE_DIST}, minY {INFINITE_DIST}, maxY {-INFINITE_DIST}, minDepth {INFINITE_DIST};
        bool behindCamera {false};

        for (auto const& corner : vo->Grid->corners()) {
            auto const s {project(xf.to_world(corner))};
            if (s.Z <= 0.0) {
                behindCamera = true;
                continue;
            }

            minX     = std::min(minX, s.X);
            maxX     = std::max(maxX, s.X);
            minY     = std::min(minY, s.Y);
            maxY     = std::max(maxY, s.Y);
            minDepth = std::min(minDepth, s.Z);
        }
        if (minDepth == INFINITE_DIST) { return std::nullopt; } // no corner in front of the camera

        rect_i const bbox {{static_cast<i32>(minX), static_cast<i32>(minY)},
                           {static_cast<i32>(maxX - minX) + 1, static_cast<i32>(maxY - minY) + 1}};
        if (!screenRect.intersects(bbox, true)) { return std::nullopt; }

        auto const clampX {[&](i32 v) { return std::clamp(v, 0, _screenSize.Width); }};
        auto const clampY {[&](i32 v) { return std::clamp(v, 0, _screenSize.Height); }};

        screen_bounds b {.Left    = clampX(bbox.Position.X),
                         .Right   = clampX(bbox.Position.X + bbox.Size.Width),
                         .Top     = clampY(bbox.Position.Y),
                         .Bottom  = clampY(bbox.Position.Y + bbox.Size.Height),
                         .Nearest = behindCamera ? 0.0 : minDepth};

        for (i32 x {b.Left}; x < b.Right; ++x) { b.MaxWallDist = std::max(b.MaxWallDist, _wallDepthBuffer[x]); }

        return b.Nearest < b.MaxWallDist ? std::optional {b} : std::nullopt;
    }};

    // nearest first, so pixels that are already covered reject the objects behind them
    _voxelOrder.clear();
    for (auto& o : level.objects()) {
        auto&       go {base_of(o)};
        auto* const vo {go.voxel()};
        if (!vo) { continue; }

        auto const goXY {go.position().xy()};
        go.set_seen(false);

        point_d const toObject {goXY - player.Position};
        _voxelOrder.emplace_back(toObject.length_squared(), &go);
    }
    std::ranges::sort(_voxelOrder, [](auto const& a, auto const& b) { return a.first < b.first; });

    std::vector<usize> measured;
    for (usize oi {0}; oi < _voxelOrder.size(); ++oi) {
        auto* const vo {_voxelOrder[oi].second->voxel()};
        vo->ScreenBounds = measure(*_voxelOrder[oi].second);
        if (vo->ScreenBounds) { measured.push_back(oi); }
    }

    // The pixel budget is shared out fairly: an object that fits within an equal share keeps full
    // resolution and hands what it does not use to the rest, so only the big, close objects get coarser.
    auto const area_of {[&](usize oi) { return static_cast<f64>(_voxelOrder[oi].second->voxel()->ScreenBounds->area()); }};
    std::ranges::sort(measured, [&](usize a, usize b) { return area_of(a) < area_of(b); }); // smallest first

    std::vector<i32> strides(_voxelOrder.size(), 1);
    f64              remaining {static_cast<f64>(_quality.VoxelPixelBudget)};
    for (usize i {0}; i < measured.size(); ++i) {
        f64 const area {area_of(measured[i])};
        f64 const share {std::max(remaining / static_cast<f64>(measured.size() - i), 1.0)};
        i32 const s {area > share ? static_cast<i32>(std::ceil(std::sqrt(area / share))) : 1};

        strides[measured[i]] = s;
        remaining            = std::max(remaining - (area / (static_cast<f64>(s) * static_cast<f64>(s))), 0.0);
    }

    for (usize oi {0}; oi < _voxelOrder.size(); ++oi) {
        auto&       go {*_voxelOrder[oi].second};
        auto const* vo {go.voxel()};

        auto const& bounds {vo->ScreenBounds};
        if (!bounds) { continue; }

        auto const [left, right, top, bottom, nearest, maxWallDist] {*bounds};
        auto const xf {vo->make_transform(go.world_position())};

        auto const localOrigin {xf.to_local(point3_d::Lift(player.Position, player.EyeZ))};

        auto const edge_len_sq {[&](i32 col) { return ComputeCameraRayDir(col, _screenSize.Width, player).length_squared(); }};
        f64 const  maxLenSq {std::max(edge_len_sq(bounds->Left), edge_len_sq(right - 1))};
        f64 const  maxSlope {std::max(std::abs(frame.CenterY - top), std::abs(bottom - frame.CenterY)) / _projPlaneDist};
        f64 const  maxT {(maxWallDist * std::sqrt(maxLenSq + (maxSlope * maxSlope))) + vo->world_diagonal()};

        i32 const stride {strides[oi]};
        i32 const strideCols {((right - bounds->Left) + stride - 1) / stride};

        std::atomic_bool drawn {false}; // set when a visibly lit pixel of the object was written

        _taskManager.run_parallel([&](par_task const& ctx) {
            bool wrote {false};

            for (isize scol {ctx.Start}; scol < ctx.End; ++scol) {
                i32 const x {static_cast<i32>(PixelIndex(stride, left, scol))};
                if (x >= right) { continue; }

                i32 const xBlockEnd {std::min(x + stride, right)};

                // the whole object is behind the walls in these columns
                f64 blockWall {0.0};
                for (i32 bx {x}; bx < xBlockEnd; ++bx) { blockWall = std::max(blockWall, _wallDepthBuffer[bx]); }
                if (nearest >= blockWall) { continue; }

                auto const rayDir2D {ComputeCameraRayDir(x + ((stride - 1) / 2), _screenSize.Width, player)};

                for (i32 y {top}; y < bottom; y += stride) {
                    // already covered by something nearer than the whole object
                    if (stride == 1 && nearest >= _objectDepthBuffer[PixelIndex(_screenSize.Width, x, y)]) { continue; }

                    f64 const  rayDirZ {(frame.CenterY - (y + (0.5 * stride))) / _projPlaneDist};
                    auto const rayDir3D {point3_d::Lift(rayDir2D, rayDirZ).normalized()};

                    auto const localDir {xf.dir_to_local(rayDir3D)};

                    auto const hit {vo->Grid->raycast(localOrigin, localDir, maxT)};
                    if (!hit.Hit) { continue; }

                    auto const localHitPos {localOrigin + (localDir * hit.T)};
                    auto const worldHit {xf.to_world(localHitPos)};
                    auto const worldHitXY {worldHit.xy()};
                    auto const depth {project(worldHit).Z};

                    if (depth <= 0.0) { continue; }
                    if (depth >= _wallDepthBuffer[x]) { continue; }

                    auto const hitCellForLight {WorldToCell(worldHitXY)};
                    f64 const  aoFactor {_quality.VoxelAo ? vo->Grid->face_ao(hit, localHitPos) : 1.0};

                    auto const tint {accumulate_light(frame, worldHitXY, worldHit.Z, hitCellForLight, vo->LightMask) * aoFactor};
                    bool const lit {IsVisiblyLit(tint)};

                    i32 const yBlockEnd {std::min(y + stride, bottom)};
                    for (i32 by {y}; by < yBlockEnd; ++by) {
                        for (i32 bx {x}; bx < xBlockEnd; ++bx) {
                            if (depth >= _wallDepthBuffer[bx]) { continue; }

                            usize const depthIndex {PixelIndex(_screenSize.Width, bx, by)};
                            if (depth >= _objectDepthBuffer[depthIndex]) { continue; }

                            _objectDepthBuffer[depthIndex] = depth;
                            screenBuf[depthIndex]          = ApplyTint(hit.Color.R, hit.Color.G, hit.Color.B, tint);
                            if (lit) { wrote = true; }
                        }
                    }
                }
            }

            if (wrote) { drawn.store(true, std::memory_order_relaxed); }
        },
                                  strideCols);

        go.set_seen(drawn.load(std::memory_order_relaxed));
    }
}

void raycaster::draw_motes(frame const& frame)
{
    constexpr static f64 MIN_LIGHT {0.04}; // motes in less light than this are skipped

    auto const& level {frame.Level};
    auto const& map {level.map()};
    auto const& player {frame.Player};
    auto const& camera {frame.Camera};

    static auto add_pixel {[](u32 px, u32 add, f64 t) -> u32 {
        u32 const w {static_cast<u32>(std::clamp(t, 0.0, 1.0) * 256.0)};
        u32 const r {std::min((px & 0xFFu) + (((add & 0xFFu) * w) >> 8), 255u)};
        u32 const g {std::min(((px >> 8) & 0xFFu) + ((((add >> 8) & 0xFFu) * w) >> 8), 255u)};
        u32 const b {std::min(((px >> 16) & 0xFFu) + ((((add >> 16) & 0xFFu) * w) >> 8), 255u)};
        return 0xFF000000u | (b << 16) | (g << 8) | r;
    }};

    static auto frac {[](f64 v) -> f64 { return v - std::floor(v); }};

    f64 const t {static_cast<f64>(level.time().count()) / 1000.0};
    f64 const lightRange {player.Settings.LightRange};

    i32 const     reach {static_cast<i32>(std::ceil(lightRange))};
    point_i const center {WorldToCell(player.Position)};
    u32*          screenBuf {_screen.data()};

    for (i32 cy {center.Y - reach}; cy <= center.Y + reach; ++cy) {
        for (i32 cx {center.X - reach}; cx <= center.X + reach; ++cx) {
            point_i const cell {cx, cy};
            if (!map_grid::Size.contains(cell) || std::holds_alternative<normal_wall>(map.get_cell(cell)) || !IsWithinPlayerLight(player, cell)) { continue; }

            auto const& motes {level.motes_at(cell)};
            if (motes.Count == 0) { continue; }

            // skip cells that are entirely behind the player or outside the view cone
            {
                auto const c {camera.project(point_d {cx + 0.5, cy + 0.5})};
                f64 const  slack {(0.7072 + motes.Drift + motes.Size) * camera.max_shift_per_unit()}; // cell half-diagonal, drift and mote size
                if (c.Depth < -slack || std::abs(c.Lateral) > c.Depth + (2.0 * slack)) { continue; }
            }

            auto const slopes {map.slopes_at(cell)};

            for (u32 i {0}; i < motes.Count; ++i) {
                point_d const pos {cx + frac(HashUnit(cx, cy, i, 3) + (motes.Wind.X * t)) + (motes.Drift * std::sin((t * motes.DriftSpeed) + (HashUnit(cx, cy, i, 5) * TAU))),
                                   cy + frac(HashUnit(cx, cy, i, 4) + (motes.Wind.Y * t)) + (motes.Drift * std::sin((t * motes.DriftSpeed * 0.8) + (HashUnit(cx, cy, i, 6) * TAU)))};

                // falls (or rises) and wraps inside the height band
                f64 const fall {HashUnit(cx, cy, i, 7) - (t * motes.FallSpeed * (1.0 + (motes.SpeedJitter * ((2.0 * HashUnit(cx, cy, i, 8)) - 1.0))))};
                f64 const floorZ {HeightAt(slopes.Floor, cell, pos)};
                f64 const ceilZ {HeightAt(slopes.Ceiling, cell, pos)};
                f64 const z {std::min(floorZ + motes.MinHeight + (frac(fall) * (motes.MaxHeight - motes.MinHeight)), ceilZ)};

                auto const p {camera.project(pos)};
                f64 const  depth {p.Depth};
                if (depth <= 0.05) { continue; }

                i32 const sx {static_cast<i32>(std::floor(p.ScreenX))};
                i32 const sy {static_cast<i32>(std::floor(camera.screen_y(z, p)))};

                i32 const widthPx {std::max(1, static_cast<i32>(std::round(motes.Size * p.Scale)))};
                i32 const lengthPx {std::max(widthPx, static_cast<i32>(std::round(motes.Length * p.Scale)))};
                i32 const left {sx - (widthPx / 2)};
                if (left >= _screenSize.Width || left + widthPx <= 0 || sy - lengthPx >= _screenSize.Height || sy < 0) { continue; }

                auto tint {accumulate_light(frame, pos, z, cell, LIGHT_LAYERS_ALL)};
                tint = tint.lerp(color_d {.R = 1.0, .G = 1.0, .B = 1.0}, motes.Emissive);
                if (!IsVisiblyLit(tint, MIN_LIGHT)) { continue; }

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
                u32 const moteColor {ApplyTint(255, 255, 255, base * tint)};

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

auto raycaster::sky_pixel(frame const& frame, i32 skyY, point_d const& rayDir) const -> u32
{
    static constexpr point3_d WorldUp {.Z = 1.0};

    static auto smooth {[](f64 e0, f64 e1, f64 x) -> f64 {
        f64 const u {std::clamp((x - e0) / (e1 - e0), 0.0, 1.0)};
        return u * u * (3.0 - (2.0 * u));
    }};

    // two octaves of noise: the shape octave and a detail octave at twice the frequency
    auto const noise {[this](f64 x, f64 y) -> f64 { return _noise({static_cast<f32>(x), static_cast<f32>(y)}); }};
    auto const detail {[&](f64 x, f64 y) -> f64 { return noise((x * 2.0) + 7.31, (y * 2.0) + 3.17); }};

    auto const& sky {frame.Level.sky().frame()};

    // view direction; the horizon is the unbobbed screen center
    f64 const  zRow {((_screenSize.Height / 2.0) - skyY) / _projPlaneDist};
    auto const dir {point3_d::Lift(rayDir, zRow).normalized()};

    // gradient
    f64 const h {std::clamp(dir.Z, 0.0, 1.0)};
    auto      color {sky.Horizon.lerp(sky.Zenith, h)};

    // dusk glow, sun halo and sun disc
    f64 const sunDot {dir.dot(sky.Sun)};
    f64 const s {std::max(sunDot, 0.0)};
    f64 const s2 {s * s};
    f64 const s8 {s2 * s2 * s2 * s2};
    f64 const s32 {s8 * s8 * s8 * s8};

    f64 const duskGlow {sky.Dusk * s8 * (1.0 - (0.7 * h))};
    f64 const haloGlow {0.35 * s32 * (0.3 + (0.7 * sky.Day))};
    color += (sky.DuskColor * duskGlow) + (sky.SunColor * haloGlow);
    if (sunDot >= sky.CosSun) { color = sky.SunColor; }

    // stars: cells of StarSize screen pixels in (azimuth, sin elevation)
    if (sky.Stars) {
        f64 const azimuth {std::atan2(rayDir.Y, rayDir.X)};
        f64 const cellsPerUnit {_projPlaneDist / std::max(sky.StarSize, 1.0)};
        f64 const size {std::min(sky.StarSize, 1.0)};
        i32 const gx {static_cast<i32>(std::floor((azimuth + std::numbers::pi) * cellsPerUnit))};
        i32 const gy {static_cast<i32>(std::floor(dir.Z * cellsPerUnit))};
        if (HashUnit(gx, gy, 0, 21) < sky.StarDensity) {
            f64 const speed {1.0 + (2.0 * HashUnit(gx, gy, 0, 22))};
            f64 const phase {HashUnit(gx, gy, 0, 23) * TAU};
            f64 const pulse {0.5 + (0.5 * std::sin((sky.Time * speed) + phase))};
            f64 const base {0.4 + (0.6 * HashUnit(gx, gy, 0, 24))};
            color += size * size * sky.Night * (1.0 - (sky.StarTwinkle * pulse)) * base;
        }
    }

    // moons and planets
    for (auto const& body : sky.Bodies) {
        f64 const d {dir.dot(body.Dir)};
        if (d < body.CosR) { continue; }

        f64 const rho2 {std::max(1.0 - (d * d), 0.0) / (body.SinR * body.SinR)};
        f64 const nz {std::sqrt(std::max(1.0 - rho2, 0.0))};

        color_d surface {body.Color};
        if (body.Noise > 0.0) {
            // disc coordinates, u to the right and v up, from the body direction and world up
            auto const right {body.Dir.cross(WorldUp).normalized()};
            auto const up {right.cross(body.Dir)};
            f64 const  u {dir.dot(right) / body.SinR};
            f64 const  v {dir.dot(up) / body.SinR};

            // features compress toward the limb, which makes the disc look round
            f64 const warp {body.NoiseScale / (0.4 + (0.6 * nz))};
            f64 const x {(u * warp) + body.NoiseSeed};
            f64 const y {v * warp};
            f64 const n {((0.5 * noise(x, y)) + (0.25 * detail(x, y))) / 0.75};
            surface = surface.lerp(body.NoiseColor, n * body.Noise);
        }

        if (body.Lit) {
            // phase: surface normal against the direction of the sun
            f64 const nDotL {((sunDot - (d * body.SunDot)) / body.SinR) - (body.SunDot * nz)};
            surface *= std::lerp(body.Ambient, 1.0, std::clamp(nDotL, 0.0, 1.0));
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

        f64 const k {sky.CloudScale / dir.Z};
        f64 const cx {(dir.X * k) + (sky.CloudWind.X * sky.Time)};
        f64 const cy {(dir.Y * k) + (sky.CloudWind.Y * sky.Time)};

        // the first octave carries the shape; if the detail octave at its maximum still stays under the edge, it is clear sky
        f64 const n0 {noise(cx, cy)};
        if (((0.5 * n0) + 0.25) / 0.75 > lo) {
            f64 const density {((0.5 * n0) + (0.25 * detail(cx, cy))) / 0.75};
            f64 const alpha {smooth(lo, hi, density) * smooth(0.03, 0.3, dir.Z)}; // fades out toward the horizon
            if (alpha > 0.0) {
                // the side facing the sun is thinner, so a cloud is lit where it is thicker than its sun-side neighbor
                f64 const toward {noise(cx + (sky.Sun.X * 0.3), cy + (sky.Sun.Y * 0.3))};
                f64 const lit {std::clamp(0.5 + ((n0 - toward) * 3.0), 0.0, 1.0)};

                f64 const  brightness {(0.12 + (0.88 * sky.Day)) * (0.55 + (0.45 * lit))};
                auto const cloud {sky.CloudColor.lerp(sky.DuskColor, sky.Dusk * s2) * brightness};
                color = color.lerp(cloud, alpha);
            }
        }
    }

    return ApplyTint(255, 255, 255, color);
}
