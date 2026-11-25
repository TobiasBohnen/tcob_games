// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <optional>
#include <vector>

#include "Common.hpp"
#include "Walls.hpp"

struct cell_surface {
    i32 FloorTexture {INVALID_INDEX};
    i32 CeilingTexture {INVALID_INDEX};
    f64 FloorReflectivity {0.0};
    f64 FloorZ {0.0};
    f64 CeilingZ {1.0};
    u8  Motes {0};
    u8  Ground {0};
    u8  Level {0};
};

struct slope {
    f64 TL {0};
    f64 TR {0};
    f64 BL {0};
    f64 BR {0};
};
struct slopes {
    slope Ceiling;
    slope Floor;
};

enum class tile_visibility : u8 {
    Hidden,  // Never seen
    Visible, // Seen in the current frame / FOV
    Explored // Seen before, but currently out of view (memory)
};

class map {
public:
    void update(f64 dt);

    auto get_cell(point_i pos) const -> cell const&;
    void set_cell(point_i pos, cell const& c);
    auto toggle_wall(point_i pos) -> bool;

    auto surface_at(point_i pos) const -> cell_surface;
    auto slopes_at(point_i pos) const -> slopes;
    auto is_blocking_light(point_i pos) const -> bool;
    auto closest_point_on_wall(point_i cellPos, point_d pos) const -> std::optional<point_d>;

    auto get_visibility(point_i pos) const -> tile_visibility;
    void mark_visible(point_i pos);
    void reveal_map();

private:
    void prepare_slopes();

    map_grid                                            _cells;
    static_grid<tile_visibility, MAP_WIDTH, MAP_HEIGHT> _seen;

    std::vector<slopes> _cellSlopes;
    bool                _slopesDirty {false};
};
