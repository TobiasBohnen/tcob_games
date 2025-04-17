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

class level {
public:
    explicit level(master_control& parent);

    auto is_line_of_sight(point_i start, point_i end) -> bool;
    auto is_passable(point_i pos) const -> bool;
    auto find_path(point_i start, point_i target) const -> std::vector<point_i>;

    auto get_tiles() -> grid<tile>&;

    void add_light(std::shared_ptr<light_source> const& light);
    void remove_light(light_source const& light);
    auto get_lights() -> std::vector<std::shared_ptr<light_source>> const&;

private:
    grid<tile> _tiles;

    std::vector<std::shared_ptr<object>>  _objects;
    std::vector<std::shared_ptr<monster>> _monsters;

    std::vector<std::shared_ptr<light_source>> _lights;

    master_control& _parent;
};

}
