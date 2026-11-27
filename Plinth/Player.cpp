// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Player.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>

#include "Common.hpp"
#include "Level.hpp"
#include "LevelObject.hpp"
#include "TextureCache.hpp"

void player::move(level const& lvl, f64 forwardAmount, f64 strafeAmount, f64 rotateAmount)
{
    // a position is reachable if the step up is climbable and nothing blocks it
    auto const can_go {[&](point_d const& pos) { return lvl.can_move_to(Position, pos, Radius, EyeZ, false, 0); }};

    // full move first, then slide along either axis
    auto const try_move {[&](point_d const& newPos) {
        if (can_go(newPos)) {
            Position = newPos;
        } else if (can_go({Position.X, newPos.Y})) {
            Position.Y = newPos.Y;
        } else if (can_go({newPos.X, Position.Y})) {
            Position.X = newPos.X;
        } else {
            return;
        }
        _doBob = true;
    }};

    // Move Forward/Backward
    if (forwardAmount != 0) { try_move(Position + (Direction * forwardAmount)); }

    // Strafe Left/Right (perpendicular to Direction)
    if (strafeAmount != 0) {
        point_d const strafe {-Direction.as_perpendicular()};
        try_move(Position + (strafe * strafeAmount));
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

void player::fire(level& level) const
{
    constexpr f64 BALL_SIZE {0.15};

    sprite_object const sprite {.Size      = {BALL_SIZE, BALL_SIZE},
                                .Texture   = ballTexture,
                                .LightMask = LIGHT_LAYER_SPELL};

    // the light sits in the middle of the ball: an offset from the projectile
    dynamic_light const light {.Position  = {.Z = BALL_SIZE * 0.5},
                               .Range     = 2.5,
                               .Color     = colors::Red,
                               .Intensity = 1.5,
                               .Layers    = LIGHT_LAYER_WORLD | LIGHT_LAYER_SPELL};

    f64 const forwardOffset {0.4};
    f64 const rightOffset {0.2};

    f64 const fwdX {Direction.X};
    f64 const fwdY {Direction.Y};
    f64 const rightX {-fwdY};
    f64 const rightY {fwdX};
    f64 const spawnX {Position.X + (forwardOffset * fwdX) + (rightOffset * rightX)};
    f64 const spawnY {Position.Y + (forwardOffset * fwdY) + (rightOffset * rightY)};

    object_desc const desc {.Position = {.X = spawnX, .Y = spawnY, .Z = EyeZ - 0.15},
                            .Visual   = sprite,
                            .LifeTime = 5s};

    level.spawn<projectile>(desc, 0.075, Direction * 10, uid {}, light);
}
