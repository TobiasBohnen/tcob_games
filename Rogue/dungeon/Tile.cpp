// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Tile.hpp"

#include "Object.hpp" // IWYU pragma: keep

namespace Rogue {

auto tile::is_passable() const -> bool
{
    if (Monster) { return false; }

    switch (Type) {
    case tile_type::Wall:  return false;
    case tile_type::Glass: return false;
    case tile_type::Floor: break;
    }

    return std::ranges::none_of(Objects, [](auto const& val) { return val->is_blocking(); });
}

auto tile::is_transparent() const -> bool
{
    switch (Type) {
    case tile_type::Wall:  return false;
    case tile_type::Floor:
    case tile_type::Glass: break;
    }

    return std::ranges::none_of(Objects, [](auto const& val) { return val->is_blocking(); });
}

}
