// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Player.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "Common.hpp"
#include "Level.hpp"

void player::move(level const& level, f64 forwardAmount, f64 strafeAmount, f64 rotateAmount)
{
    // Move Forward/Backward
    if (forwardAmount != 0) {
        point_d const newPos {Position + (Direction * forwardAmount)};

        if (level.is_clear(newPos, Radius)) {
            Position = newPos;
            _doBob   = true;
        } else if (level.is_clear({Position.X, newPos.Y}, Radius)) {
            Position.Y = newPos.Y;
            _doBob     = true;
        } else if (level.is_clear({newPos.X, Position.Y}, Radius)) {
            Position.X = newPos.X;
            _doBob     = true;
        }
    }

    // Strafe Left/Right (perpendicular to Direction)
    if (strafeAmount != 0) {
        point_d const strafe {-Direction.as_perpendicular()};
        point_d const newPos {Position + (strafe * strafeAmount)};

        if (level.is_clear(newPos, Radius)) {
            Position = newPos;
            _doBob   = true;
        } else if (level.is_clear({Position.X, newPos.Y}, Radius)) {
            Position.Y = newPos.Y;
            _doBob     = true;
        } else if (level.is_clear({newPos.X, Position.Y}, Radius)) {
            Position.X = newPos.X;
            _doBob     = true;
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
    }
}

void player::update(milliseconds deltaTime)
{
    constexpr f64 bobSpeed {8.0};

    auto const dt {deltaTime.count() / 1000};

    if (_doBob) {
        _bobPhase += dt * bobSpeed;
        _bobAmount = std::abs(std::sin(_bobPhase)) * Settings.BobHeight;
        _doBob     = false;
        return;
    }
    if (_bobAmount != 0.0) {
        _bobPhase  = 0.0;
        _bobAmount = std::max(_bobAmount - (dt * Settings.BobHeight * 4.0), 0.0);
    }
}

auto player::bob_amount() const -> f64
{
    return _bobAmount;
}
