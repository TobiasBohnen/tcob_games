// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Level.hpp"

#include <utility>

#include "Common.hpp"

level::level(map_t map)
    : _map {std::move(map)}
{
    // DEBUGCODE START
    Settings.CeilingTexture = 11;
    Settings.FloorTexture   = 10;
    Settings.IsSkybox       = false;
    // DEBUGCODE END
}

void level::update(milliseconds deltaSeconds)
{
    _messageTimer -= deltaSeconds;
    if (_messageTimer.count() <= 0) {
        _message.clear();
    }

    f64 const dt {deltaSeconds.count() / 1000};
    for (auto& cell : _map) {
        std::visit(
            [dt](auto&& w) {
                if constexpr (requires { w.update(dt); }) {
                    w.update(dt);
                }
            },
            cell);
    }
}

auto level::get_cell(point_i p) const -> cell const&
{
    return _map[p];
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

auto level::is_clear(point_d pos, f64 radius) const -> bool
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

    bool const spritesClear {std::ranges::all_of(Sprites, [&](sprite const& spr) {
        if (!spr.Solid) { return true; }
        f64 const     combinedRadius {radius + (spr.Size.Width / 2.0)};
        point_d const d {spr.Position - pos};
        return d.dot(d) >= combinedRadius * combinedRadius;
    })};

    if (!spritesClear) { return false; }

    return std::ranges::all_of(VoxelObjects, [&](voxel_object const& obj) {
        if (!obj.Grid) { return true; }

        f64 const     objRadius {(std::max(obj.Grid->Size.X, obj.Grid->Size.Y) / 2.0) * obj.Scale};
        f64 const     combinedRadius {radius + objRadius};
        point_d const d {obj.Position - pos};
        return d.dot(d) >= combinedRadius * combinedRadius;
    });
}

void level::toggle_wall(point_i p)
{
    std::visit(
        [](auto&& w) {
            if constexpr (requires { w.toggle(); }) {
                w.toggle();
            }
        },
        _map[p]);
}

auto level::is_seen(point_i cell) const -> bool
{
    if (!map_t::Size.contains(cell)) { return false; }
    return _seen[cell];
}

void level::mark_seen(point_i cell, point_d playerPos)
{
    if (!map_t::Size.contains(cell)) { return; }

    point_d const cellCenter {cell.X + 0.5, cell.Y + 0.5};
    point_d const delta {cellCenter.X - playerPos.X, cellCenter.Y - playerPos.Y};
    f64 const     dist {std::sqrt((delta.X * delta.X) + (delta.Y * delta.Y))};

    f64 const visibleRange {Settings.FogDistance * (1.0 - Settings.FogMin)};
    if (dist < visibleRange) {
        _seen[cell] = true;
    }
}

void level::mark_all_seen()
{
    _seen.fill(true);
}

void level::show_message(string const& msg)
{
    _messageTimer = 2s;
    _message      = msg;
}

auto level::get_message() const -> string const&
{
    return _message;
}
