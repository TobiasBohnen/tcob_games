// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Tile.hpp"

namespace Rogue {

auto tile::is_passable() const -> bool
{
    switch (Type) {
    case tile_type::Floor: return true;
    case tile_type::Wall: return false;
    case tile_type::Glass: return false;
    }
    return false;
}

auto tile::is_transparent() const -> bool
{
    switch (Type) {
    case tile_type::Floor: return true;
    case tile_type::Wall: return false;
    case tile_type::Glass: return true;
    }
    return false;
}

}
