// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"
#include "Voxel.hpp"
#include "Walls.hpp"

struct level_settings {
    i32 FloorTexture {0};
    i32 CeilingTexture {0};
};

class sprite_object {
public:
    point_d Position;
    f64     BaseZ {0.0};

    size_d   Size {size_d::One};
    i32      Texture {-1};
    degree_d Facing {0};
    bool     Solid {true};
    u32      LightMask {LIGHT_LAYERS_ALL};

    auto facing_index(point_d cameraPos) const -> i32
    {
        auto const viewAngle {Position.angle_to(cameraPos)};
        auto const relativeAngle {(Facing - viewAngle).as_normalized(angle_normalize::PositiveFullTurn)};

        constexpr f64 wedge {360.0 / NUM_FACINGS};
        return static_cast<i32>((relativeAngle.Value + (wedge / 2.0)) / wedge) % NUM_FACINGS;
    }

    auto screen_size(f64 scale) const -> size_i { return static_cast<size_i>(Size * std::abs(scale)); }

    auto screen_bottom(f64 scale, i32 screenCenterY, f64 eyeHeight) const -> i32 { return screenCenterY + static_cast<i32>((eyeHeight - BaseZ) * scale); }
};

struct dynamic_light {
    point_d Position {};
    f64     Z {0.5};
    f64     Range {8.0};
    color   Color {colors::White};
    f64     Intensity {1.0};
    u32     Layers {LIGHT_LAYER_WORLD};
};

class level {
public:
    explicit level(map_t map);

    std::vector<sprite_object> SpriteObjects;
    std::vector<voxel_object>  VoxelObjects;
    std::vector<dynamic_light> DynamicLights;

    level_settings Settings;

    void update(milliseconds deltaSeconds);

    auto get_cell(point_i p) const -> cell const&;
    auto is_clear(point_d pos, f64 radius) const -> bool;

    void toggle_wall(point_i p);

    auto is_seen(point_i cell) const -> bool;
    void mark_seen(point_i cell);
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
