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
    struct wall_extent {
        i32 Top;
        i32 Bottom;
        i32 ScreenCenterY;
    };

    void precompute_light_visibility(level const& level);
    auto is_light_visible(point_i cell, usize lightIndex) const -> bool;
    auto accumulate_light(level const& level, player const& player, point_d const& surfacePos, f64 surfaceZ, point_i const& cell) const -> vec3_d;

    void draw_columns(level& level, player const& player, i32 columnStart, i32 columnEnd);
    void draw_wall_column(wall_hit const& hit, level const& level, player const& player, isize x, point_d rayDir, point_i cell, wall_extent const& extent);
    void draw_floor_ceiling_column(wall_hit const& hit, level const& level, player const& player, isize x, point_d rayDir, wall_extent const& extent);
    void draw_sprites(level const& level, player const& player);
    void draw_voxel_objects(level const& level, player const& player);

    void draw_screen_effect(level const& level, player const& player);
    void draw_weapon(player const& player);
    void draw_hud(player const& player);
    void draw_message(level const& level);

    std::vector<u32> _screen;

    std::vector<f64> _zBuffer;
    std::vector<f64> _objectDepthBuffer;

    texture_cache& _cache;
    size_i         _screenSize;
    f64            _projPlaneDist;

    std::vector<u8>               _lightVisibility;
    usize                         _numDynamicLights {0};
    std::vector<std::vector<u32>> _cellLights;

    task_manager& _taskManager;
};
