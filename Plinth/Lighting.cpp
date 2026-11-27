// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Lighting.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>
#include <span>
#include <vector>

#include "CellWalk.hpp"
#include "Common.hpp"
#include "Ground.hpp"
#include "Level.hpp"
#include "LevelObject.hpp"
#include "Map.hpp"
#include "Walls.hpp"

// whether the way from one point to another meets no blocking cell; the cell of `to` itself is not tested
static auto HasLineOfSight(map const& map, point_d from, point_d to) -> bool
{
    point_d const diff {to - from};
    f64 const     dist {diff.length()};
    if (dist < 1e-6) { return true; }

    auto const toCell {WorldToCell(to)};

    cell_walk walk {from, {diff.X / dist, diff.Y / dist}};
    for (;;) {
        if (walk.exit() >= dist) { return true; } // reached the end before hitting anything

        walk.advance();
        if (!map_grid::Size.contains(walk.cell()) || walk.cell() == toCell) { return true; }
        if (map.is_blocking_light(walk.cell())) { return false; }
    }
}

lighting::lighting()
    : _taskManager {locate_service<task_manager>()}
{
}

void lighting::update(level const& lvl, usize losSamples)
{
    if (_dirty) { rebuild_glow(lvl); }

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

    if (!rebuild) { return; }

    _losSamples = losSamples;

    _cellLights.resize(static_cast<usize>(MAP_WIDTH) * MAP_HEIGHT);
    for (auto& list : _cellLights) { list.clear(); }
    if (_lights.empty()) { return; }

    auto const cell_has_any_line_of_sight {[&](point_i cell, point_d const& lightPos) -> bool {
        std::array<point_d, 5> const samples {{
            {cell.X + 0.5, cell.Y + 0.5},
            {cell.X + 0.1, cell.Y + 0.1},
            {cell.X + 0.9, cell.Y + 0.9},
            {cell.X + 0.9, cell.Y + 0.1},
            {cell.X + 0.1, cell.Y + 0.9},
        }};

        return std::ranges::any_of(std::span {samples}.first(losSamples), [&](point_d const& from) { return HasLineOfSight(lvl.map(), from, lightPos); });
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

                    if (cell_has_any_line_of_sight(point_i {cx, cy}, lightXY)) {
                        _cellLights[PixelIndex(MAP_WIDTH, cx, cy)].push_back(static_cast<u32>(li));
                    }
                }
            }
        }
    },
                              MAP_HEIGHT);
}

auto lighting::lights() const -> std::span<dynamic_light const> { return _lights; }

auto lighting::lights_at(point_i map) const -> std::span<u32 const>
{
    if (!map_grid::Size.contains(map) || _cellLights.empty()) { return {}; }
    return _cellLights[PixelIndex(MAP_WIDTH, map.X, map.Y)];
}

auto lighting::glow_at(point_d pos) const -> color_d
{
    if (_glow.empty()) { return {}; }

    f64 const u {pos.X - 0.5};
    f64 const v {pos.Y - 0.5};
    i32 const x0 {static_cast<i32>(std::floor(u))};
    i32 const y0 {static_cast<i32>(std::floor(v))};
    f64 const fx {u - x0};
    f64 const fy {v - y0};

    // blends the four nearest cells, but only the open ones: a wall must neither take glow nor pass it on to the other side
    color_d    sum {};
    f64        weights {0.0};
    auto const add {[&](i32 x, i32 y, f64 weight) {
        if (!map_grid::Size.contains(point_i {x, y})) { return; }

        auto const& cell {_glow[PixelIndex(MAP_WIDTH, x, y)]};
        if (!cell.Open) { return; }

        sum += cell.Color * weight;
        weights += weight;
    }};
    add(x0, y0, (1.0 - fx) * (1.0 - fy));
    add(x0 + 1, y0, fx * (1.0 - fy));
    add(x0, y0 + 1, (1.0 - fx) * fy);
    add(x0 + 1, y0 + 1, fx * fy);

    return weights > 1e-6 ? sum * (1.0 / weights) : color_d {};
}

void lighting::invalidate() { _dirty = true; }

// The light of emissive ground: every open cell whose ground has an Emissive and a GlowRange spreads its Light color over
// the open cells around it, with a smooth falloff and only where it can see them.
void lighting::rebuild_glow(level const& lvl)
{
    auto const& map {lvl.map()};

    _glow.assign(static_cast<usize>(MAP_WIDTH) * MAP_HEIGHT, glow_cell {});
    for (i32 cy {0}; cy < MAP_HEIGHT; ++cy) {
        for (i32 cx {0}; cx < MAP_WIDTH; ++cx) { _glow[PixelIndex(MAP_WIDTH, cx, cy)].Open = !map.is_blocking_light(point_i {cx, cy}); }
    }

    auto const spread {[&](point_i source, ground_settings const& ground) {
        color_d const emitted {ground.Light * (ground.Emissive * ground.GlowIntensity)};
        f64 const     rangeSq {ground.GlowRange * ground.GlowRange};
        i32 const     reach {static_cast<i32>(std::ceil(ground.GlowRange))};

        for (i32 dy {-reach}; dy <= reach; ++dy) {
            for (i32 dx {-reach}; dx <= reach; ++dx) {
                point_i const cell {source.X + dx, source.Y + dy};
                if (!map_grid::Size.contains(cell)) { continue; }

                auto& target {_glow[PixelIndex(MAP_WIDTH, cell.X, cell.Y)]};
                if (!target.Open) { continue; }

                f64 const distSq {static_cast<f64>((dx * dx) + (dy * dy))};
                if (distSq >= rangeSq) { continue; }
                if (distSq > 0.0 && !HasLineOfSight(map, {source.X + 0.5, source.Y + 0.5}, {cell.X + 0.5, cell.Y + 0.5})) { continue; }

                f64 const falloff {1.0 - (distSq / rangeSq)};
                target.Color += emitted * (falloff * falloff);
            }
        }
    }};

    for (i32 cy {0}; cy < MAP_HEIGHT; ++cy) {
        for (i32 cx {0}; cx < MAP_WIDTH; ++cx) {
            point_i const cell {cx, cy};
            if (!_glow[PixelIndex(MAP_WIDTH, cx, cy)].Open) { continue; }

            auto const& ground {lvl.ground_at(cell)};
            if (ground.Emissive > 0.0 && ground.GlowRange > 0.0) { spread(cell, ground); }
        }
    }
}
