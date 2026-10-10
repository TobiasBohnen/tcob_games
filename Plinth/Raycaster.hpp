// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <vector>

#include "Common.hpp"

#include "Level.hpp"
#include "Walls.hpp"

class raycaster {
public:
    explicit raycaster(texture_cache& cache);

    auto draw(level& level, player const& player) -> u32 const*;

    void set_quality(quality_settings const& quality, f64 projPlaneDist);

private:
    struct wall_extent {
        i32 Top;
        i32 Bottom;
        i32 ScreenCenterY;
    };

    void precompute_light_visibility(level const& level);
    auto accumulate_light(level const& level, player const& player, point_d const& surfacePos, f64 surfaceZ, point_i const& cell, u32 lightMask = LIGHT_LAYERS_ALL) const -> vec3_d;
    void shade_and_write(u32* screenBuf, isize x, isize y, u8 const* tex, isize srcIdx, level const& level, player const& player, point_d const& surfacePos, f64 surfaceZ) const;

    void draw_columns(level& level, player const& player, i32 screenCenterY, i32 columnStart, i32 columnEnd);
    void draw_wall_column(wall_hit const& hit, level const& level, player const& player, isize x, point_d rayDir, point_i cell, wall_extent const& extent);
    void draw_floor_ceiling_column(wall_hit const& hit, level const& level, player const& player, isize x, point_d rayDir, wall_extent const& extent);
    void draw_sprites(level const& level, player const& player, i32 screenCenterY);
    void draw_voxel_objects(level const& level, player const& player, i32 screenCenterY);

    void draw_screen_effect(level const& level, player const& player);
    void draw_weapon(player const& player);
    void draw_hud(player const& player);
    void draw_message(level const& level);

    std::vector<dynamic_light>    _dynamicLights;
    std::vector<std::vector<u32>> _cellLights;

    std::vector<u32> _screen;

    std::vector<f64> _zBuffer;
    std::vector<f64> _objectDepthBuffer;

    texture_cache&   _cache;
    quality_settings _quality;
    size_i           _screenSize;
    f64              _projPlaneDist {0};

    task_manager& _taskManager;
};
