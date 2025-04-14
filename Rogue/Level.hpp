// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

class level {
public:
    explicit level(master_control& parent);

    auto is_passable(point_i pos) const -> bool;

    auto find_path(point_i start, point_i target) const -> std::vector<point_i>;

    auto get_tiles() -> grid<tile>&;

private:
    grid<tile> _tiles;

    std::vector<std::shared_ptr<object>>  _objects;
    std::vector<std::shared_ptr<monster>> _monsters;

    master_control& _parent;
};

}
