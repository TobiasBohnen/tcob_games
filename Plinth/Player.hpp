// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

struct player_settings {
    f64 MoveSpeed {4.0};
    f64 RotateSpeed {3.0};

    f64 BobHeight {6.0};
};

class player {
public:
    player_settings Settings {};

    point_d Position {};
    point_d Direction {};
    point_d Plane {};

    bool Cheater {false};

    f64 BobAmount {0.0};

    void move(level const& level, f64 forwardAmount, f64 strafeAmount, f64 rotateAmount);
    void bob(milliseconds deltaTime);

    static constexpr f64 Radius {0.25};

private:
    bool _isMoving {false};
    f64  _bobPhase {0.0};
};
