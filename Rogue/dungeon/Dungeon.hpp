// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp"
#include "Tile.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

struct light_source {
    color   Color {colors::White};
    f32     Intensity {1};
    f64     Range {3};
    bool    Falloff {true};
    point_i Position {point_i::Zero};
};

////////////////////////////////////////////////////////////

class dungeon {
public:
    dungeon();

    auto is_line_of_sight(point_i start, point_i end) -> bool;
    auto is_passable(point_i pos) const -> bool;
    auto find_path(point_i start, point_i target) const -> std::vector<point_i>;

    auto tiles() -> grid<tile>&;
    auto tiles() const -> grid<tile> const&;

    auto objects() -> std::vector<std::shared_ptr<object>>&;
    auto objects() const -> std::vector<std::shared_ptr<object>> const&;

    auto monsters() -> std::vector<std::shared_ptr<monster>>&;
    auto monsters() const -> std::vector<std::shared_ptr<monster>> const&;

    auto lights() -> std::vector<std::shared_ptr<light_source>>&;
    auto lights() const -> std::vector<std::shared_ptr<light_source>> const&;

private:
    grid<tile> _tiles;
    u64        _seed;

    std::vector<std::shared_ptr<object>>  _objects;
    std::vector<std::shared_ptr<monster>> _monsters;

    std::vector<std::shared_ptr<light_source>> _lights;
};

}
