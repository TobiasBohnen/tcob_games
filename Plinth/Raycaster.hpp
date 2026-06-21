// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <vector>

#include "../_common/Common.hpp"
#include "Cache.hpp"

struct sprite {
    point_d  Pos;
    size_d   Size {size_d::One}; // world-unit width/height; {1,1} = one full tile
    i32      Texture {-1};
    degree_f Facing {0};
    bool     Solid {true};
};

using world_map_t = static_grid<u8, /*mapWidth*/ 24, /*mapHeight*/ 24>;

class raycaster {
public:
    raycaster(cache& cache, std::vector<sprite>& sprites, size_i screenSize, f64 horizontalFovDegrees);

    auto get_player_position() const -> point_d;
    void set_player_position(point_d pos);
    void set_world_map(world_map_t const& worldMap);

    auto move(f64 forwardAmount, f64 strafeAmount, f64 rotateAmount) -> bool;

    void prepare_draw();
    void draw_walls(u32* screenBuf, i32 columnStart, i32 columnEnd);
    void draw_sprites(u32* screenBuf, i32 columnStart, i32 columnEnd);

private:
    auto is_position_clear(point_d pos, f64 radius) const -> bool;

    cache&               _cache;
    std::vector<sprite>& _sprites;
    world_map_t const*   _worldMap {nullptr};
    size_i               _screenSize {};

    point_d _pos {};
    point_d _dir {};
    point_d _plane {};
    f64     _projPlaneDist {};

    std::vector<f64> _zBuffer;
    std::vector<f64> _rowDist;
    std::vector<f64> _spriteDepthBuffer;

    static constexpr f64 PlayerRadius {0.25};
};
