// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

#include <span>
#include <vector>

#include "Level.hpp"

class raycaster {
public:
    explicit raycaster(texture_cache& cache);

    auto draw(level& level, player& player) -> std::span<u32>;

    void set_quality(quality_settings const& quality, f64 projPlaneDist);

private:
    struct column_hit;

    auto accumulate_light(point_d const& surfacePos, f64 surfaceZ, point_i cell, u32 lightMask) const -> color_d;

    void draw_surface_pixel(isize x, isize y, u8 const* tex, isize srcIdx, point_d const& surfacePos, f64 surfaceZ, f64 shade);

    void draw_columns(i32 screenCenterY, i32 columnStart, i32 columnEnd);
    void draw_floor_ceiling_column(column_hit& col);
    void draw_wall_column(column_hit const& col);

    void draw_sprites(i32 screenCenterY);
    void draw_voxel_objects(i32 screenCenterY);
    void draw_motes(i32 screenCenterY);

    auto sky_pixel(i32 y, point_d const& rayDir) const -> u32;

    std::vector<u32> _screen;

    std::vector<f64> _zBuffer;
    std::vector<f64> _objectDepthBuffer;

    texture_cache&   _cache;
    quality_settings _quality;
    size_i           _screenSize;
    f64              _projPlaneDist {0};

    task_manager& _taskManager;
    level*        _level {nullptr};
    player const* _player {nullptr};

    procgen::value_noise _noise {{16, 16}, 0.05f};
};
