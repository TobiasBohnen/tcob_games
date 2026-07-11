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
    map_t map {};
    std::ranges::fill(_occupied, false);

    rng rng {params.Seed == 0 ? clock::now().time_since_epoch().count() : params.Seed};

    std::vector<placed_prefab> placed;

    for (i32 i {0}; i < params.PrefabCount && !_library.empty(); ++i) {
        map_prefab const& prefab {_library[rng(usize {0}, _library.size() - 1)]};
        if (auto origin {try_place_prefab(prefab, params, rng)}) {
            stamp_prefab(map, prefab, *origin);
            placed.push_back({.Prefab = &prefab, .Origin = *origin});
        }
    }

    connect_prefabs(map, params, placed, rng);
    fill_remaining_with_wall(map, params);

    return map;
}

auto map_generator::try_place_prefab(map_prefab const& prefab, map_gen_params const& params, rng& rng) -> std::optional<point_i>
{
    i32 const maxX {params.GenArea.Width - prefab.Size.Width - 1};
    i32 const maxY {params.GenArea.Height - prefab.Size.Height - 1};
    if (maxX < 1 || maxY < 1) { return std::nullopt; }

    for (i32 attempt {0}; attempt < params.PlacementAttempts; ++attempt) {
        point_i const origin {rng(1, maxX), rng(1, maxY)};
        if (rect_free(origin, prefab.Size)) { return origin; }
    }
    return std::nullopt;
}

auto map_generator::rect_free(point_i origin, size_i size) const -> bool
{
    // pad by 1 cell on each side so prefabs never touch directly, leaving room for corridor carving
    for (i32 y {origin.Y - 1}; y <= origin.Y + size.Height; ++y) {
        for (i32 x {origin.X - 1}; x <= origin.X + size.Width; ++x) {
            point_i const cell {x, y};
            if (!map_t::Size.contains(cell)) { return false; }
            if (_occupied[cell]) { return false; }
        }
    }
    return true;
}

void map_generator::stamp_prefab(map_t& map, map_prefab const& prefab, point_i origin)
{
    for (i32 y {0}; y < prefab.Size.Height; ++y) {
        for (i32 x {0}; x < prefab.Size.Width; ++x) {
            point_i const world {origin.X + x, origin.Y + y};
            map[world]       = prefab.Cells[x + (y * prefab.Size.Width)];
            _occupied[world] = true;
        }
    }
}

void map_generator::carve_corridor(map_t& map, map_gen_params const& params, point_i from, point_i to)
{
    i32 const halfWidth {params.CorridorWidth / 2};

    auto const carve_point {[&](point_i p) {
        for (i32 dy {-halfWidth}; dy <= halfWidth; ++dy) {
            for (i32 dx {-halfWidth}; dx <= halfWidth; ++dx) {
                point_i const cellPos {p.X + dx, p.Y + dy};
                if (!map_t::Size.contains(cellPos)) { continue; }

                map[cellPos]       = floor_cell {};
                _occupied[cellPos] = true;
            }
        }
    }};

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
}

void map_generator::connect_prefabs(map_t& map, map_gen_params const& params, std::vector<placed_prefab> const& placed, rng& rng)
{
    if (placed.size() < 2) { return; }

    // Connect via minimum-spanning-tree over room centers so every room is guaranteed
    // reachable, rather than a fully random graph that could leave a room stranded.
    std::vector<bool> linked(placed.size(), false);
    linked[0] = true;

    for (size_t linkedCount {1}; linkedCount < placed.size(); ++linkedCount) {
        f64    bestDist {std::numeric_limits<f64>::infinity()};
        size_t bestFrom {0};
        size_t bestTo {0};

        for (size_t i {0}; i < placed.size(); ++i) {
            if (!linked[i]) { continue; }
            for (size_t j {0}; j < placed.size(); ++j) {
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

        carve_corridor(map, params, pick_connector(placed[bestFrom]), pick_connector(placed[bestTo]));
        linked[bestTo] = true;
    }
}

void map_generator::fill_remaining_with_wall(map_t& map, map_gen_params const& params)
{
    normal_wall const defaultWall {.Texture = params.DefaultWallTexture};

    for (i32 y {0}; y < MAP_HEIGHT; ++y) {
        for (i32 x {0}; x < MAP_WIDTH; ++x) {
            point_i const cellPos {x, y};
            if (!_occupied[cellPos]) {
                map[cellPos] = defaultWall;
            }
        }
    }
}
