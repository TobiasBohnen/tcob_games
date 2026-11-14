// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"
#include "Player.hpp"

// World to camera space, with the inverse determinant of the camera matrix [Direction | Plane]:
// [lateral]   1/det * [  Direction.Y  -Direction.X ] * [rel.X]
// [depth  ] =         [ -Plane.Y       Plane.X     ]   [rel.Y]
// Lateral is in plane units, depth is the distance along the view direction.
// Built once per frame from the player's camera; it holds copies, so it never dangles.
class camera {
public:
    struct projected {
        f64 Lateral {};
        f64 Depth {}; // zero or less: not in front of the camera, Scale and ScreenX stay unset
        f64 Scale {}; // screen pixels per world unit at this depth
        f64 ScreenX {};
    };

    camera() = default;

    camera(player const& pl, i32 screenWidth, f64 projPlaneDist, i32 screenCenterY);

    auto project(point_d const& world) const -> projected;

    auto screen_y(f64 worldZ, projected const& p) const -> f64;

    auto max_shift_per_unit() const -> f64; // upper bound for how far a world-space offset of length 1 can move a point in camera space

private:
    point_d _position {};
    point_d _direction {};
    point_d _plane {};
    f64     _eyeZ {};
    f64     _invDet {};
    f64     _halfWidth {};
    f64     _projPlaneDist {};
    i32     _centerY {};
};
