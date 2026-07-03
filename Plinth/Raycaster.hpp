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
    raycaster(texture_cache& cache, size_i screenSize, f64 projPlaneDist);

    auto draw(level& level, player const& player) -> u32 const*;

private:
    void draw_columns(level& level, player const& player, f64 invFogDistance, i32 columnStart, i32 columnEnd);

    void draw_wall_column(wall_hit const& hit, level const& level, player const& player, isize x, f64 invFogDistance, bool transparent);
    void draw_floor_ceiling_column(wall_hit const& hit, level const& level, player const& player, isize x, point_d rayDir, f64 invFogDistance);

    void draw_sprites(level const& level, player const& player, f64 invFogDistance, i32 columnStart, i32 columnEnd);

    void draw_weapon(player const& player);
    void draw_hud(player const& player);

    std::vector<u32> _screen;

    std::vector<f64> _zBuffer;
    std::vector<f64> _spriteDepthBuffer;

    texture_cache& _cache;
    size_i         _screenSize;
    f64            _projPlaneDist;
};
