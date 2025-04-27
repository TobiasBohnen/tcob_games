// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Dungeon.hpp"

#include "../actors/Monster.hpp"
#include "Layout.hpp"
#include "Object.hpp"

namespace Rogue {

////////////////////////////////////////////////////////////

dungeon::dungeon()
    : _seed {12345}
{
    // u64 seed {static_cast<u64>(clock::now().time_since_epoch().count())};

    // _tiles = tunneling {20, 5, 12}.generate(seed, {120, 120});
    // _tiles = drunkards_walk {.4f, 25000, 0.15f, 0.7f}.generate(_seed, {120, 120});
    _tiles = bsp_tree {6, 15}.generate(_seed, {120, 120});
    // _tiles = cellular_automata {30000, 4, 0.5f, 16, 500, true, 1}.generate(seed, {120, 120});
    // _tiles = city_walls {8, 16}.generate(seed, {120, 120});
    // _tiles = maze_with_rooms {6, 13, 100, 0.04f, 0.1f, false}.generate(seed, {121, 121});
    // _tiles = messy_bsp_tree {6, 15, true, 1, 3}.generate(seed, {120, 120});

    gfx::l_system lsystem {_seed};
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
    _lights.push_back(light0);

    auto light1 {std::make_shared<light_source>()};
    light1->Color    = colors::Blue;
    light1->Position = point_i {18, 5};
    light1->Range    = 5;
    light1->Falloff  = false;
    _lights.push_back(light1);

    _tiles[{18, 1}] = WALL0;
    _tiles[{18, 2}] = GLASS;
    _tiles[{18, 3}] = WALL0;
}

auto dungeon::is_line_of_sight(point_i start, point_i end) -> bool
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
        if (pos != end && !_tiles[pos].is_transparent()) { return false; }
    }
    return true;
}

auto dungeon::is_passable(point_i pos) const -> bool
{
    if (!_tiles.contains(pos)) { return false; }
    return _tiles[pos].is_passable();
}

auto dungeon::find_path(point_i start, point_i target) const -> std::vector<point_i>
{
    ai::astar_pathfinding path {true, ai::astar_pathfinding::heuristic::Chebyshev};

    struct grid_path {
        grid<tile> const* Parent;

        auto get_cost(point_i from, point_i to) const -> u64
        {
            auto const& tile {(*Parent)[to]};
            if (!tile.is_passable() || !tile.Seen) { return ai::astar_pathfinding::IMPASSABLE_COST; }
            bool const diagonal {(from.X != to.X) && (from.Y != to.Y)};
            return diagonal ? 14 : 10;
        }
    } pathfinding {&_tiles};

    return path.find_path(pathfinding, _tiles.size(), start, target);
}

void dungeon::add_object(std::shared_ptr<object> const& object)
{
    _objects.push_back(object);
    _tiles[object->Position].Objects.push_back(object);
}

void dungeon::remove_object(std::shared_ptr<object> const& object)
{
    helper::erase_first(_objects, [object](auto const& val) { return val == object; });
    helper::erase_first(_tiles[object->Position].Objects, [object](auto const& val) { return val == object; });
}

auto dungeon::tiles() -> grid<tile>&
{
    return _tiles;
}

auto dungeon::objects() const -> std::span<std::shared_ptr<object> const>
{
    return _objects;
}

auto dungeon::monsters() const -> std::span<std::shared_ptr<monster> const>
{
    return _monsters;
}

auto dungeon::lights() const -> std::span<std::shared_ptr<light_source> const>
{
    return _lights;
}

}
