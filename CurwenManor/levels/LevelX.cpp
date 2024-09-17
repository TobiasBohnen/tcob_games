// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Level1.hpp"

#include "../GameCanvas.hpp"

namespace stn {

level1::level1(game_scene* parent)
    : base_level {parent}
{
}

void level1::update(milliseconds deltaTime)
{
    _currentTime += deltaTime;
}

void level1::draw(canvas& canvas)
{
}

}
