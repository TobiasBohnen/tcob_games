// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#pragma once

#include <optional>
#include <vector>

#include "Common.hpp"

struct vec3_d {
    f64 X {0.0};
    f64 Y {0.0};
    f64 Z {0.0};

    auto dot(vec3_d const& b) const -> f64;
    auto normalized() const -> vec3_d;

    friend auto operator+(vec3_d const& a, vec3_d const& b) -> vec3_d { return {a.X + b.X, a.Y + b.Y, a.Z + b.Z}; }
    friend auto operator-(vec3_d const& a, vec3_d const& b) -> vec3_d { return {a.X - b.X, a.Y - b.Y, a.Z - b.Z}; }
    friend auto operator*(vec3_d const& a, f64 s) -> vec3_d { return {a.X * s, a.Y * s, a.Z * s}; }
};

struct vec3_i {
    i32 X {0};
    i32 Y {0};
    i32 Z {0};
};

////////////////////////////////////////////////////////////

struct voxel {
    vec3_i Position {};
    color  Color {};
};

struct bake_lighting {
    f64 KeyAzimuthOffsetDeg {-35.0};
    f64 KeyElevationDeg {35.0};
    f64 KeyDiffuse {0.30};
    f64 FillAzimuthOffsetDeg {150.0};
    f64 FillElevationDeg {5.0};
    f64 FillDiffuse {0.10};
    f64 AmbientSky {0.75};
    f64 AmbientGround {0.40};
    f64 AoStrength {0.25};
    f64 HeightBandingStrength {0.30};
};

class voxel_grid {
public:
    explicit voxel_grid(std::vector<voxel> const& voxels);

    vec3_i Size {};

    auto occupied(i32 x, i32 y, i32 z) const -> bool;
    auto color_at(i32 x, i32 y, i32 z) const -> color; // caller must have already checked occupied()

    auto bake_facings(i32 frameSize, f64 frontFacingDegrees, i32 numFacings, bake_lighting const& lighting = {}) const -> std::vector<u8>;

private:
    auto flat_index(i32 x, i32 y, i32 z) const -> isize;

    std::vector<color> _cells;
    std::vector<bool>  _occupied; // separate from _cells -- avoids needing a sentinel "empty" color value
};

                                  ////////////////////////////////////////////////////////////

auto load_vox_file(string const& path) -> std::optional<voxel_grid>;
