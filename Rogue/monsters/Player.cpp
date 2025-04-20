// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Player.hpp"

#include "../level/Level.hpp"

namespace Rogue {

constexpr i32 XP_SCALE {50};
constexpr i32 VIT_SCALE {10};
constexpr i32 INT_SCALE {15};

auto player::profile::level() const -> i32
{
    return static_cast<i32>((1.0f + std::sqrt(1.0f + 4.0f * (static_cast<f32>(XP) / XP_SCALE))) * 0.5f);
}

auto player::profile::xp_required_for(i32 level) const -> i32
{
    return XP_SCALE * (level - 1) * level;
}

auto player::profile::hp_max() const -> i32
{
    // Base 100 HP + 10 * Vitality per level
    return 100 + ((level() - 1) * VIT_SCALE * Attributes.Vitality);
}

auto player::profile::mp_max() const -> i32
{
    // Base 50 MP + 15 * Intelligence per level
    return 50 + ((level() - 1) * INT_SCALE * Attributes.Intelligence);
}

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
    _stats.Gold += amount;
}

auto player::stats() const -> profile const&
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
