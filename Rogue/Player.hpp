// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

namespace Rogue {

class player {
public:
    explicit player(level& level);

    color              Color {colors::White};
    point_i            Position {};
    std::array<f32, 3> LightFalloff {9, 7, 5};

    void update(milliseconds deltaTime);

    auto try_move(point_i pos) -> bool;

private:
    level& _level;
};

}