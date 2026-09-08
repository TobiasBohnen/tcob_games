// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"
#include "Voxel.hpp"
#include "Walls.hpp"

struct voxel_lighting {
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
    f64 SunDirection {0};
};

struct level_settings {
    f64 FogMin {0.0};
    f64 FogDistance {12.0};
    f64 AmbientLight {1.0};

    i32  FloorTexture {0};
    i32  CeilingTexture {0};
    bool IsSkybox {false};

    voxel_lighting VoxelLighting;
};

struct sprite {
    point_d  Position;
    size_d   Size {size_d::One};
    i32      Texture {-1};
    degree_f Facing {0};
    bool     Solid {true};
};

struct voxel_object {
    point_d           Position {};
    f64               BaseZ {0.0};
    degree_d          Yaw {0.0};
    f64               Scale {1.0};
    voxel_grid const* Grid {nullptr};
};

class level {
public:
    explicit level(map_t map);

    std::vector<sprite>       Sprites;
    std::vector<voxel_object> VoxelObjects;

    level_settings Settings;

    void update(milliseconds deltaSeconds);

    auto get_cell(point_i p) const -> cell const&;

    void toggle_wall(point_i p);

    auto is_seen(point_i cell) const -> bool;
    void mark_seen(point_i cell, point_d playerPos);
    void mark_all_seen();

    void show_message(string const& msg);
    auto get_message() const -> string const&;

private:
    map_t _map;

    static_grid<bool, MAP_WIDTH, MAP_HEIGHT> _seen;

    milliseconds _messageTimer {};
    string       _message;
};
