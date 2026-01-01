// Copyright (c) 2026 Tobias Bohnen
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

    auto is_line_of_sight(point_i start, point_i end) const -> bool;
    auto is_passable(point_i pos) const -> bool;
    auto find_path(point_i start, point_i target) const -> std::vector<point_i>;

    void add_object(std::shared_ptr<object> const& object);
    void remove_object(object const& object);

    void draw(renderer& renderer, point_i center, player const& player);

    auto tiles() -> grid<tile>&;

private:
    auto objects() const -> std::span<std::shared_ptr<object> const>;
    auto monsters() const -> std::span<std::shared_ptr<monster> const>;
    auto lights() const -> std::span<std::shared_ptr<light_source> const>;

    auto lighting(tile& tile, point_i gridPos, point_i playerPos, f32 playerRange, color playerLightColor) const -> color_pair;

    grid<tile> _tiles;
    u64        _seed;

    std::vector<std::shared_ptr<object>>  _objects;
    std::vector<std::shared_ptr<monster>> _monsters;

    std::vector<std::shared_ptr<light_source>> _lights;
};

}
