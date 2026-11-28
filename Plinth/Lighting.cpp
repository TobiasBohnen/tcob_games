// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Lighting.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>
#include <optional>
#include <span>
#include <vector>

#include "Common.hpp"
#include "Level.hpp"
#include "LevelObject.hpp"
#include "Map.hpp"
#include "Walls.hpp"

auto cell_light::sees(point_d pos) const -> bool
{
    if (Seen == 0xFFFF) { return true; }

    i32 const x {std::clamp(static_cast<i32>((pos.X - std::floor(pos.X)) * 4.0), 0, 3)};
    i32 const y {std::clamp(static_cast<i32>((pos.Y - std::floor(pos.Y)) * 4.0), 0, 3)};
    return ((Seen >> ((y * 4) + x)) & 1) != 0;
}

lighting::lighting()
    : _taskManager {locate_service<task_manager>()}
{
}

void lighting::update(level const& lvl, usize losSamples)
{
    if (_dirty) {
        rebuild_blocking(lvl);
        rebuild_glow(lvl);
    }

    constexpr static f64 CELL_DIAGONAL_HALF {std::numbers::sqrt2 / 2}; // max distance from a cell's center to its farthest corner

    bool rebuild {_dirty || losSamples != _losSamples};
    _dirty = false;

    // the lights of the objects, placed in the world; a change in position or range means the cell lists are stale
    usize index {0};
    for (auto const& go : lvl.objects()) {
        auto const light {world_light(go)};
        if (!light) { continue; }

        if (index >= _lights.size()) {
            _lights.push_back(*light);
            rebuild = true;
        } else {
            auto& cached {_lights[index]};
            if (light->Position != cached.Position || light->Range != cached.Range) { rebuild = true; }
            cached = *light;
        }
        ++index;
    }

    if (index != _lights.size()) {
        _lights.resize(index);
        rebuild = true;
    }

    _params.resize(_lights.size());
    for (usize i {0}; i < _lights.size(); ++i) {
        auto const& lt {_lights[i]};
        _params[i] = MakeLightParams(lt.Position.xy(), lt.Position.Z, lt.Range, lt.Intensity, lt.Color, lt.Layers);
    }

    if (!rebuild) { return; }

    _losSamples = losSamples;

    _cellLights.resize(static_cast<usize>(MAP_WIDTH) * MAP_HEIGHT);
    for (auto& list : _cellLights) { list.clear(); }
    if (_lights.empty()) { return; }

    // which parts of a cell see a light: all of it when its center does, which costs one walk. Otherwise the corners are tried,
    // and only if one of them sees the light, the parts are told apart, so the cell is lit up to the shadow edge and not beyond
    auto const seen_parts {[&](point_i cell, point_d const& lightPos) -> std::optional<u16> {
        std::array<point_d, 5> const samples {{
            {cell.X + 0.5, cell.Y + 0.5},
            {cell.X + 0.1, cell.Y + 0.1},
            {cell.X + 0.9, cell.Y + 0.9},
            {cell.X + 0.9, cell.Y + 0.1},
            {cell.X + 0.1, cell.Y + 0.9},
        }};

        auto const used {std::span {samples}.first(losSamples)};
        auto const sees {[&](point_d const& from) { return lvl.map().has_line_of_sight(from, lightPos); }};

        if (used.empty()) { return std::nullopt; }
        if (sees(used.front())) { return u16 {0xFFFF}; }
        if (!std::ranges::any_of(used.subspan(1), sees)) { return std::nullopt; }

        u16 mask {0};
        for (i32 y {0}; y < 4; ++y) {
            for (i32 x {0}; x < 4; ++x) {
                if (sees({cell.X + ((x + 0.5) / 4.0), cell.Y + ((y + 0.5) / 4.0)})) { mask |= static_cast<u16>(1 << ((y * 4) + x)); }
            }
        }
        return mask != 0 ? std::optional<u16> {mask} : std::nullopt;
    }};

    struct light_sweep {
        bool Active {false};
        i32  CxMin {0};
        i32  CxMax {-1};
        i32  CyMin {0};
        i32  CyMax {-1};
        f64  RadiusSq {0.0};
    };

    std::vector<light_sweep> sweeps(_lights.size());
    for (usize li {0}; li < _lights.size(); ++li) {
        auto const&   lt {_lights[li]};
        point_d const lightXY {lt.Position.xy()};
        if (!map_grid::Size.contains(WorldToCell(lightXY))) { continue; }

        f64 const sweepRadius {lt.Range + CELL_DIAGONAL_HALF};

        auto& sweep {sweeps[li]};
        sweep.Active   = true;
        sweep.RadiusSq = sweepRadius * sweepRadius;
        sweep.CxMin    = std::clamp(static_cast<i32>(std::floor(lightXY.X - sweepRadius)), 0, MAP_WIDTH - 1);
        sweep.CxMax    = std::clamp(static_cast<i32>(std::floor(lightXY.X + sweepRadius)), 0, MAP_WIDTH - 1);
        sweep.CyMin    = std::clamp(static_cast<i32>(std::floor(lightXY.Y - sweepRadius)), 0, MAP_HEIGHT - 1);
        sweep.CyMax    = std::clamp(static_cast<i32>(std::floor(lightXY.Y + sweepRadius)), 0, MAP_HEIGHT - 1);
    }

    _taskManager.run_parallel([&](par_task const& ctx) {
        for (i32 cy {static_cast<i32>(ctx.Start)}; cy < static_cast<i32>(ctx.End); ++cy) {
            for (usize li {0}; li < sweeps.size(); ++li) {
                auto const& sweep {sweeps[li]};
                if (!sweep.Active || cy < sweep.CyMin || cy > sweep.CyMax) { continue; }

                auto const&   lt {_lights[li]};
                point_d const lightXY {lt.Position.xy()};

                for (i32 cx {sweep.CxMin}; cx <= sweep.CxMax; ++cx) {
                    point_d const cellCenter {cx + 0.5, cy + 0.5};
                    point_d const toLightXY {lightXY - cellCenter};

                    auto const slp {lvl.map().slopes_at(point_i {cx, cy})};
                    f64 const  lowZ {std::min({slp.Floor.TL, slp.Floor.TR, slp.Floor.BL, slp.Floor.BR})};
                    f64 const  highZ {std::max({slp.Ceiling.TL, slp.Ceiling.TR, slp.Ceiling.BL, slp.Ceiling.BR})};
                    f64 const  dz {std::max({lowZ - lt.Position.Z, lt.Position.Z - highZ, 0.0})};

                    f64 const distSq {toLightXY.dot(toLightXY) + (dz * dz)};
                    if (distSq >= sweep.RadiusSq) { continue; }

                    if (auto const parts {seen_parts(point_i {cx, cy}, lightXY)}) {
                        _cellLights[PixelIndex(MAP_WIDTH, cx, cy)].push_back({.Light = static_cast<u32>(li), .Seen = *parts});
                    }
                }
            }
        }
    },
                              MAP_HEIGHT);
}

auto lighting::lights() const -> std::span<light_params const> { return _params; }

auto lighting::lights_at(point_i map) const -> std::span<cell_light const>
{
    if (!map_grid::Size.contains(map) || _cellLights.empty()) { return {}; }
    return _cellLights[PixelIndex(MAP_WIDTH, map.X, map.Y)];
}

static constexpr f64 INV_255 {1.0 / 255.0};
static constexpr f64 GLOW_INTENSITY {0.6}; // how strongly an emissive floor lights its surroundings, at the floor itself

auto lighting::glow_at(point_d pos) const -> color_d
{
    if (_glow.empty()) { return {}; }

    f64 const u {pos.X - 0.5};
    f64 const v {pos.Y - 0.5};
    i32 const x0 {static_cast<i32>(std::floor(u))};
    i32 const y0 {static_cast<i32>(std::floor(v))};
    f64 const fx {u - x0};
    f64 const fy {v - y0};

    // blends the four nearest cells, but only the open ones: a wall must neither take glow nor pass it on to the other side.
    // The cell diagonal to the one `pos` is in only counts if both cells next to it are open as well, otherwise its glow would
    // come through the corner where the walls meet
    auto const open {[&](i32 x, i32 y) { return map_grid::Size.contains(point_i {x, y}) && _blocked[PixelIndex(MAP_WIDTH, x, y)] == 0; }};
    i32 const  ownX {fx < 0.5 ? x0 : x0 + 1};
    i32 const  ownY {fy < 0.5 ? y0 : y0 + 1};

    color_d    sum {};
    f64        weights {0.0};
    auto const add {[&](i32 x, i32 y, f64 weight) {
        if (!open(x, y)) { return; }
        if (x != ownX && y != ownY && !(open(x, ownY) && open(ownX, y))) { return; }

        sum += _glow[PixelIndex(MAP_WIDTH, x, y)] * weight;
        weights += weight;
    }};
    add(x0, y0, (1.0 - fx) * (1.0 - fy));
    add(x0 + 1, y0, fx * (1.0 - fy));
    add(x0, y0 + 1, (1.0 - fx) * fy);
    add(x0 + 1, y0 + 1, fx * fy);

    return weights > 1e-6 ? sum * (1.0 / weights) : color_d {};
}

auto lighting::blocks_light(point_i cell) const -> bool
{
    return map_grid::Size.contains(cell) && !_blocked.empty() && _blocked[PixelIndex(MAP_WIDTH, cell.X, cell.Y)] != 0;
}

void lighting::invalidate() { _dirty = true; }

void lighting::rebuild_blocking(level const& lvl)
{
    auto const& map {lvl.map()};

    _blocked.assign(static_cast<usize>(MAP_WIDTH) * MAP_HEIGHT, 0);
    for (i32 cy {0}; cy < MAP_HEIGHT; ++cy) {
        for (i32 cx {0}; cx < MAP_WIDTH; ++cx) { _blocked[PixelIndex(MAP_WIDTH, cx, cy)] = map.is_blocking_light(point_i {cx, cy}) ? 1 : 0; }
    }
}

// The light of emissive floors: every open cell whose floor has a FloorEmissive and a FloorGlowRange spreads its FloorGlowColor over
// the open cells around it, with a smooth falloff and only where it can see them.
void lighting::rebuild_glow(level const& lvl)
{
    auto const& map {lvl.map()};

    _glow.assign(static_cast<usize>(MAP_WIDTH) * MAP_HEIGHT, color_d {});
    bool any {false};

    auto const spread {[&](point_i source, cell_surface const& floor) {
        any = true;
        color_d const emitted {color_d {.R = floor.FloorGlowColor.R * INV_255, .G = floor.FloorGlowColor.G * INV_255, .B = floor.FloorGlowColor.B * INV_255} * (floor.FloorEmissive * GLOW_INTENSITY)};
        f64 const     rangeSq {floor.FloorGlowRange * floor.FloorGlowRange};
        i32 const     reach {static_cast<i32>(std::ceil(floor.FloorGlowRange))};

        for (i32 dy {-reach}; dy <= reach; ++dy) {
            for (i32 dx {-reach}; dx <= reach; ++dx) {
                point_i const cell {source.X + dx, source.Y + dy};
                if (!map_grid::Size.contains(cell)) { continue; }

                auto const index {PixelIndex(MAP_WIDTH, cell.X, cell.Y)};
                if (_blocked[index] != 0) { continue; }

                f64 const distSq {static_cast<f64>((dx * dx) + (dy * dy))};
                if (distSq >= rangeSq) { continue; }
                if (distSq > 0.0 && !map.has_line_of_sight({source.X + 0.5, source.Y + 0.5}, {cell.X + 0.5, cell.Y + 0.5})) { continue; }

                f64 const falloff {1.0 - (distSq / rangeSq)};
                _glow[index] += emitted * (falloff * falloff);
            }
        }
    }};

    for (i32 cy {0}; cy < MAP_HEIGHT; ++cy) {
        for (i32 cx {0}; cx < MAP_WIDTH; ++cx) {
            point_i const cell {cx, cy};
            if (_blocked[PixelIndex(MAP_WIDTH, cx, cy)] != 0) { continue; }

            auto const floor {map.surface_at(cell)};
            if (floor.FloorEmissive > 0.0 && floor.FloorGlowRange > 0.0) { spread(cell, floor); }
        }
    }

    if (!any) { _glow.clear(); } // glow_at has nothing to do then
}
