// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Level.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <numbers>
#include <optional>
#include <span>
#include <type_traits>
#include <variant>
#include <vector>

#include "Common.hpp"
#include "Walls.hpp"

level::level()
    : _taskManager {locate_service<task_manager>()}
{
    _motes.resize(16);
}

void level::update(milliseconds deltaTime)
{
    _time += deltaTime;
    prepare_sky();

    f64 const dt {deltaTime.count() / 1000};
    for (auto& cell : _map) {
        std::visit(
            [dt](auto&& w) {
                if constexpr (requires { w.update(dt); }) {
                    w.update(dt);
                }
            },
            cell);
    }

    for (auto& obj : _objects) {
        obj.set_position(obj.position() + obj.direction() * dt);
    }

    std::erase_if(_objects, [](level_object const& go) { return go.dead(); });
}

auto level::get_cell(point_i idx) const -> cell const&
{
    return _map[idx];
}

void level::set_cell(point_i idx, cell const& cell)
{
    _map[idx] = cell;
}

void level::precompute_slopes()
{
    _cellSlopes.resize(static_cast<usize>(MAP_WIDTH) * MAP_HEIGHT);

    auto const vertex_height {[&](i32 vx, i32 vy, bool isFloor) -> f64 {
        f64  extremeZ {0.0};
        bool found {false};
        for (i32 cy : {vy - 1, vy}) {
            for (i32 cx : {vx - 1, vx}) {
                point_i const c {cx, cy};
                if (map_t::Size.contains(c)) {
                    f64 const z {isFloor ? surface_at(c).FloorZ : surface_at(c).CeilingZ};
                    if (!found) {
                        extremeZ = z;
                        found    = true;
                    } else {
                        if (std::abs(z) > std::abs(extremeZ)) {
                            extremeZ = z;
                        } else if (std::abs(z) == std::abs(extremeZ)) {
                            extremeZ = std::max(extremeZ, z);
                        }
                    }
                }
            }
        }
        return found ? extremeZ : 0.0;
    }};

    for (i32 y {0}; y < MAP_HEIGHT; ++y) {
        for (i32 x {0}; x < MAP_WIDTH; ++x) {
            usize const index {static_cast<usize>((y * MAP_WIDTH) + x)};
            _cellSlopes[index] = {.Ceiling = {.TL = vertex_height(x, y, false), .TR = vertex_height(x + 1, y, false), .BL = vertex_height(x, y + 1, false), .BR = vertex_height(x + 1, y + 1, false)},
                                  .Floor   = {.TL = vertex_height(x, y, true), .TR = vertex_height(x + 1, y, true), .BL = vertex_height(x, y + 1, true), .BR = vertex_height(x + 1, y + 1, true)}};
        }
    }
}

auto level::slopes_at(point_i idx) const -> slopes
{
    if (!map_t::Size.contains(idx)) { return {}; }
    return _cellSlopes[static_cast<usize>((idx.Y * MAP_WIDTH) + idx.X)];
}

auto level::lights() const -> std::span<dynamic_light const> { return _lights; }

auto level::lights_at(point_i idx) const -> std::span<u32 const>
{
    if (!map_t::Size.contains(idx) || _cellLights.empty()) { return {}; }
    return _cellLights[static_cast<usize>((idx.Y * MAP_WIDTH) + idx.X)];
}

void level::update_light_visibility(usize losSamples)
{
    constexpr static f64 INFINITE_DIST {1e30};                         // stand-in for "ray never crosses this axis"
    constexpr static f64 CELL_DIAGONAL_HALF {std::numbers::sqrt2 / 2}; // max distance from a cell's center to its farthest corner

    usize count {0};
    for (auto const& go : objects()) {
        if (go.light()) { ++count; }
    }

    bool rebuild {losSamples != _losSamples};
    if (count != _lights.size()) {
        _lights.resize(count);
        rebuild = true;
    }

    usize index {0};
    for (auto const& go : objects()) {
        auto const* source {go.light()};
        if (!source) { continue; }

        auto& cached {_lights[index++]};
        if (source->Position.X != cached.Position.X || source->Position.Y != cached.Position.Y || source->Z != cached.Z || source->Range != cached.Range) { rebuild = true; }
        cached = *source;
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

                if (is_blocking_light(map)) {
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
        auto const& lt {_lights[li]};
        if (!map_t::Size.contains(WorldToCell(lt.Position))) { continue; }

        f64 const sweepRadius {lt.Range + CELL_DIAGONAL_HALF};

        auto& sweep {sweeps[li]};
        sweep.Active   = true;
        sweep.RadiusSq = sweepRadius * sweepRadius;
        sweep.CxMin    = std::clamp(static_cast<i32>(std::floor(lt.Position.X - sweepRadius)), 0, MAP_WIDTH - 1);
        sweep.CxMax    = std::clamp(static_cast<i32>(std::floor(lt.Position.X + sweepRadius)), 0, MAP_WIDTH - 1);
        sweep.CyMin    = std::clamp(static_cast<i32>(std::floor(lt.Position.Y - sweepRadius)), 0, MAP_HEIGHT - 1);
        sweep.CyMax    = std::clamp(static_cast<i32>(std::floor(lt.Position.Y + sweepRadius)), 0, MAP_HEIGHT - 1);
    }

    _taskManager.run_parallel([&](par_task const& ctx) {
        for (i32 cy {static_cast<i32>(ctx.Start)}; cy < static_cast<i32>(ctx.End); ++cy) {
            for (usize li {0}; li < sweeps.size(); ++li) {
                auto const& sweep {sweeps[li]};
                if (!sweep.Active || cy < sweep.CyMin || cy > sweep.CyMax) { continue; }

                auto const& lt {_lights[li]};

                for (i32 cx {sweep.CxMin}; cx <= sweep.CxMax; ++cx) {
                    point_d const cellCenter {cx + 0.5, cy + 0.5};
                    point_d const toLightXY {lt.Position - cellCenter};

                    auto const slp {slopes_at(point_i {cx, cy})};
                    f64 const  lowZ {std::min({slp.Floor.TL, slp.Floor.TR, slp.Floor.BL, slp.Floor.BR})};
                    f64 const  highZ {std::max({slp.Ceiling.TL, slp.Ceiling.TR, slp.Ceiling.BL, slp.Ceiling.BR})};
                    f64 const  dz {std::max({lowZ - lt.Z, lt.Z - highZ, 0.0})};

                    f64 const distSq {toLightXY.dot(toLightXY) + (dz * dz)};
                    if (distSq >= sweep.RadiusSq) { continue; }

                    if (cell_has_any_line_of_sight(point_i {cx, cy}, lt.Position)) {
                        usize const cellIndex {static_cast<usize>((cy * MAP_WIDTH) + cx)};
                        _cellLights[cellIndex].push_back(static_cast<u32>(li));
                    }
                }
            }
        }
    },
                              MAP_HEIGHT);
}

auto level::surface_at(point_i idx) const -> cell_surface
{
    if (!map_t::Size.contains(idx)) { return {}; }

    return std::visit([](auto&& c) -> cell_surface {
        cell_surface s {};
        if constexpr (requires { c.FloorTexture; }) { s.FloorTexture = c.FloorTexture; }
        if constexpr (requires { c.CeilingTexture; }) { s.CeilingTexture = c.CeilingTexture; }
        if constexpr (requires { c.FloorReflectivity; }) { s.FloorReflectivity = c.FloorReflectivity; }
        if constexpr (requires { c.FloorZ; }) { s.FloorZ = c.FloorZ; }
        if constexpr (requires { c.CeilingZ; }) { s.CeilingZ = c.CeilingZ; }
        if constexpr (requires { c.Motes; }) { s.Motes = c.Motes; }
        return s;
    },
                      get_cell(idx));
}

auto level::is_clear(point_d pos, f64 radius, uid ignore) const -> bool
{
    return is_wall_clear(pos, radius) && hit_test(pos, radius, ignore) == nullptr;
}

auto level::is_wall_clear(point_d pos, f64 radius) const -> bool
{
    i32 const minCellX {static_cast<i32>(std::floor(pos.X - radius))};
    i32 const maxCellX {static_cast<i32>(std::floor(pos.X + radius))};
    i32 const minCellY {static_cast<i32>(std::floor(pos.Y - radius))};
    i32 const maxCellY {static_cast<i32>(std::floor(pos.Y + radius))};

    for (i32 ty {minCellY}; ty <= maxCellY; ++ty) {
        for (i32 tx {minCellX}; tx <= maxCellX; ++tx) {
            if (tx < 0 || tx >= map_t::Size.Width || ty < 0 || ty >= map_t::Size.Height) { return false; }

            auto const closest {closest_point_on_wall({tx, ty}, pos)};
            if (!closest) { continue; }

            point_d const d {pos.X - closest->X, pos.Y - closest->Y};
            if (d.dot(d) < radius * radius) { return false; }
        }
    }

    return true;
}

auto level::is_blocking_light(point_i idx) const -> bool
{
    if (!map_t::Size.contains(idx)) { return {}; }

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
                      get_cell(idx));
}

auto level::closest_point_on_wall(point_i map, point_d pos) const -> std::optional<point_d>
{
    rect_d const clampRect {static_cast<f64>(map.X), static_cast<f64>(map.Y), 1.0, 1.0};

    return overloaded_visit(
        get_cell(map),
        [](floor_cell const&) -> std::optional<point_d> { return std::nullopt; },
        [&](normal_wall const&) -> std::optional<point_d> {
            return point_d {std::clamp(pos.X, clampRect.left(), clampRect.right()),
                            std::clamp(pos.Y, clampRect.top(), clampRect.bottom())};
        },
        [&](obstacle const& w) -> std::optional<point_d> {
            std::optional<point_d> closest {};
            f64                    minDistSq {std::numeric_limits<f64>::infinity()};

            for (auto const& def : w.Shapes) {
                rect_d r {def.LocalBounds};
                r.move_by(map);

                point_d candidate {};

                if (def.IsRound) {
                    point_d const center {r.center()};
                    auto const [a, b] {r.local_center()};
                    if (a <= 0.0 || b <= 0.0) {
                        candidate = center;
                    } else {
                        point_d const uv {(pos.X - center.X) / a, (pos.Y - center.Y) / b};
                        f64 const     len {std::sqrt(uv.dot(uv))};
                        if (len == 0.0) {
                            candidate = center;
                        } else {
                            point_d const onUnitCircle {uv.X / len, uv.Y / len};
                            candidate = point_d {center.X + (onUnitCircle.X * a), center.Y + (onUnitCircle.Y * b)};
                        }
                    }
                } else {
                    candidate = point_d {std::clamp(pos.X, r.left(), r.right()), std::clamp(pos.Y, r.top(), r.bottom())};
                }

                point_d const diff {pos - candidate};
                f64 const     distSq {diff.dot(diff)};
                if (distSq < minDistSq) {
                    minDistSq = distSq;
                    closest   = candidate;
                }
            }

            return closest;
        },
        [&](diagonal_wall const& w) -> std::optional<point_d> {
            f64 const     cX {static_cast<f64>(map.X)};
            f64 const     cY {static_cast<f64>(map.Y)};
            bool const    nwSe {w.Orientation == diagonal_wall::orientation::NorthWestToSouthEast};
            point_d const a {cX, nwSe ? cY : cY + 1.0};
            point_d const b {cX + 1.0, nwSe ? cY + 1.0 : cY};
            point_d const ab {b.X - a.X, b.Y - a.Y};
            f64 const     t {std::clamp(((pos.X - a.X) * ab.X + (pos.Y - a.Y) * ab.Y) / (ab.X * ab.X + ab.Y * ab.Y), 0.0, 1.0)};
            return point_d {a.X + (t * ab.X), a.Y + (t * ab.Y)};
        },
        [&](auto const& w) -> std::optional<point_d> {
            if (w.State != wall_state::Open) {
                return point_d {std::clamp(pos.X, clampRect.left(), clampRect.right()),
                                std::clamp(pos.Y, clampRect.top(), clampRect.bottom())};
            }
            return std::nullopt;
        });
}

void level::prepare_sky()
{
    auto const& sky {_skySettings};

    f64 const t {static_cast<f64>(_time.count()) / 1000.0};

    auto const direction {[](radian_d azimuth, radian_d elevation) -> point3_d {
        return {.X = elevation.cos() * azimuth.cos(), .Y = elevation.cos() * azimuth.sin(), .Z = elevation.sin()};
    }};

    auto const dot {[](point3_d const& a, point3_d const& b) { return (a.X * b.X) + (a.Y * b.Y) + (a.Z * b.Z); }};

    f64 const days {sky.TimeOfDay + (sky.DayLength > 0.0 ? t / sky.DayLength : 0.0)};
    f64 const timeOfDay {days - std::floor(days)};

    // the sun rises at +X, peaks at noon and sets at -X
    f64 const sunAngle {TAU * (timeOfDay - 0.25)};
    f64 const sx {std::cos(sunAngle)};
    f64 const sy {sky.SunTilt * std::sin(sunAngle)};
    f64 const sz {std::sin(sunAngle)};
    f64 const sunInv {1.0 / std::sqrt((sx * sx) + (sy * sy) + (sz * sz))};
    _sky.Sun = {.X = sx * sunInv, .Y = sy * sunInv, .Z = sz * sunInv};

    f64 const u {std::clamp((_sky.Sun.Z + 0.12) / 0.37, 0.0, 1.0)};
    _sky.Day           = u * u * (3.0 - (2.0 * u));
    _sky.Night         = 1.0 - _sky.Day;
    _sky.Dusk          = std::clamp(1.0 - (std::abs(_sky.Sun.Z) / 0.3), 0.0, 1.0);
    _sky.CosSun        = sky.SunSize.cos();
    _sky.Time          = t;
    _sky.Stars         = sky.StarDensity > 0.0 && _sky.Night > 0.02;
    _sky.StarDensity   = sky.StarDensity;
    _sky.StarSize      = sky.StarSize;
    _sky.StarTwinkle   = sky.StarTwinkle;
    _sky.Zenith        = sky.NightZenith.lerp(sky.DayZenith, _sky.Day);
    _sky.Horizon       = sky.NightHorizon.lerp(sky.DayHorizon, _sky.Day);
    _sky.SunColor      = sky.SunColor;
    _sky.DuskColor     = sky.DuskColor;
    _sky.CloudCoverage = sky.CloudCoverage;
    _sky.CloudScale    = sky.CloudScale;
    _sky.CloudSoftness = sky.CloudSoftness;
    _sky.CloudWind     = sky.CloudWind;
    _sky.CloudColor    = sky.CloudColor;

    _sky.Bodies.clear();
    for (auto const& body : sky.Bodies) {
        auto const dir {direction(body.Azimuth + (body.OrbitSpeed * t), body.Elevation)};
        _sky.Bodies.push_back({.Dir        = dir,
                               .CosR       = body.Radius.cos(),
                               .SinR       = body.Radius.sin(),
                               .SunDot     = dot(dir, _sky.Sun),
                               .Ambient    = body.Ambient,
                               .Lit        = body.Lit,
                               .Color      = body.Color,
                               .Noise      = body.Noise,
                               .NoiseScale = body.NoiseScale,
                               .NoiseSeed  = body.NoiseSeed,
                               .NoiseColor = body.NoiseColor});
    }
}

void level::toggle_wall(point_i idx)
{
    std::visit(
        [](auto&& w) {
            if constexpr (requires { w.toggle(); }) {
                w.toggle();
            }
        },
        _map[idx]);
}

auto level::get_visibility(point_i idx) const -> tile_visibility
{
    if (!map_t::Size.contains(idx)) { return tile_visibility::Hidden; }
    return _seen[idx];
}

void level::mark_visible(point_i idx)
{
    if (!map_t::Size.contains(idx)) { return; }
    std::atomic_ref<tile_visibility> {_seen[idx]}.store(tile_visibility::Visible, std::memory_order_relaxed);
}

void level::reveal_map()
{
    _seen.fill(tile_visibility::Explored);
}

void level::fade_vision()
{
    for (auto& s : _seen) {
        if (s == tile_visibility::Visible) {
            s = tile_visibility::Explored;
        }
    }
}

auto level::motes_at(point_i idx) const -> motes_settings const&
{
    u8 const motes {surface_at(idx).Motes};
    return _motes[motes < _motes.size() ? motes : 0];
}

void level::set_motes(u8 idx, motes_settings const& motes)
{
    if (idx >= _motes.size()) { _motes.resize(static_cast<usize>(idx) + 1); }
    _motes[idx] = motes;
}

auto level::sky() const -> sky_frame const&
{
    return _sky;
}

void level::set_sky(sky_settings const& sky)
{
    _skySettings = sky;
}

auto level::spawn(level_object_desc desc) -> uid
{
    uid const id {_nextId++};
    _objects.emplace_back(id, desc);
    return id;
}

auto level::find(uid id) -> level_object*
{
    auto const it {std::ranges::lower_bound(_objects, id, {}, &level_object::id)};
    return (it != _objects.end() && it->id() == id) ? &*it : nullptr;
}

auto level::objects() const -> std::span<level_object const>
{
    return _objects;
}

auto level::objects() -> std::span<level_object>
{
    return _objects;
}
auto level::time() const -> milliseconds
{
    return _time;
}
