// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

struct player_settings {
    f64 MoveSpeed {4.0};

    f64 BobHeight {0};

    f64 LightRange {8.0};
    f64 LightIntensity {4.0};
};

class player {
public:
    player_settings Settings {};

    point_d Position {};
    f64     EyeZ {0.0};
    point_d Direction {};
    point_d Plane {};

    bool Cheater {false};

    void move(level const& level, f64 forwardAmount, f64 strafeAmount, f64 rotateAmount);
    void update(milliseconds deltaTime);

    auto bob_amount() const -> f64;

    static constexpr f64 Radius {0.25};

private:
    bool _doBob {false};
    f64  _bobPhase {0.0};
    f64  _bobAmount {0.0};
};
