// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"
#include "Walls.hpp"

using occupancy_grid = static_grid<bool, MAP_WIDTH, MAP_HEIGHT>;

struct map_prefab {
    std::vector<std::string_view> Rows; // ASCII art; parsed into cells at generation time
    i32                           WallTexture {0};
    i32                           FloorTexture {0};
    i32                           CeilingTexture {0};
    i32                           Weight {1};
};

struct map_gen_params {
    size_i GenArea {MAP_WIDTH, MAP_HEIGHT};
    i32    PrefabCount {8};
    i32    PlacementAttempts {30};
    i32    CorridorRadius {1};
    i32    DefaultWallTexture {1};
    u64    Seed {0};
};

class map_generator {
public:
    explicit map_generator(std::vector<map_prefab> prefabLibrary);

    auto generate(map_gen_params const& params) -> map_t;

private:
    struct placed_prefab {
        map_prefab const*    Prefab {};
        point_i              Origin;     // top-left in map space
        size_i               Size {};    // derived from Prefab->Rows at placement time
        std::vector<point_i> Connectors; // world-space, collected while stamping
    };

    struct nearest_pair {
        usize From {0};
        usize To {0};
        f64   Dist {std::numeric_limits<f64>::infinity()};
    };

    auto prefab_size(map_prefab const& prefab) -> size_i;
    auto stamp_prefab(map_t& map, map_prefab const& prefab, point_i origin) -> std::vector<point_i>;
    auto rect_free(point_i origin, size_i size) -> bool;
    auto try_place_prefab(map_prefab const& prefab, map_gen_params const& params, rng& rng) -> std::optional<point_i>;
    void add_border_walls(map_t& map, i32 wallTexture);
    auto pick_weighted_prefab(rng& rng, i32 totalWeight) -> map_prefab const*;

    auto find_corridor_path(point_i from, point_i to, occupancy_grid const& blocked) -> std::vector<point_i>;
    void carve_point(map_t& map, occupancy_grid const& blocked, i32 halfWidth, point_i p);
    void carve_corridor(map_t& map, map_gen_params const& params, point_i from, point_i to, occupancy_grid const& blocked);
    void connect_prefabs(map_t& map, map_gen_params const& params, std::vector<placed_prefab> const& placed, rng& rng);
    auto find_nearest_unlinked_pair(std::vector<placed_prefab> const& placed, std::vector<bool> const& linked) -> nearest_pair;
    auto pick_connector(placed_prefab const& p, rng& rng, occupancy_grid& connectorUsed) -> point_i;
    void fill_remaining_with_wall(map_t& map, map_gen_params const& params);

    std::vector<map_prefab> _library;
    occupancy_grid          _occupied;
};
