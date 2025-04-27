// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Player.hpp"

#include "../MasterControl.hpp"
#include "../dungeon/Dungeon.hpp"
#include "../dungeon/Object.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

player::player(master_control& parent)
    : _parent {parent}
{
}

void player::update(milliseconds deltaTime)
{
    if (_animationTimer <= 0s) { return; }

    _animationTimer -= deltaTime;
    if (_animationTimer <= 0s) {
        if (!_path.empty()) {
            if (!try_move(_path.front())) {
                _path.clear();
                FinishedPath();
            } else {
                _path.erase(_path.begin());
            }
            if (_path.empty()) {
                FinishedPath();
            } else {
                _animationTimer = ANIMATION_DELAY;
            }
        }
    }
}

auto player::busy() const -> bool
{
    return !_path.empty();
}

void player::start_path(std::vector<point_i> const& path)
{
    _path           = path;
    _animationTimer = ANIMATION_DELAY;
}

auto player::try_move(point_i pos) -> bool
{
    if (_parent.current_dungeon().is_passable(pos)) {
        Position = pos;
        EndTurn();
        return true;
    }

    return false;
}

auto player::try_pickup(std::shared_ptr<item> const& item) -> bool
{
    if (!item->can_pickup(*this)) { return false; }

    auto const message {item->pickup(*this)};

    bool const stackable {item->can_stack()};
    bool       added {false};

    if (stackable) {
        for (auto& [inv_item, count] : _profile.Inventory) {
            if (inv_item->type() == item->type() && inv_item->name() == item->name()) {
                count += item->amount();
                added = true;
                break;
            }
        }
    }

    if (!added) {
        _profile.Inventory.emplace_back(item, item->amount());
    }

    if (!message.empty()) {
        _parent.log(message);
    }

    EndTurn();
    return true;
}

auto player::current_profile() -> profile
{
    // TODO: add inventory, effects, etc.
    return _profile;
}

auto player::symbol() const -> string
{
    return "@";
}

auto player::color() const -> tcob::color
{
    f32 const ratio {static_cast<f32>(_profile.HP) / static_cast<f32>(hp_max())};
    return tcob::color::Lerp(colors::Red, colors::White, ratio);
}

auto player::light_color() const -> tcob::color
{
    return colors::White;
}

auto player::current_level() const -> i32
{
    return static_cast<i32>((1.0f + std::sqrt(1.0f + (4.0f * (static_cast<f32>(_profile.XP) / XP_SCALE)))) * 0.5f);
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

auto player::count_gold() const -> i32
{
    i32 retValue {0};
    for (auto const& [item, count] : _profile.Inventory) {
        if (item->type() == item_type::Gold) {
            retValue += count;
        }
    }
    return retValue;
}
}
