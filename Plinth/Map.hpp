// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <algorithm>
#include <optional>
#include <vector>

#include "Common.hpp"
#include "Walls.hpp"

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

constexpr auto HeightAt(slope const& slope, point_i cell, point_d const& pos) -> f64
{
    f64 const dx {std::clamp(pos.X - cell.X, 0.0, 1.0)};
    f64 const dy {std::clamp(pos.Y - cell.Y, 0.0, 1.0)};
    return (slope.TL * (1.0 - dx) * (1.0 - dy)) + (slope.TR * dx * (1.0 - dy)) + (slope.BL * (1.0 - dx) * dy) + (slope.BR * dx * dy);
}

enum class tile_visibility : u8 {
    Hidden,  // Never seen
    Visible, // Seen in the current frame / FOV
    Explored // Seen before, but currently out of view (memory)
};

class map {
public:
    map();

    void update(f64 dt);

    // whether a cell started or stopped blocking light since the last call: the lighting has to be rebuilt
    auto light_changed() -> bool;

    auto get_cell(point_i pos) const -> cell const&;
    void set_cell(point_i pos, cell const& c);
    auto toggle_wall(point_i pos) -> bool;

    auto surface_at(point_i pos) const -> cell_surface;
    auto slopes_at(point_i pos) const -> slopes;
    auto is_blocking_light(point_i pos) const -> bool;
    auto has_line_of_sight(point_d from, point_d to) const -> bool; // no blocking cell in between; the cells of both points are not tested
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
    bool                _lightChanged {false};
};
