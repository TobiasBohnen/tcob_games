// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

class player {
public:
    ////////////////////////////////////////////////////////////

    struct statistics {
        string Name {"Tim"};

        i32 HP {75};
        i32 HPMax {100};

        i32 MP {25};
        i32 MPMax {100};

        f32 VisualRange {7};
    };

    ////////////////////////////////////////////////////////////

    player();

    point_i Position {};

    void update(milliseconds deltaTime);

    auto try_move(point_i pos, level const& level) -> bool;

    auto stats() const -> statistics const&;

    auto symbol() const -> string;
    auto color() const -> color;

private:
    statistics _stats;
};

}