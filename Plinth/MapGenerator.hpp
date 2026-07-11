// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"
#include "Walls.hpp"

struct map_prefab {
    size_i               Size {};
    std::vector<cell>    Cells;      // Size.Width * Size.Height entries, row-major (local coords)
    std::vector<point_i> Connectors; // local-space edge cells where a corridor may attach
};

struct map_gen_params {
    size_i GenArea {MAP_WIDTH, MAP_HEIGHT}; // area within the fixed map_t grid to actively fill; rest becomes solid wall
    i32    PrefabCount {8};                 // how many prefabs to attempt to place
    i32    PlacementAttempts {30};          // per-prefab random-position retries before giving up
    i32    CorridorWidth {1};
    i32    DefaultWallTexture {1};
    u64    Seed {0};
};

class map_generator {
public:
    explicit map_generator(std::vector<map_prefab> prefabLibrary);

    auto generate(map_gen_params const& params) -> map_t;

private:
    struct placed_prefab {
        map_prefab const* Prefab {};
        point_i           Origin; // top-left in map space
    };

    auto try_place_prefab(map_prefab const& prefab, map_gen_params const& params, rng& rng) -> std::optional<point_i>;
    auto rect_free(point_i origin, size_i size) const -> bool;
    void stamp_prefab(map_t& map, map_prefab const& prefab, point_i origin);
    void carve_corridor(map_t& map, map_gen_params const& params, point_i from, point_i to);
    void connect_prefabs(map_t& map, map_gen_params const& params, std::vector<placed_prefab> const& placed, rng& rng);
    void fill_remaining_with_wall(map_t& map, map_gen_params const& params);

    std::vector<map_prefab>                  _library;
    static_grid<bool, MAP_WIDTH, MAP_HEIGHT> _occupied; // cells already claimed by a prefab or corridor
};
