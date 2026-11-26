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
#include "Level.hpp"
#include "LevelObject.hpp"
#include "Walls.hpp"

lighting::lighting()
    : _taskManager {locate_service<task_manager>()}
{
}

// The light of emissive ground: every cell whose ground has an Emissive and a GlowRange spreads its Light color over the
// open cells around it, with a smooth falloff and only where it can see them. The cells hold the sum; glow_at blends them.
void lighting::rebuild_glow(level const& lvl)
{
    _glow.assign(static_cast<usize>(MAP_WIDTH) * MAP_HEIGHT, color_d {});

    auto const& map {lvl.map()};
    auto const  open {[&](point_i cell) { return map_grid::Size.contains(cell) && !map.is_blocking_light(cell); }};

    // line of sight between the centers of two cells
    auto const sees {[&](point_i a, point_i b) -> bool {
        point_d const from {a.X + 0.5, a.Y + 0.5};
        point_d const diff {point_d {b.X + 0.5, b.Y + 0.5} - from};
        f64 const     dist {diff.length()};

        cell_walk walk {from, {diff.X / dist, diff.Y / dist}};
        for (;;) {
            if (walk.exit() >= dist) { return true; }

            walk.advance();
            if (!map_grid::Size.contains(walk.cell()) || walk.cell() == b) { return true; }
            if (map.is_blocking_light(walk.cell())) { return false; }
        }
    }};

    for (i32 cy {0}; cy < MAP_HEIGHT; ++cy) {
        for (i32 cx {0}; cx < MAP_WIDTH; ++cx) {
            point_i const source {cx, cy};
            if (!open(source)) { continue; }

            auto const& ground {lvl.ground_at(source)};
            if (ground.Emissive <= 0.0 || ground.GlowRange <= 0.0) { continue; }

            color_d const emitted {ground.Light * (ground.Emissive * ground.GlowIntensity)};
            i32 const     reach {static_cast<i32>(std::ceil(ground.GlowRange))};

            for (i32 dy {-reach}; dy <= reach; ++dy) {
                for (i32 dx {-reach}; dx <= reach; ++dx) {
                    point_i const cell {cx + dx, cy + dy};
                    if (!open(cell)) { continue; }

                    f64 const distSq {static_cast<f64>((dx * dx) + (dy * dy))};
                    f64 const rangeSq {ground.GlowRange * ground.GlowRange};
                    if (distSq >= rangeSq || (distSq > 0.0 && !sees(source, cell))) { continue; }

                    f64 const falloff {1.0 - (distSq / rangeSq)};
                    _glow[PixelIndex(MAP_WIDTH, cell.X, cell.Y)] += emitted * (falloff * falloff);
                }
            }
        }
    }

    // a wall takes the glow of the open cells next to it, so a wall face is lit like the floor in front of it
    auto const walls {_glow};
    for (i32 cy {0}; cy < MAP_HEIGHT; ++cy) {
        for (i32 cx {0}; cx < MAP_WIDTH; ++cx) {
            if (open(point_i {cx, cy})) { continue; }

            color_d sum {};
            i32     count {0};
            for (i32 dy {-1}; dy <= 1; ++dy) {
                for (i32 dx {-1}; dx <= 1; ++dx) {
                    point_i const next {cx + dx, cy + dy};
                    if (!open(next)) { continue; }

                    sum += walls[PixelIndex(MAP_WIDTH, next.X, next.Y)];
                    ++count;
                }
            }
            if (count > 0) { _glow[PixelIndex(MAP_WIDTH, cx, cy)] = sum * (1.0 / count); }
        }
    }
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

    auto const at {[&](i32 x, i32 y) -> color_d {
        return map_grid::Size.contains(point_i {x, y}) ? _glow[PixelIndex(MAP_WIDTH, x, y)] : color_d {};
    }};

    return (at(x0, y0) * ((1.0 - fx) * (1.0 - fy))) + (at(x0 + 1, y0) * (fx * (1.0 - fy))) + (at(x0, y0 + 1) * ((1.0 - fx) * fy)) + (at(x0 + 1, y0 + 1) * (fx * fy));
}

auto lighting::lights() const -> std::span<dynamic_light const> { return _lights; }

auto lighting::lights_at(point_i map) const -> std::span<u32 const>
{
    if (!map_grid::Size.contains(map) || _cellLights.empty()) { return {}; }
    return _cellLights[PixelIndex(MAP_WIDTH, map.X, map.Y)];
}

void lighting::update(level const& lvl, usize losSamples)
{
    if (_glow.empty()) { rebuild_glow(lvl); }

    constexpr static f64 CELL_DIAGONAL_HALF {std::numbers::sqrt2 / 2}; // max distance from a cell's center to its farthest corner

    bool rebuild {losSamples != _losSamples};

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
        auto const lightCell {WorldToCell(lightPos)};

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

            cell_walk walk {from, {diff.X / dist, diff.Y / dist}};

            bool visible {true};
            for (;;) {
                if (walk.exit() >= dist) { break; } // reached the light before hitting anything

                walk.advance();
                if (!map_grid::Size.contains(walk.cell()) || walk.cell() == lightCell) { break; }

                if (lvl.map().is_blocking_light(walk.cell())) {
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
