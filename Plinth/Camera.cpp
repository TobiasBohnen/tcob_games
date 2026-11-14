// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Camera.hpp"

#include <cmath>
#include <cstdlib>

#include "Common.hpp"
#include "Player.hpp"

camera::camera(player const& pl, i32 screenWidth, f64 projPlaneDist, i32 screenCenterY)
    : _position {pl.Position}
    , _direction {pl.Direction}
    , _plane {pl.Plane}
    , _eyeZ {pl.EyeZ}
    , _invDet {1.0 / pl.Plane.cross(pl.Direction)}
    , _halfWidth {screenWidth / 2.0}
    , _projPlaneDist {projPlaneDist}
    , _centerY {screenCenterY}
{
}

auto camera::project(point_d const& world) const -> projected
{
    auto const rel {world - _position};

    projected p {.Lateral = _invDet * rel.cross(_direction), .Depth = _invDet * _plane.cross(rel)};
    if (p.Depth > 0.0) {
        p.Scale   = _projPlaneDist / p.Depth;
        p.ScreenX = _halfWidth * (1.0 + (p.Lateral / p.Depth));
    }
    return p;
}
auto camera::screen_y(f64 worldZ, projected const& p) const -> f64 { return _centerY - ((worldZ - _eyeZ) * p.Scale); }
auto camera::max_shift_per_unit() const -> f64 { return std::abs(_invDet) * (std::sqrt(_plane.dot(_plane)) + std::sqrt(_direction.dot(_direction))); }
