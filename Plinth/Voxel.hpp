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

////////////////////////////////////////////////////////////

class voxel_transform {
public:
    mat3_d   Rotation {};
    point_d  Position {};
    point3_d Pivot {};
    f64      BaseZ {0.0};
    f64      Scale {1.0};

    auto rotate(point3_d v) const -> point3_d;

    auto rotate_inverse(point3_d v) const -> point3_d;

    auto to_world(point3_d local) const -> point3_d;

    auto to_local(point3_d world) const -> point3_d;

    auto dir_to_local(point3_d worldDir) const -> point3_d;
};

////////////////////////////////////////////////////////////

class voxel_grid {
public:
    explicit voxel_grid(std::vector<voxel> const& voxels);

    vec3_i Size {};

    auto face_ao(voxel_ray_hit const& hit, point3_d const& localHitPos) const -> f64; // 0..1, 1 = unoccluded

    auto corner_ao(vec3_i layer, i32 faceAxis, i32 cu, i32 cv) const -> i32;

    auto raycast(point3_d const& origin, point3_d const& dir, f64 maxT) const -> voxel_ray_hit;

    auto corners() const -> std::array<point3_d, 8>;
    auto pivot() const -> point3_d;
    auto diagonal() const -> f64;

    static auto Load(string const& path) -> std::optional<voxel_grid>;

private:
    auto color_at(i32 x, i32 y, i32 z) const -> color;
    auto occupied(i32 x, i32 y, i32 z) const -> bool;
    auto flat_index(i32 x, i32 y, i32 z) const -> isize;

    std::vector<color> _cells;
    std::vector<bool>  _occupied;
};

////////////////////////////////////////////////////////////
