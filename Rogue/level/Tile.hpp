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
    Wall,
    Glass
};

struct tile {
    tile_type Type;
    string    Symbol {};
    color     ForegroundColor {colors::White};
    color     BackgroundColor {colors::Black};

    bool Seen {false};
    bool InSight {false};

    auto is_passable() const -> bool;
    auto is_transparent() const -> bool;
};

////////////////////////////////////////////////////////////

static constinit tile const FLOOR {.Type = tile_type::Floor, .Symbol = ".", .ForegroundColor = colors::Gray, .BackgroundColor = colors::SeaShell};
static constinit tile const WALL {.Type = tile_type::Wall, .Symbol = "\u2592", .ForegroundColor = colors::Black};
static constinit tile const WALL0 {.Type = tile_type::Wall, .Symbol = "\u2592", .ForegroundColor = colors::Black, .BackgroundColor = colors::Silver};
static constinit tile const GLASS {.Type = tile_type::Glass, .Symbol = "\u2591"};

static constinit std::array<color, 3> const WALL_COLORS {{colors::DimGray, colors::Silver, colors::LightSlateGray}};

////////////////////////////////////////////////////////////

}
