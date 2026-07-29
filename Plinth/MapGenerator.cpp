// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "MapGenerator.hpp"

#include "Common.hpp"

map_generator::map_generator(std::vector<map_prefab> prefabLibrary)
    : _library {std::move(prefabLibrary)}
{
}

auto map_generator::generate(map_gen_params const& params) -> map_t
{
    auto const stamp_prefab {[this](map_t& map, map_prefab const& prefab, point_i origin) {
        for (i32 y {0}; y < prefab.Size.Height; ++y) {
            for (i32 x {0}; x < prefab.Size.Width; ++x) {
                point_i const world {origin.X + x, origin.Y + y};
                map[world]       = prefab.Cells[x + (y * prefab.Size.Width)];
                _occupied[world] = true;
            }
        }
    }};
    auto const try_place_prefab {[this](map_prefab const& prefab, map_gen_params const& params, rng& rng) -> std::optional<point_i> {
        auto const rect_free {[this](point_i origin, size_i size) -> bool {
            // pad by 1 cell on each side so prefabs never touch directly, leaving room for corridor carving
            for (i32 y {origin.Y - 1}; y <= origin.Y + size.Height; ++y) {
                for (i32 x {origin.X - 1}; x <= origin.X + size.Width; ++x) {
                    point_i const cell {x, y};
                    if (!map_t::Size.contains(cell)) { return false; }
                    if (_occupied[cell]) { return false; }
                }
            }
            return true;
        }};

        i32 const maxX {params.GenArea.Width - prefab.Size.Width - 1};
        i32 const maxY {params.GenArea.Height - prefab.Size.Height - 1};
        if (maxX < 1 || maxY < 1) { return std::nullopt; }

        for (i32 attempt {0}; attempt < params.PlacementAttempts; ++attempt) {
            point_i const origin {rng(1, maxX), rng(1, maxY)};
            if (rect_free(origin, prefab.Size)) { return origin; }
        }
        return std::nullopt;
    }};

    map_t map {};
    std::ranges::fill(_occupied, false);

    rng rng {params.Seed == 0 ? clock::now().time_since_epoch().count() : params.Seed};

    std::vector<placed_prefab> placed;

    i32 totalWeight {0};
    for (auto const& p : _library) { totalWeight += p.Weight; }

    for (i32 i {0}; i < params.PrefabCount && !_library.empty() && totalWeight > 0; ++i) {
        i32               roll {rng(0, totalWeight - 1)};
        map_prefab const* prefab {nullptr};
        for (auto const& p : _library) {
            roll -= p.Weight;
            if (roll < 0) {
                prefab = &p;
                break;
            }
        }

        if (auto origin {try_place_prefab(*prefab, params, rng)}) {
            stamp_prefab(map, *prefab, *origin);
            placed.push_back({.Prefab = prefab, .Origin = *origin});
        }
    }

    connect_prefabs(map, params, placed, rng);
    fill_remaining_with_wall(map, params);

    return map;
}

auto map_generator::find_corridor_path(point_i from, point_i to, static_grid<bool, MAP_WIDTH, MAP_HEIGHT> const& blocked) -> std::vector<point_i>
{
    static_grid<point_i, MAP_WIDTH, MAP_HEIGHT> cameFrom {};
    static_grid<bool, MAP_WIDTH, MAP_HEIGHT>    visited {};

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

void map_generator::carve_corridor(map_t& map, map_gen_params const& params, point_i from, point_i to, static_grid<bool, MAP_WIDTH, MAP_HEIGHT> const& blocked)
{
    i32 const halfWidth {params.CorridorRadius};

    auto const carve_point {[&](point_i p) {
        for (i32 dy {-halfWidth}; dy <= halfWidth; ++dy) {
            for (i32 dx {-halfWidth}; dx <= halfWidth; ++dx) {
                point_i const cellPos {p.X + dx, p.Y + dy};
                if (!map_t::Size.contains(cellPos)) { continue; }
                if (blocked[cellPos]) { continue; }

                map[cellPos]       = floor_cell {};
                _occupied[cellPos] = true;
            }
        }
    }};

    std::vector<point_i> const path {find_corridor_path(from, to, blocked)};
    if (path.empty()) {
        // No route around obstacles (extremely tight layout) — fall back to the old straight-line
        // carve so the rooms end up connected even if it cuts through something.
        point_i   cursor {from};
        i32 const stepX {to.X > from.X ? 1 : (to.X < from.X ? -1 : 0)};
        while (cursor.X != to.X) {
            carve_point(cursor);
            cursor.X += stepX;
        }
        i32 const stepY {to.Y > from.Y ? 1 : (to.Y < from.Y ? -1 : 0)};
        while (cursor.Y != to.Y) {
            carve_point(cursor);
            cursor.Y += stepY;
        }
        carve_point(to);
        return;
    }

    for (point_i const p : path) { carve_point(p); }
}

void map_generator::connect_prefabs(map_t& map, map_gen_params const& params, std::vector<placed_prefab> const& placed, rng& rng)
{
    if (placed.size() < 2) { return; }

    // Snapshot which cells belong to placed prefabs *before* any carving starts, so corridors
    // are only ever blocked by rooms, never by earlier corridor segments overlapping later ones.
    auto const prefabOccupied {_occupied};

    // Connect via minimum-spanning-tree over room centers so every room is guaranteed
    // reachable, rather than a fully random graph that could leave a room stranded.
    std::vector<bool> linked(placed.size(), false);
    linked[0] = true;

    for (usize linkedCount {1}; linkedCount < placed.size(); ++linkedCount) {
        f64   bestDist {std::numeric_limits<f64>::infinity()};
        usize bestFrom {0};
        usize bestTo {0};

        for (usize i {0}; i < placed.size(); ++i) {
            if (!linked[i]) { continue; }
            for (usize j {0}; j < placed.size(); ++j) {
                if (linked[j]) { continue; }

                point_i const centerI {placed[i].Origin.X + (placed[i].Prefab->Size.Width / 2),
                                       placed[i].Origin.Y + (placed[i].Prefab->Size.Height / 2)};
                point_i const centerJ {placed[j].Origin.X + (placed[j].Prefab->Size.Width / 2),
                                       placed[j].Origin.Y + (placed[j].Prefab->Size.Height / 2)};

                f64 const dx {static_cast<f64>(centerI.X - centerJ.X)};
                f64 const dy {static_cast<f64>(centerI.Y - centerJ.Y)};
                f64 const dist {(dx * dx) + (dy * dy)};

                if (dist < bestDist) {
                    bestDist = dist;
                    bestFrom = i;
                    bestTo   = j;
                }
            }
        }

        auto const pick_connector {[&](placed_prefab const& p) {
            if (p.Prefab->Connectors.empty()) {
                return point_i {p.Origin.X + (p.Prefab->Size.Width / 2), p.Origin.Y + (p.Prefab->Size.Height / 2)};
            }
            point_i const local {p.Prefab->Connectors[rng(usize {0}, p.Prefab->Connectors.size() - 1)]};
            return point_i {p.Origin.X + local.X, p.Origin.Y + local.Y};
        }};

        carve_corridor(map, params, pick_connector(placed[bestFrom]), pick_connector(placed[bestTo]), prefabOccupied);
        linked[bestTo] = true;
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
