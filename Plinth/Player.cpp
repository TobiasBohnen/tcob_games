// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Player.hpp"

#include "Common.hpp"
#include "Level.hpp"
#include "Walls.hpp"

static auto closest_point_on_wall(level const& level, point_i map, point_d pos) -> std::optional<point_d>
{
    auto const&  c {level.Map[map]};
    rect_d const clampRect {static_cast<f64>(map.X), static_cast<f64>(map.Y), 1.0, 1.0};

    return overloaded_visit(
        c,
        [](empty_cell const&) -> std::optional<point_d> { return std::nullopt; },
        [](cell const&) -> std::optional<point_d> { return std::nullopt; },
        [&](normal_wall const&) -> std::optional<point_d> {
            return point_d {std::clamp(pos.X, clampRect.left(), clampRect.right()),
                            std::clamp(pos.Y, clampRect.top(), clampRect.bottom())};
        },
        [&](box_wall const& w) -> std::optional<point_d> {
            rect_d r {w.LocalBounds};
            r.move_by(map);
            return point_d {std::clamp(pos.X, r.left(), r.right()),
                            std::clamp(pos.Y, r.top(), r.bottom())};
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
        [&](round_pillar const& w) -> std::optional<point_d> {
            point_d const center {map.X + 0.5, map.Y + 0.5};
            point_d const d {pos.X - center.X, pos.Y - center.Y};
            f64 const     len {std::sqrt(d.dot(d))};
            if (len == 0.0) { return center; }
            return point_d {center.X + ((d.X / len) * w.Radius), center.Y + ((d.Y / len) * w.Radius)};
        },
        [&](auto const& w) -> std::optional<point_d> {
            if (w.State != wall_state::Open) {
                return point_d {std::clamp(pos.X, clampRect.left(), clampRect.right()),
                                std::clamp(pos.Y, clampRect.top(), clampRect.bottom())};
            }
            return std::nullopt;
        });
}

static auto is_position_clear(level const& level, point_d pos, f64 radius) -> bool
{
    i32 const minTileX {static_cast<i32>(std::floor(pos.X - radius))};
    i32 const maxTileX {static_cast<i32>(std::floor(pos.X + radius))};
    i32 const minTileY {static_cast<i32>(std::floor(pos.Y - radius))};
    i32 const maxTileY {static_cast<i32>(std::floor(pos.Y + radius))};

    for (i32 ty {minTileY}; ty <= maxTileY; ++ty) {
        for (i32 tx {minTileX}; tx <= maxTileX; ++tx) {
            if (tx < 0 || tx >= map_t::Size.Width || ty < 0 || ty >= map_t::Size.Height) { return false; }

            auto const closest {closest_point_on_wall(level, {tx, ty}, pos)};
            if (!closest) { continue; }

            point_d const d {pos.X - closest->X, pos.Y - closest->Y};
            if (d.dot(d) < radius * radius) { return false; }
        }
    }

    return std::ranges::all_of(level.Sprites, [&](sprite const& spr) {
        if (!spr.Solid) { return true; }
        f64 const     combinedRadius {radius + (spr.Size.Width / 2.0)};
        point_d const d {spr.Position - pos};
        return d.dot(d) >= combinedRadius * combinedRadius;
    });
}

auto player::move(level const& level, f64 forwardAmount, f64 strafeAmount, f64 rotateAmount) -> bool
{
    _isMoving = false;

    // Move Forward/Backward
    if (forwardAmount != 0) {
        point_d const newPos {Position + (Direction * forwardAmount)};

        if (is_position_clear(level, newPos, Radius)) {
            Position  = newPos;
            _isMoving = true;
        } else if (is_position_clear(level, {Position.X, newPos.Y}, Radius)) {
            Position.Y = newPos.Y;
            _isMoving  = true;
        } else if (is_position_clear(level, {newPos.X, Position.Y}, Radius)) {
            Position.X = newPos.X;
            _isMoving  = true;
        }
    }

    // Strafe Left/Right (perpendicular to _dir)
    if (strafeAmount != 0) {
        point_d const strafe {-Direction.as_perpendicular()};
        point_d const newPos {Position + (strafe * strafeAmount)};

        if (is_position_clear(level, newPos, Radius)) {
            Position  = newPos;
            _isMoving = true;
        } else if (is_position_clear(level, {Position.X, newPos.Y}, Radius)) {
            Position.Y = newPos.Y;
            _isMoving  = true;
        } else if (is_position_clear(level, {newPos.X, Position.Y}, Radius)) {
            Position.X = newPos.X;
            _isMoving  = true;
        }
    }

    // Rotate (both direction and plane vectors must rotate together)
    if (rotateAmount != 0) {
        f64 const oldDirX {Direction.X};
        Direction.X = (Direction.X * std::cos(rotateAmount)) - (Direction.Y * std::sin(rotateAmount));
        Direction.Y = (oldDirX * std::sin(rotateAmount)) + (Direction.Y * std::cos(rotateAmount));

        f64 const oldPlaneX {Plane.X};
        Plane.X = (Plane.X * std::cos(rotateAmount)) - (Plane.Y * std::sin(rotateAmount));
        Plane.Y = (oldPlaneX * std::sin(rotateAmount)) + (Plane.Y * std::cos(rotateAmount));

        _isMoving = true;
    }

    return _isMoving;
}

auto player::bob(milliseconds deltaTime) -> bool
{
    constexpr f64 bobSpeed {8.0};
    constexpr f64 bobHeight {6.0}; // pixels

    auto const dt {deltaTime.count() / 1000};

    if (_isMoving) {
        _bobPhase += dt * bobSpeed;
        BobAmount = std::abs(std::sin(_bobPhase)) * bobHeight;
        return true;
    }
    if (BobAmount != 0.0) {
        _bobPhase = 0.0;
        BobAmount = std::max(BobAmount - (dt * bobHeight * 4.0), 0.0);
        return true;
    }
    return false;
}
