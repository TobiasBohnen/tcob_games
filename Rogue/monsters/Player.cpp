// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Player.hpp"

#include "../level/Level.hpp"

namespace Rogue {

player::player() = default;

void player::update(milliseconds deltaTime)
{
}

auto player::try_move(point_i pos, level const& level) -> bool
{
    if (level.is_passable(pos)) {
        Position = pos;
        return true;
    }

    return false;
}

auto player::stats() const -> statistics const&
{
    return _stats;
}

auto player::symbol() const -> string
{
    return "@";
}

auto player::color() const -> tcob::color
{
    return colors::White;
}

}
