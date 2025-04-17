// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

enum class tile_type {
    Floor,
    Wall
};

struct tile {
    tile_type Type;
    string    Symbol {};
    color     ForegroundColor {colors::White};
    color     BackgroundColor {colors::Black};
    bool      Seen {false};
    bool      InSight {false};

    std::vector<std::shared_ptr<object>> Objects;
    std::shared_ptr<monster>             Monster;
};

struct tile_traits {
    static auto passable(tile const& tile) -> bool
    {
        switch (tile.Type) {
        case tile_type::Floor: return true;
        case tile_type::Wall: return false;
        }
        return false;
    }
};

}
