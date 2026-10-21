// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Common.hpp"

#include "Light.hpp"
#include "Voxel.hpp"

////////////////////////////////////////////////////////////

class sprite_object {
public:
    point_d Position {};
    f64     BaseZ {0.0};

    size_d   Size {size_d::One};
    i32      Texture {-1};
    degree_d Facing {0};
    u32      LightMask {LIGHT_LAYERS_ALL};

    auto facing_index(point_d cameraPos) const -> i32;

    auto screen_size(f64 scale) const -> size_i;

    auto screen_bottom(f64 scale, i32 screenCenterY, f64 eyeHeight) const -> i32;
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

    bool Seen {false};

    voxel_grid const* Grid {nullptr};

    auto make_transform() const -> voxel_transform;

    auto world_diagonal() const -> f64;

private:
    auto make_rotation() const -> mat3_d;
};

////////////////////////////////////////////////////////////

using object_visual = std::variant<std::monostate, sprite_object, voxel_object>;

struct level_object_desc {
    point_d Position {};
    f64     Radius {0.0};
    color   MapMarker {colors::Green};

    object_visual                Visual;
    std::optional<dynamic_light> Light {};
};

class level_object {
public:
    level_object(uid id, level_object_desc desc);

    auto id() const -> uid;
    auto radius() const -> f64;
    auto map_marker() const -> color;

    auto position() const -> point_d;
    void set_position(point_d p);

    auto dead() const -> bool;
    void kill();

    auto overlaps(point_d pos, f64 radius) const -> bool;

    auto sprite() const -> sprite_object const*;
    auto sprite() -> sprite_object*;

    auto voxel() const -> voxel_object const*;
    auto voxel() -> voxel_object*;

    auto light() const -> dynamic_light const*;
    auto light() -> dynamic_light*;

private:
    uid  _id;
    bool _dead {false};
    f64  _radius;

    color _mapMarker;

    object_visual                _visual;
    std::optional<dynamic_light> _light;
};
