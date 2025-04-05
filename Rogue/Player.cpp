// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Player.hpp"

#include "Level.hpp"

namespace Rogue {

player::player(level& level)
    : _level {level}
{
}

void player::update(milliseconds deltaTime)
{
}

auto player::try_move(point_i pos) -> bool
{
    if (_level.is_passable(pos)) {
        Position = pos;
        return true;
    }

    return false;
}
}
