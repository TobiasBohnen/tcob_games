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
    mat3_d  Rotation {};
    point_d Position {};
    vec3_d  Pivot {};
    f64     BaseZ {0.0};
    f64     Scale {1.0};

    auto rotate(vec3_d v) const -> vec3_d;

    auto rotate_inverse(vec3_d v) const -> vec3_d;

    auto to_world(vec3_d local) const -> vec3_d;

    auto to_local(vec3_d world) const -> vec3_d;

    auto dir_to_local(vec3_d worldDir) const -> vec3_d;
};

////////////////////////////////////////////////////////////

class voxel_grid {
public:
    explicit voxel_grid(std::vector<voxel> const& voxels);

    vec3_i Size {};

    auto face_ao(voxel_ray_hit const& hit, vec3_d const& localHitPos) const -> f64; // 0..1, 1 = unoccluded

    auto corner_ao(vec3_i layer, i32 faceAxis, i32 cu, i32 cv) const -> i32;

    auto raycast(vec3_d const& origin, vec3_d const& dir, f64 maxT) const -> voxel_ray_hit;

    auto corners() const -> std::array<vec3_d, 8>;
    auto pivot() const -> vec3_d;
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

class voxel_object {
public:
    point_d Position {};
    f64     Scale {1.0};

    f64 BaseZ {0.0};

    radian_d Yaw {0.0};
    radian_d Pitch {0.0};
    radian_d Roll {0.0};

    voxel_grid const* Grid {nullptr};

    auto make_transform() const -> voxel_transform;

    auto world_diagonal() const -> f64;

private:
    auto make_rotation() const -> mat3_d;
};