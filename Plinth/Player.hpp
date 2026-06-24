// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

class player {
public:
    point_d Pos {};
    point_d Direction {};
    point_d Plane {};

    f64 ProjPlaneDist {};

    auto move(level const& level, f64 forwardAmount, f64 strafeAmount, f64 rotateAmount) -> bool;

    static constexpr f64 Radius {0.25};
};
