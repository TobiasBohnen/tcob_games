// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

class player {
public:
    player();

    color   Color {colors::White};
    point_i Position {};

    void update(milliseconds deltaTime);

    auto try_move(point_i pos, level const& level) -> bool;

    auto get_name() const -> string const&;

    auto get_hp() const -> i32;
    auto get_hp_max() const -> i32;

    auto get_mp() const -> i32;
    auto get_mp_max() const -> i32;

    auto get_visual_range() const -> std::span<f32 const>;

private:
    std::array<f32, 3> _visualRange {8, 7, 5};

    string _name {"Tim"};

    i32 _hp {75};
    i32 _hpMax {100};

    i32 _mp {25};
    i32 _mpMax {100};
};

}