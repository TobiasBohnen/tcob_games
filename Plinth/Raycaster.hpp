// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

#include <span>
#include <utility>
#include <vector>

#include "Camera.hpp"
#include "Level.hpp"
#include "LevelObject.hpp"

class raycaster {
public:
    explicit raycaster(texture_cache& cache);

    auto draw(level& level, player const& player) -> std::span<u32>;

    void set_quality(quality_settings const& quality, f64 projPlaneDist);

private:
    struct frame {
        level&        Level;
        player const& Player;
        camera        Camera;
        i32           CenterY {};
    };

    struct column_hit;

    auto accumulate_light(frame const& frame, point_d const& surfacePos, f64 surfaceZ, point_i cell, u32 lightMask) const -> color_d;

    void draw_surface_pixel(frame const& frame, isize x, isize y, u8 const* tex, isize srcIdx, point_d const& surfacePos, f64 surfaceZ, f64 shade);

    void draw_columns(frame const& frame, i32 columnStart, i32 columnEnd);
    void draw_floor_ceiling_column(frame const& frame, column_hit& col);
    void draw_wall_column(frame const& frame, column_hit const& col);

    void draw_sprites(frame const& frame);
    void draw_voxel_objects(frame const& frame);
    void draw_motes(frame const& frame);

    std::vector<u32> _screen;

    std::vector<f64>                          _wallDepthBuffer;
    std::vector<f64>                          _objectDepthBuffer;
    std::vector<std::pair<f64, object_base*>> _voxelOrder;

    texture_cache&   _cache;
    quality_settings _quality;
    size_i           _screenSize;
    f64              _projPlaneDist {0};

    task_manager& _taskManager;

    procgen::value_noise _noise {{16, 16}, 0.05f};
};
