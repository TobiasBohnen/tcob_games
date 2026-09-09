// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#pragma once

#include <optional>
#include <vector>

#include "Common.hpp"

////////////////////////////////////////////////////////////

struct voxel {
    vec3_i Position {};
    color  Color {};
};

struct voxel_ray_hit {
    bool   Hit {false};
    f64    T {0.0};
    color  Color {0, 0, 0};
    i32    FaceAxis {0};
    i32    FaceSign {1};
    vec3_i Cell {};
};

class voxel_grid {
public:
    explicit voxel_grid(std::vector<voxel> const& voxels);

    vec3_i Size {};

    auto occupied(i32 x, i32 y, i32 z) const -> bool;
    auto color_at(i32 x, i32 y, i32 z) const -> color;

    auto corner_ao(vec3_i layer, i32 faceAxis, i32 cu, i32 cv) const -> i32;

    auto raycast(vec3_d const& origin, vec3_d const& dir, f64 maxT) const -> voxel_ray_hit;

    static auto Load(string const& path) -> std::optional<voxel_grid>;

private:
    auto flat_index(i32 x, i32 y, i32 z) const -> isize;

    std::vector<color> _cells;
    std::vector<bool>  _occupied;
};
