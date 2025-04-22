// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Object.hpp"

#include "../monsters/Player.hpp"

namespace Rogue {

////////////////////////////////////////////////////////////

door::door(bool open)
    : _open {open}
{
}

auto door::symbol() const -> string
{
    return _open ? "'" : "+";
}

auto door::colors() const -> color_pair
{
    return {colors::Sienna, colors::Transparent};
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
    return {colors::Gold, colors::Transparent};
}

auto gold::can_pickup(player& player) const -> bool
{
    return true;
}

auto gold::pickup(player& player) -> string
{
    return std::format("you picked up {} gold coins", _amount);
}

auto gold::can_stack() const -> bool
{
    return true;
}

auto gold::type() const -> item_type
{
    return item_type::Gold;
}

auto gold::name() const -> string
{
    return "Gold";
}

auto gold::amount() const -> i32
{
    return _amount;
}

////////////////////////////////////////////////////////////

wand::wand(quality quality)
{
}

auto wand::symbol() const -> string
{
    return "-";
}

auto wand::colors() const -> color_pair
{
    return {colors::Blue, colors::Transparent}; // TODO
}

auto wand::can_pickup(player& player) const -> bool
{
    return true; // TODO
}

auto wand::pickup(player& player) -> string
{
    return std::format("you picked up a wand");
}

auto wand::type() const -> item_type
{
    return item_type::Weapon;
}

auto wand::name() const -> string
{
    return "Wand";
}

////////////////////////////////////////////////////////////

rod::rod(quality quality)
{
}

auto rod::symbol() const -> string
{
    return "/";
}

auto rod::colors() const -> color_pair
{
    return {colors::Blue, colors::Transparent}; // TODO
}

auto rod::can_pickup(player& player) const -> bool
{
    return true; // TODO
}

auto rod::pickup(player& player) -> string
{
    return std::format("you picked up a rod");
}

auto rod::type() const -> item_type
{
    return item_type::Weapon;
}

auto rod::name() const -> string
{
    return "Rod";
}

////////////////////////////////////////////////////////////

staff::staff(quality quality)
{
}

auto staff::symbol() const -> string
{
    return "|";
}

auto staff::colors() const -> color_pair
{
    return {colors::Blue, colors::Transparent}; // TODO
}

auto staff::can_pickup(player& player) const -> bool
{
    return true; // TODO
}

auto staff::pickup(player& player) -> string
{
    return std::format("you picked up a staff");
}

auto staff::type() const -> item_type
{
    return item_type::Weapon;
}

auto staff::name() const -> string
{
    return "Staff";
}

////////////////////////////////////////////////////////////

}
