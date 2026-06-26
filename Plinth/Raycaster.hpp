// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <vector>

#include "Common.hpp"
#include "Walls.hpp"

class raycaster {
public:
    raycaster(cache& cache, size_i screenSize, f64 projPlaneDist);

    void prepare_draw();
    void draw(level const& level, player const& player, u32* screenBuf, i32 columnStart, i32 columnEnd);

private:
    void draw_walls(level const& level, player const& player, u32* screenBuf, i32 columnStart, i32 columnEnd);

    void draw_wall_column(wall_hit const& hit, f64 invFogDistance, level const& level, player const& player, u32* screenBuf, isize x, bool transparent);
    void draw_floor_ceiling_column(wall_hit const& hit, f64 invFogDistance, level const& level, player const& player, u32* screenBuf, isize x, point_d rayDir);

    void draw_sprites(level const& level, player const& player, u32* screenBuf, i32 columnStart, i32 columnEnd);

    cache& _cache;
    size_i _screenSize {};

    std::vector<f64> _zBuffer;
    std::vector<f64> _rowDist;
    std::vector<f64> _spriteDepthBuffer;
};
