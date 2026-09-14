// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"
#include "Voxel.hpp"
#include "Walls.hpp"

struct level_settings {
    f64 FogMin {0.0};
    f64 FogDistance {12.0};

    f64 AmbientLight {1.0};

    i32  FloorTexture {0};
    i32  CeilingTexture {0};
    bool IsSkybox {false};
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
    auto is_clear(point_d pos, f64 radius) const -> bool;

    void toggle_wall(point_i p);

    auto is_seen(point_i cell) const -> bool;
    void mark_seen(point_i cell, point_d playerPos);
    void mark_all_seen();

    void show_message(string const& msg);
    auto get_message() const -> string const&;

private:
    auto closest_point_on_wall(point_i map, point_d pos) const -> std::optional<point_d>;

    map_t _map;

    static_grid<bool, MAP_WIDTH, MAP_HEIGHT> _seen;

    milliseconds _messageTimer {};
    string       _message;
};
