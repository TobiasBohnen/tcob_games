// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Object.hpp"

#include "../actors/Actor.hpp"
#include "Tile.hpp"

namespace Rogue {

////////////////////////////////////////////////////////////

door::door(bool visible, bool open)
    : _visible {visible}
    , _open {open}
{
}

auto door::symbol() const -> string
{
    if (!_visible) { return WALL.Symbol; }
    return _open ? "'" : "+";
}

auto door::colors() const -> color_pair
{
    if (!_visible) { return {WALL.ForegroundColor, WALL_COLORS[0]}; }
    return {colors::Sienna, colors::Black};
}

auto door::is_blocking() const -> bool
{
    if (!_visible) { return true; }
    if (!_open) { return true; }
    return false;
}

auto door::can_interact(actor& actor) const -> bool
{
    return _visible;
}

auto door::interact(actor& actor) -> log_message
{
    if (can_interact(actor)) {
        _open = !_open;
        return _open ? log_message {"you opened the door"} : log_message {"you closed the door"};
    }

    return {};
}

auto door::on_enter(actor& actor) -> log_message
{
    if (!_visible) { return {}; }
    return !_open ? interact(actor) : log_message {};
}

auto door::on_search(actor& actor) -> log_message
{
    if (!_visible) {
        if (actor.intelligence_check(0.5f)) {
            _visible = true;
            return {"discovered a door"};
        }
    }

    return {};
}

////////////////////////////////////////////////////////////

trap::trap(bool visible, trap_type type)
    : _visible {visible}
    , _type {type}
{
}

auto trap::symbol() const -> string
{
    return _visible ? "^" : "";
}

auto trap::colors() const -> color_pair
{
    return {colors::Red, colors::Transparent};
}

auto trap::can_interact(actor& actor) const -> bool
{
    return _visible;
}

auto trap::interact(actor& actor) -> log_message
{
    if (can_interact(actor)) {
        _armed = !_armed;
        return _armed ? log_message {"trap armed"} : log_message {"trap disarmed"};
    }

    return {};
}

auto trap::on_enter(actor& actor) -> log_message
{
    if (_armed) {
        _visible = true;
        return {"stepped on trap"};
    }

    return {};
}

auto trap::on_search(actor& actor) -> log_message
{
    if (!_visible) {
        if (actor.intelligence_check(0.05f)) {
            _visible = true;
            return {"discovered a trap"};
        }
    }

    return {};
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

auto gold::can_pickup(actor& actor) const -> bool
{
    return true;
}

auto gold::pickup(actor& actor) -> log_message
{
    return {std::format("you picked up {} gold coins", _amount)};
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

auto wand::can_pickup(actor& actor) const -> bool
{
    return true; // TODO
}

auto wand::pickup(actor& actor) -> log_message
{
    return {std::format("you picked up a wand")};
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

auto rod::can_pickup(actor& actor) const -> bool
{
    return true; // TODO
}

auto rod::pickup(actor& actor) -> log_message
{
    return {std::format("you picked up a rod")};
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

auto staff::can_pickup(actor& actor) const -> bool
{
    return true; // TODO
}

auto staff::pickup(actor& actor) -> log_message
{
    return {std::format("you picked up a staff")};
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
