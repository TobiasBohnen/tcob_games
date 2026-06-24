// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Player.hpp"

#include "Common.hpp"
#include "Level.hpp"
#include "Walls.hpp"

static auto is_position_clear(level const& level, point_d pos, f64 radius) -> bool
{
    // wall collision
    i32 const minTileX {static_cast<i32>(std::floor(pos.X - radius))};
    i32 const maxTileX {static_cast<i32>(std::floor(pos.X + radius))};
    i32 const minTileY {static_cast<i32>(std::floor(pos.Y - radius))};
    i32 const maxTileY {static_cast<i32>(std::floor(pos.Y + radius))};

    for (i32 ty {minTileY}; ty <= maxTileY; ++ty) {
        for (i32 tx {minTileX}; tx <= maxTileX; ++tx) {
            if (tx < 0 || tx >= map_t::Size.Width || ty < 0 || ty >= map_t::Size.Height) { return false; }
            auto const& cell {level.Map[point_i {tx, ty}]};
            rect_d      clampRect {static_cast<f64>(tx), static_cast<f64>(ty), 1.0, 1.0};
            bool const  isSolid {overloaded_visit(
                cell,
                [](empty const&) { return false; },
                [](normal_wall const&) { return true; },
                [&clampRect, tx, ty](half_wall const& w) {
                    clampRect = w.LocalBounds;
                    clampRect.move_by(point_i {tx, ty});
                    return true;
                },
                [](auto const& w) {
                    return (w.State != wall_state::Open);
                })};

            if (!isSolid) { continue; }

            f64 const closestX {std::clamp(pos.X, clampRect.left(), clampRect.right())};
            f64 const closestY {std::clamp(pos.Y, clampRect.top(), clampRect.bottom())};
            f64 const dx {pos.X - closestX};
            f64 const dy {pos.Y - closestY};

            if (((dx * dx) + (dy * dy)) < (radius * radius)) {
                return false;
            }
        }
    }

    // sprite collision check
    return std::ranges::all_of(level.Sprites, [&](sprite const& spr) {
        if (!spr.Solid) { return true; }

        f64 const     combinedRadius {radius + (spr.Size.Width / 2.0)};
        point_d const d {spr.Pos - pos};
        return d.dot(d) >= combinedRadius * combinedRadius;
    });
}

auto player::move(level const& level, f64 forwardAmount, f64 strafeAmount, f64 rotateAmount) -> bool
{
    bool retValue {false};

    // Move Forward/Backward
    if (forwardAmount != 0) {
        point_d const newPos {Pos + (Direction * forwardAmount)};

        if (is_position_clear(level, newPos, Radius)) {
            Pos      = newPos;
            retValue = true;
        } else if (is_position_clear(level, {Pos.X, newPos.Y}, Radius)) {
            Pos.Y    = newPos.Y;
            retValue = true;
        } else if (is_position_clear(level, {newPos.X, Pos.Y}, Radius)) {
            Pos.X    = newPos.X;
            retValue = true;
        }
    }

    // Strafe Left/Right (perpendicular to _dir)
    if (strafeAmount != 0) {
        point_d const strafe {Direction.as_perpendicular()};
        point_d const newPos {Pos + (strafe * strafeAmount)};

        if (is_position_clear(level, newPos, Radius)) {
            Pos      = newPos;
            retValue = true;
        } else if (is_position_clear(level, {Pos.X, newPos.Y}, Radius)) {
            Pos.Y    = newPos.Y;
            retValue = true;
        } else if (is_position_clear(level, {newPos.X, Pos.Y}, Radius)) {
            Pos.X    = newPos.X;
            retValue = true;
        }
    }

    // Rotate (both direction and plane vectors must rotate together)
    if (rotateAmount != 0) {
        f64 const old_dirX {Direction.X};
        Direction.X = (Direction.X * std::cos(rotateAmount)) - (Direction.Y * std::sin(rotateAmount));
        Direction.Y = (old_dirX * std::sin(rotateAmount)) + (Direction.Y * std::cos(rotateAmount));

        f64 const oldPlaneX {Plane.X};
        Plane.X = (Plane.X * std::cos(rotateAmount)) - (Plane.Y * std::sin(rotateAmount));
        Plane.Y = (oldPlaneX * std::sin(rotateAmount)) + (Plane.Y * std::cos(rotateAmount));

        retValue = true;
    }

    return retValue;
}
