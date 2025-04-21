// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Player.hpp"

#include "../level/Level.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

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

auto player::can_add_item(inv_item const& item) const -> bool
{

    return true;
}

void player::add_item(std::shared_ptr<inv_item> const& item)
{
    _inventory.push_back(item);
}

void player::add_gold(i32 amount)
{
    _profile.Gold += amount;
}

auto player::inventory() const -> std::vector<std::shared_ptr<inv_item>> const&
{
    return _inventory;
}

auto player::current_profile() const -> profile
{
    // TODO: add inventory, etc.
    return _profile;
}

auto player::symbol() const -> string
{
    return "@";
}

auto player::color() const -> tcob::color
{
    return colors::White;
}

auto player::current_level() const -> i32
{
    return static_cast<i32>((1.0f + std::sqrt(1.0f + 4.0f * (static_cast<f32>(_profile.XP) / XP_SCALE))) * 0.5f);
}

auto player::hp_max() const -> i32
{
    // Base 100 HP + 10 * Vitality per level
    return 100 + ((current_level() - 1) * VIT_SCALE * _profile.Attributes.Vitality);
}

auto player::mp_max() const -> i32
{
    // Base 50 MP + 15 * Intelligence per level
    return 50 + ((current_level() - 1) * INT_SCALE * _profile.Attributes.Intelligence);
}

}
