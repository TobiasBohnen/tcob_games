// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Object.hpp"

namespace Rogue {

////////////////////////////////////////////////////////////

auto door::symbol() const -> string
{
    return _open ? "-" : "+";
}

auto door::colors() const -> color_pair
{
    return {colors::White, colors::Black};
}

auto door::is_blocking() const -> bool
{
    return !_open;
}

auto door::can_interact(player& player) const -> bool
{
    return true;
}

auto door::interact(player& player) -> string
{
    _open = !_open;
    return _open ? "you opened the door" : "you closed the door";
}

////////////////////////////////////////////////////////////

gold::gold(i32 amount)
    : _amount {amount}
{
}

auto gold::symbol() const -> string
{
    return "$";
}

auto gold::colors() const -> color_pair
{
    return {colors::Gold, colors::Black};
}

auto gold::can_pickup(player& player) const -> bool
{
    return true;
}

auto gold::pickup(player& player) -> string
{
    // player.give();
    return std::format("you picked up {} gold coins", _amount);
}

////////////////////////////////////////////////////////////

}
