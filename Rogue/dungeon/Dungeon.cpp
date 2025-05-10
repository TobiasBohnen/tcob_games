// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Dungeon.hpp"

#include "../actors/Monster.hpp"
#include "../actors/Player.hpp"
#include "../ui/Renderer.hpp"
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

auto dungeon::is_line_of_sight(point_i start, point_i end) const -> bool
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

void dungeon::remove_object(object const& object)
{
    helper::erase_first(_objects, [&object](auto const& val) { return val.get() == &object; });
    helper::erase_first(_tiles[object.Position].Objects, [&object](auto const& val) { return val.get() == &object; });
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

void dungeon::draw(renderer& renderer, point_i center, player const& player)
{
    auto&      tiles {_tiles};
    auto const range {player.light_range()};
    auto const color {player.light_color()};

    for (i32 y {0}; y < TERM_MAP_SIZE.Height; ++y) {
        for (i32 x {0}; x < TERM_MAP_SIZE.Width; ++x) {
            point_i const termPos {point_i {x, y}};
            point_i const gridPos {term_to_grid(termPos, center)};
            if (!tiles.contains(gridPos)) { continue; }

            auto& tile {tiles[gridPos]};
            auto [fg, bg] {lighting(tile, gridPos, player.position(), range, color)};
            if (gridPos == center) {
                renderer.set_color(colors::White, colors::Black);
                renderer.draw_cell(termPos, "+");
            } else if (tile.Seen || tile.InSight) {
                renderer.set_color(fg, bg);
                renderer.draw_cell(termPos, tile.Symbol);

                // draw objects
                auto const& objects {tile.Objects};
                for (auto const& object : objects) {
                    point_i const termPos {grid_to_term(gridPos, center)};
                    auto const    symbol {object->symbol()};
                    if (TERM_MAP_SIZE.contains(termPos) && !symbol.empty()) {
                        auto const colors {tile.InSight ? object->colors() : SEEN_COLORS};
                        renderer.set_color(colors.first, colors.second == colors::Transparent ? bg : colors.second);
                        renderer.draw_cell(termPos, symbol);
                    }
                }
            }
        }
    }
}

auto dungeon::lighting(tile& tile, point_i gridPos, point_i playerPos, f32 playerRange, color playerLightColor) const -> color_pair
{
    if (!is_line_of_sight(playerPos, gridPos)) {
        tile.InSight = false;
        return tile.Seen ? SEEN_COLORS
                         : std::pair {colors::Black, colors::Black};
    }

    f32 lightFgR {0.f}, lightFgG {0.f}, lightFgB {0.f};
    f32 lightBgR {0.f}, lightBgG {0.f}, lightBgB {0.f};
    f32 totalLightFactor {0.f};

    auto const accumulateLight {[&](color const& lightColor, f32 const factor) -> void {
        lightFgR += (lightColor.R / 255.f) * factor;
        lightFgG += (lightColor.G / 255.f) * factor;
        lightFgB += (lightColor.B / 255.f) * factor;

        lightBgR += (lightColor.R / 255.f) * factor;
        lightBgG += (lightColor.G / 255.f) * factor;
        lightBgB += (lightColor.B / 255.f) * factor;

        totalLightFactor += factor;
    }};

    auto const falloff {[](f64 range, f64 distance) -> f32 {
        if (distance == 0) { return 1.f; }
        if (range == 0) { return 0.f; }

        return static_cast<f32>(std::clamp((range * range) / (distance * distance) * ((range - distance) / range), 0., 1.));
    }};

    for (auto const& light : _lights) {
        f64 const distance {euclidean_distance(light->Position, gridPos)};
        f64 const range {light->Range};
        if (distance > range) { continue; }

        if (!is_line_of_sight(light->Position, gridPos)) { continue; }

        f32 const factor {light->Falloff ? falloff(range, distance) : 1.0f};
        accumulateLight(light->Color, factor * light->Intensity);
    }

    f64 const distance {euclidean_distance(playerPos, gridPos)};
    if (distance <= playerRange) {
        f32 const factor {falloff(playerRange, distance)};
        accumulateLight(playerLightColor, factor);
    }

    if (totalLightFactor > 0.f) {
        tile.Seen    = true;
        tile.InSight = true;

        totalLightFactor = std::max(1.0f, totalLightFactor);
        auto const  apply {[&](u8 c, f32 light) -> u8 { return std::min(255.f, c * (light / totalLightFactor)); }};
        color const fgColor {apply(tile.ForegroundColor.R, lightFgR), apply(tile.ForegroundColor.G, lightFgG), apply(tile.ForegroundColor.B, lightFgB)};
        color const bgColor {apply(tile.BackgroundColor.R, lightBgR), apply(tile.BackgroundColor.G, lightBgG), apply(tile.BackgroundColor.B, lightBgB)};
        return color_pair {fgColor, bgColor};
    }

    tile.InSight = false;
    return std::pair {color::Lerp(colors::Black, colors::White, 0.15f), colors::Black};
}

}
