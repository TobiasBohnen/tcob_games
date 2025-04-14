// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Level.hpp"

#include "Layout.hpp"
#include "MasterControl.hpp"

namespace Rogue {

////////////////////////////////////////////////////////////

level::level(master_control& parent)
    : _parent {parent}
{
    u64 seed {static_cast<u64>(clock::now().time_since_epoch().count())};
    seed   = 12345;
    // _tiles = tunneling {20, 5, 12}.generate(seed, {120, 120});
    // _tiles = drunkards_walk {.4f, 25000, 0.15f, 0.7f}.generate(seed, {120, 120});
    _tiles = bsp_tree {6, 15}.generate(seed, {120, 120});
    // _tiles = cellular_automata {30000, 4, 0.5f, 16, 500, true, 1}.generate(seed, {120, 120});
    // _tiles = city_walls {8, 16}.generate(seed, {120, 120});
    // _tiles = maze_with_rooms {6, 13, 100, 0.04f, 0.1f, false}.generate(seed, {121, 121});
    // _tiles = messy_bsp_tree {6, 15, true, 1, 3}.generate(seed, {120, 120});

    gfx::l_system lsystem {seed};
    lsystem.add_rule('X', {"[RPRPRPR]Y", 1.0f}); // Create room and path

    // Rules for expanding the path and changing directions
    lsystem.add_rule('Y', {"[>RPW*R]PY", 0.50f}); // Expand path right, add room, and path
    lsystem.add_rule('Y', {"[>PRPPR]PY", 0.25f}); // Expand path right with smaller rooms
    lsystem.add_rule('Y', {"[<PRPPR]PY", 0.25f}); // Expand path left with smaller rooms

    //"[RPRPRPR][>RPR]P[>PRPPR]P[>PRPPR]"2

    //  _tiles = turtle {turtle::pen {.Position = {5, 8}, .Direction = direction::Right, .RoomSize = {3, 5}, .PathLength = 5},
    //                 lsystem.generate("X", 5)}
    //             .generate(seed, {120, 120});
}

auto level::is_passable(point_i pos) const -> bool
{
    if (!_tiles.contains(pos)) { return false; }
    return tile_traits::passable(_tiles[pos]);
}

auto level::find_path(point_i start, point_i target) const -> std::vector<point_i>
{
    ai::astar_pathfinding path {true, ai::astar_pathfinding::heuristic::Chebyshev};

    struct grid_path {
        grid<tile> const* Parent;

        auto get_cost(point_i p) const -> u64
        {
            auto const& tile {(*Parent)[p]};
            if (!tile_traits::passable(tile) || !tile.Seen) { return ai::astar_pathfinding::IMPASSABLE_COST; }
            return 1;
        }
    } pathfinding {&_tiles};

    return path.find_path(pathfinding, _tiles.size(), start, target);
}

auto level::get_tiles() -> grid<tile>&
{
    return _tiles;
}

}
