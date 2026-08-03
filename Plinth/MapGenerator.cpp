// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "MapGenerator.hpp"

#include "Common.hpp"

struct parsed_cell {
    cell Cell;
    bool IsConnector {false};
};

// ASCII legend:
//   '#'  normal_wall (uses prefab.WallTexture)
//   '1'-'9'  normal_wall with that literal texture index
//   '.'  floor_cell (open interior)
//   'D'  door_wall, orientation from which edge it sits on; becomes a connector
//        only if it sits on the prefab's boundary
//   'S'  push_wall (secret door), PushDirection from which edge it sits on
//   'B'  box_wall, LocalBounds defaults to the full cell {0,0,1,1}
//   '\'  diagonal_wall NorthWestToSouthEast
//   '/'  diagonal_wall SouthWestToNorthEast
//   'P'  round_pillar
//   'o'  connector (open floor cell) — boundary only
//   any other character -> floor_cell
//
// Converts a single ASCII symbol at local position (x, y) within a width x height prefab into
// the cell it represents. Adding a new symbol only means adding a case here.
static auto parse_ascii_cell(char symbol, i32 x, i32 y, i32 width, i32 height, map_prefab const& prefab) -> parsed_cell
{
    bool const onEdge {x == 0 || y == 0 || x == width - 1 || y == height - 1};

    auto const make_floor {[&] {
        floor_cell f {};
        f.FloorTexture   = prefab.FloorTexture;
        f.CeilingTexture = prefab.CeilingTexture;
        return f;
    }};

    switch (symbol) {
    case '#': {
        normal_wall w {};
        w.Texture = prefab.WallTexture;
        return {.Cell = w, .IsConnector = false};
    }
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': {
        normal_wall w {};
        w.Texture = symbol - '0';
        return {.Cell = w, .IsConnector = false};
    }
    case 'D': {
        door_wall d {};
        d.Texture        = prefab.WallTexture;
        d.FrameTexture   = prefab.WallTexture;
        d.FloorTexture   = prefab.FloorTexture;
        d.CeilingTexture = prefab.CeilingTexture;
        d.Orientation    = (y == 0 || y == height - 1) ? door_wall::orientation::BlocksNorthSouth
                                                       : door_wall::orientation::BlocksEastWest;
        return {.Cell = d, .IsConnector = onEdge}; // (a) doors only act as connectors on the boundary
    }
    case 'S': {
        push_wall s {};
        s.Texture        = prefab.WallTexture;
        s.FloorTexture   = prefab.FloorTexture;
        s.CeilingTexture = prefab.CeilingTexture;
        if (y == 0) {
            s.PushDirection = {0, -1};
        } else if (y == height - 1) {
            s.PushDirection = {0, 1};
        } else if (x == 0) {
            s.PushDirection = {-1, 0};
        } else if (x == width - 1) {
            s.PushDirection = {1, 0};
        }
        return {.Cell = s, .IsConnector = false};
    }
    case 'B': {
        box_wall b {};
        b.LocalBounds    = {0.0, 0.0, 1.0, 1.0};
        b.Texture        = prefab.WallTexture;
        b.FloorTexture   = prefab.FloorTexture;
        b.CeilingTexture = prefab.CeilingTexture;
        return {.Cell = b, .IsConnector = false};
    }
    case '\\': {
        diagonal_wall dg {};
        dg.Orientation    = diagonal_wall::orientation::NorthWestToSouthEast;
        dg.Texture        = prefab.WallTexture;
        dg.FloorTexture   = prefab.FloorTexture;
        dg.CeilingTexture = prefab.CeilingTexture;
        return {.Cell = dg, .IsConnector = false};
    }
    case '/': {
        diagonal_wall dg {};
        dg.Orientation    = diagonal_wall::orientation::SouthWestToNorthEast;
        dg.Texture        = prefab.WallTexture;
        dg.FloorTexture   = prefab.FloorTexture;
        dg.CeilingTexture = prefab.CeilingTexture;
        return {.Cell = dg, .IsConnector = false};
    }
    case 'o':
        return {make_floor(), onEdge}; // (a) same boundary-only rule as doors
    case 'P': {
        round_pillar p {};
        p.Radius         = 0.3;
        p.Texture        = prefab.WallTexture;
        p.FloorTexture   = prefab.FloorTexture;
        p.CeilingTexture = prefab.CeilingTexture;
        return {.Cell = p, .IsConnector = false};
    }
    case '.':
    default:
        return {.Cell = make_floor(), .IsConnector = false};
    }
}

map_generator::map_generator(std::vector<map_prefab> prefabLibrary)
    : _library {std::move(prefabLibrary)}
{
}

auto map_generator::generate(map_gen_params const& params) -> map_t
{
    map_t map {};
    _occupied.fill(false);

    add_border_walls(map, params.DefaultWallTexture);

    rng rng {params.Seed == 0 ? clock::now().time_since_epoch().count() : params.Seed};

    std::vector<placed_prefab> placed;

    i32 totalWeight {0};
    for (auto const& p : _library) { totalWeight += p.Weight; }

    for (i32 i {0}; i < params.PrefabCount && !_library.empty() && totalWeight > 0; ++i) {
        map_prefab const* prefab {pick_weighted_prefab(rng, totalWeight)};
        if (!prefab) { continue; }

        if (auto origin {try_place_prefab(*prefab, params, rng)}) {
            std::vector<point_i> connectors {stamp_prefab(map, *prefab, *origin)};
            placed.push_back({.Prefab = prefab, .Origin = *origin, .Size = prefab_size(*prefab), .Connectors = std::move(connectors)});
        }
    }

    connect_prefabs(map, params, placed, rng);
    fill_remaining_with_wall(map, params);

    return map;
}

auto map_generator::prefab_size(map_prefab const& prefab) -> size_i
{
    i32 const h {static_cast<i32>(prefab.Rows.size())};
    i32 const w {h > 0 ? static_cast<i32>(prefab.Rows[0].size()) : 0};
    return {w, h};
}

auto map_generator::stamp_prefab(map_t& map, map_prefab const& prefab, point_i origin) -> std::vector<point_i>
{
    std::vector<point_i> connectors;

    i32 const height {static_cast<i32>(prefab.Rows.size())};
    i32 const width {height > 0 ? static_cast<i32>(prefab.Rows[0].size()) : 0};

    for (i32 y {0}; y < height; ++y) {
        for (i32 x {0}; x < width; ++x) {
            point_i const world {origin.X + x, origin.Y + y};
            auto const [cellValue, isConnector] {parse_ascii_cell(prefab.Rows[y][x], x, y, width, height, prefab)};

            map[world]       = cellValue;
            _occupied[world] = true;
            if (isConnector) { connectors.push_back(world); }
        }
    }

    return connectors;
}

auto map_generator::try_place_prefab(map_prefab const& prefab, map_gen_params const& params, rng& rng) -> std::optional<point_i>
{
    size_i const size {prefab_size(prefab)};
    i32 const    maxX {params.GenArea.Width - size.Width - 1};
    i32 const    maxY {params.GenArea.Height - size.Height - 1};
    if (maxX < 1 || maxY < 1) { return std::nullopt; }

    for (i32 attempt {0}; attempt < params.PlacementAttempts; ++attempt) {
        point_i const origin {rng(1, maxX), rng(1, maxY)};

        // pad by 1 cell on each side so prefabs never touch directly, leaving room for corridor carving
        bool free {true};
        for (i32 y {origin.Y - 1}; free && y <= origin.Y + size.Height; ++y) {
            for (i32 x {origin.X - 1}; x <= origin.X + size.Width; ++x) {
                point_i const cell {x, y};
                if (!map_t::Size.contains(cell) || _occupied[cell]) {
                    free = false;
                    break;
                }
            }
        }

        if (free) { return origin; }
    }
    return std::nullopt;
}

void map_generator::add_border_walls(map_t& map, i32 wallTexture)
{
    normal_wall borderWall {};
    borderWall.Texture = wallTexture;
    for (i32 x {0}; x < MAP_WIDTH; ++x) {
        map[{x, 0}]                    = borderWall;
        map[{x, MAP_HEIGHT - 1}]       = borderWall;
        _occupied[{x, 0}]              = true;
        _occupied[{x, MAP_HEIGHT - 1}] = true;
    }
    for (i32 y {0}; y < MAP_HEIGHT; ++y) {
        map[{0, y}]                   = borderWall;
        map[{MAP_WIDTH - 1, y}]       = borderWall;
        _occupied[{0, y}]             = true;
        _occupied[{MAP_WIDTH - 1, y}] = true;
    }
}

auto map_generator::pick_weighted_prefab(rng& rng, i32 totalWeight) -> map_prefab const*
{
    i32 roll {rng(0, totalWeight - 1)};
    for (auto const& p : _library) {
        roll -= p.Weight;
        if (roll < 0) { return &p; }
    }
    return nullptr;
}

auto map_generator::find_corridor_path(point_i from, point_i to, occupancy_grid const& blocked) -> std::vector<point_i>
{
    static_grid<point_i, MAP_WIDTH, MAP_HEIGHT> cameFrom {};
    occupancy_grid                              visited {};

    std::queue<point_i> frontier;
    frontier.push(from);
    visited[from] = true;

    constexpr std::array<point_i, 4> dirs {{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};

    bool found {false};
    while (!frontier.empty()) {
        point_i const cur {frontier.front()};
        frontier.pop();

        if (cur == to) {
            found = true;
            break;
        }

        for (auto const& d : dirs) {
            point_i const next {cur.X + d.X, cur.Y + d.Y};
            if (!map_t::Size.contains(next)) { continue; }
            if (visited[next]) { continue; }
            if (blocked[next] && next != to) { continue; } // allow stepping onto the destination even if it's "blocked" (it's the target room's own connector)

            visited[next]  = true;
            cameFrom[next] = cur;
            frontier.push(next);
        }
    }

    std::vector<point_i> path;
    if (!found) { return path; } // no route around obstacles; caller decides fallback

    for (point_i p {to}; p != from; p = cameFrom[p]) { path.push_back(p); }
    path.push_back(from);
    std::ranges::reverse(path);
    return path;
}

void map_generator::carve_point(map_t& map, occupancy_grid const& blocked, i32 halfWidth, point_i p)
{
    for (i32 dy {-halfWidth}; dy <= halfWidth; ++dy) {
        for (i32 dx {-halfWidth}; dx <= halfWidth; ++dx) {
            point_i const cellPos {p.X + dx, p.Y + dy};
            if (!map_t::Size.contains(cellPos)) { continue; }
            if (blocked[cellPos]) { continue; }

            map[cellPos]       = floor_cell {};
            _occupied[cellPos] = true;
        }
    }
}

void map_generator::carve_corridor(map_t& map, map_gen_params const& params, point_i from, point_i to, occupancy_grid const& blocked)
{
    i32 const halfWidth {params.CorridorRadius};

    std::vector<point_i> const path {find_corridor_path(from, to, blocked)};
    if (path.empty()) {
        // No route around obstacles (extremely tight layout) — fall back to a straight-line carve
        // so the rooms end up connected even if it cuts through something.
        point_i   cursor {from};
        i32 const stepX {to.X > from.X ? 1 : (to.X < from.X ? -1 : 0)};
        i32 const stepY {to.Y > from.Y ? 1 : (to.Y < from.Y ? -1 : 0)};
        while (cursor.X != to.X || cursor.Y != to.Y) {
            carve_point(map, blocked, halfWidth, cursor);
            if (cursor.X != to.X) { cursor.X += stepX; }
            if (cursor.Y != to.Y) { cursor.Y += stepY; }
        }
        carve_point(map, blocked, halfWidth, to);
        return;
    }

    for (point_i const p : path) { carve_point(map, blocked, halfWidth, p); }
}

auto map_generator::find_nearest_unlinked_pair(std::vector<placed_prefab> const& placed, std::vector<bool> const& linked) -> nearest_pair
{
    nearest_pair best;

    for (usize i {0}; i < placed.size(); ++i) {
        if (!linked[i]) { continue; }
        for (usize j {0}; j < placed.size(); ++j) {
            if (linked[j]) { continue; }

            point_i const centerI {placed[i].Origin.X + (placed[i].Size.Width / 2),
                                   placed[i].Origin.Y + (placed[i].Size.Height / 2)};
            point_i const centerJ {placed[j].Origin.X + (placed[j].Size.Width / 2),
                                   placed[j].Origin.Y + (placed[j].Size.Height / 2)};

            f64 const dx {static_cast<f64>(centerI.X - centerJ.X)};
            f64 const dy {static_cast<f64>(centerI.Y - centerJ.Y)};
            f64 const dist {(dx * dx) + (dy * dy)};

            if (dist < best.Dist) { best = {i, j, dist}; }
        }
    }
    return best;
}

auto map_generator::pick_connector(placed_prefab const& p, rng& rng, occupancy_grid& connectorUsed) -> point_i
{
    point_i const world {p.Connectors.empty()
                             ? point_i {p.Origin.X + (p.Size.Width / 2), p.Origin.Y + (p.Size.Height / 2)}
                             : p.Connectors[rng(usize {0}, p.Connectors.size() - 1)]};
    connectorUsed[world] = true;
    return world;
}

void map_generator::connect_prefabs(map_t& map, map_gen_params const& params, std::vector<placed_prefab> const& placed, rng& rng)
{
    if (placed.size() < 2) { return; }

    // Snapshot which cells belong to placed prefabs *before* any carving starts, so corridors
    // are only ever blocked by rooms, never by earlier corridor segments overlapping later ones.
    auto const prefabOccupied {_occupied};

    // Tracks which connector cells (world space) actually got used as a corridor endpoint, so
    // unused ones can be sealed afterward.
    occupancy_grid connectorUsed {};
    connectorUsed.fill(false);

    // Connect via minimum-spanning-tree over room centers so every room is guaranteed
    // reachable, rather than a fully random graph that could leave a room stranded.
    std::vector<bool> linked(placed.size(), false);
    linked[0] = true;

    for (usize linkedCount {1}; linkedCount < placed.size(); ++linkedCount) {
        auto const [bestFrom, bestTo, bestDist] {find_nearest_unlinked_pair(placed, linked)};

        carve_corridor(map, params, pick_connector(placed[bestFrom], rng, connectorUsed), pick_connector(placed[bestTo], rng, connectorUsed), prefabOccupied);
        linked[bestTo] = true;
    }

    // Seal every connector that was never used as a corridor endpoint, so unused doors/openings
    // don't lead nowhere.
    normal_wall sealWall {};
    sealWall.Texture = params.DefaultWallTexture;

    for (auto const& p : placed) {
        for (point_i const world : p.Connectors) {
            if (!connectorUsed[world]) {
                map[world] = sealWall;
            }
        }
    }
}

void map_generator::fill_remaining_with_wall(map_t& map, map_gen_params const& params)
{
    normal_wall defaultWall {};
    defaultWall.Texture = params.DefaultWallTexture;

    for (i32 y {0}; y < MAP_HEIGHT; ++y) {
        for (i32 x {0}; x < MAP_WIDTH; ++x) {
            point_i const cellPos {x, y};
            if (!_occupied[cellPos]) {
                map[cellPos] = defaultWall;
            }
        }
    }
}
