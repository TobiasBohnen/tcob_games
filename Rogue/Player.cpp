// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Player.hpp"

#include "Level.hpp"

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

auto player::get_name() const -> string const&
{
    return _name;
}

auto player::get_hp() const -> i32
{
    return _hp;
}

auto player::get_hp_max() const -> i32
{
    return _hpMax;
}

auto player::get_mp() const -> i32
{
    return _mp;
}

auto player::get_mp_max() const -> i32
{
    return _mpMax;
}

auto player::get_visual_range() const -> std::span<f32 const>
{
    return _visualRange;
}

}
