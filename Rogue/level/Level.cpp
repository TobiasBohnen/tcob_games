// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Level.hpp"

#include "../MasterControl.hpp"
#include "Layout.hpp"

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

    auto light0 {std::make_shared<light_source>()};
    light0->Color     = colors::Red;
    light0->Intensity = 1.f;
    light0->Position  = point_i {15, 2};
    light0->Range     = 5;
    add_light(light0);

    auto light1 {std::make_shared<light_source>()};
    light1->Color    = colors::Blue;
    light1->Position = point_i {18, 5};
    light1->Range    = 5;
    light1->Falloff  = false;
    add_light(light1);
}

auto level::is_line_of_sight(point_i start, point_i end) -> bool
{
    auto [x0, y0] {start};
    auto const [x1, y1] {end};
    i32 const dx {std::abs(x1 - x0)};
    i32 const dy {std::abs(y1 - y0)};
    i32 const sx {(x0 < x1) ? 1 : -1};
    i32 const sy {(y0 < y1) ? 1 : -1};
    i32       err {dx - dy};

    for (;;) {
        if (x0 == x1 && y0 == y1) { break; }

        i32 const e2 {2 * err};
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }

        point_i const pos {x0, y0};
        if (pos != end && !tile_traits::passable(_tiles[pos])) { return false; }
    }
    return true;
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

void level::add_light(std::shared_ptr<light_source> const& light)
{
    _lights.push_back(light);
}

void level::remove_light(light_source const& light)
{
    helper::erase_first(_lights, [light](auto const& val) { return &light == val.get(); });
}

auto level::get_lights() -> std::vector<std::shared_ptr<light_source>> const&
{
    return _lights;
}

}
