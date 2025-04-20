// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Object.hpp"

#include "../monsters/Player.hpp"
#include "InvItem.hpp"

namespace Rogue {

////////////////////////////////////////////////////////////

auto door::symbol() const -> string
{
    return _open ? "'" : "+";
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
    return {colors::Gold, colors::Transparent};
}

auto gold::can_pickup(player& player) const -> bool
{
    return true;
}

auto gold::pickup(player& player) -> pickup_result
{
    player.add_gold(_amount);
    return {.Message = std::format("you picked up {} gold coins", _amount), .Item = nullptr};
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

auto wand::pickup(player& player) -> pickup_result
{
    return {.Message = std::format("you picked up the wand"), .Item = std::make_shared<weapon>()};
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

auto rod::pickup(player& player) -> pickup_result
{
    return {.Message = std::format("you picked up the rod"), .Item = std::make_shared<weapon>()};
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

auto staff::pickup(player& player) -> pickup_result
{
    return {.Message = std::format("you picked up the staff"), .Item = std::make_shared<weapon>()};
}

////////////////////////////////////////////////////////////

}
