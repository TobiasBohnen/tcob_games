// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

#include <concepts>
#include <optional>
#include <variant>

#include "Lighting.hpp"
#include "Voxel.hpp"

////////////////////////////////////////////////////////////

class sprite_object {
public:
    size_d   Size {size_d::One};
    i32      Texture {-1};
    degree_d Facing {0};

    u32 LightMask {LIGHT_LAYERS_ALL};

    auto facing_index(point_d objPos, point_d cameraPos) const -> i32;
    auto screen_size(f64 scale) const -> size_i;
    // eyeHeight is the height of the eye above the bottom of the sprite
    auto screen_bottom(f64 scale, i32 screenCenterY, f64 eyeHeight) const -> i32;
};

////////////////////////////////////////////////////////////

class voxel_object {
public:
    f64 Scale {1.0};

    radian_d Yaw {0.0};
    radian_d Pitch {0.0};
    radian_d Roll {0.0};

    u32 LightMask {LIGHT_LAYERS_ALL};

    std::optional<screen_bounds> ScreenBounds {};

    voxel_grid const* Grid {nullptr};

    // worldPos: x/y of the object and the absolute height of the bottom of the grid
    auto make_transform(point3_d worldPos) const -> voxel_transform;
    auto world_diagonal() const -> f64;
    auto segment_hits(point3_d worldPos, point_d from, point_d to, f64 z, f64 radius) const -> bool;
    auto filled_at(point3_d worldPos, point_d pos, f64 z) const -> bool;

private:
    auto make_rotation() const -> mat3_d;
};

using object_visual = std::variant<std::monostate, sprite_object, voxel_object>;

////////////////////////////////////////////////////////////

struct object_desc {
    string        Name {};
    point3_d      Position {}; // Z: height above the floor, or absolute height for kinds that fly
    color         MapMarker {};
    object_visual Visual {};

    std::optional<milliseconds> LifeTime {};
};

////////////////////////////////////////////////////////////

class object_base {
public:
    auto id() const -> uid;
    auto name() const -> string const&;

    auto position() const -> point3_d; // x/y and Position.Z (relative or absolute, see the kind)
    void set_position(point3_d p);
    void set_xy(point_d p);

    auto floor_z() const -> f64;  // reported by the renderer each frame: the floor height the object stands on, 0 for flying kinds
    void set_floor_z(f64 floorZ);
    auto world_z() const -> f64;  // actual absolute height of the object's base
    auto world_position() const -> point3_d;
    auto center_z() const -> f64; // actual absolute height of the middle of the visual

    auto map_marker() const -> color;

    auto is_seen() const -> bool;
    void set_seen(bool b);

    auto dead() const -> bool;
    void kill();

    auto life_time() const -> std::optional<milliseconds>; // what is left, empty if it does not expire
    void age(milliseconds dt);                             // counts the life time down and kills the object when it runs out

    auto sprite() const -> sprite_object const*;
    auto sprite() -> sprite_object*;
    auto voxel() const -> voxel_object const*;
    auto voxel() -> voxel_object*;

protected:
    object_base(uid id, object_desc const& desc);

private:
    uid           _id;
    string        _name;
    point3_d      _position;
    f64           _floorZ {0.0};
    object_visual _visual;
    color         _mapMarker;
    bool          _dead {false};
    bool          _seen {false};

    std::optional<milliseconds> _lifeTime;
};

////////////////////////////////////////////////////////////
// collides with walls and other objects
class solid {
public:
    f64 Radius {0.0}; // 0: not solid
};

// moves by itself
class movable {
public:
    point_d Direction {};
    bool    Blocked {false}; // set when it ran into something, so the collision is reported once
};

// carries a light; the light's Position is an offset from the owner's world position (see world_light)
class emitter {
public:
    std::optional<dynamic_light> Light;
};

////////////////////////////////////////////////////////////
// kinds

class enemy final : public object_base, public solid, public movable {
public:
    enemy(uid id, object_desc const& desc, f64 radius, bool flying, i32 health, point_d direction = {});

    i32 Health {1};

    auto flying() const -> bool { return _flying; }

private:
    bool _flying;
};

class projectile final : public object_base, public solid, public movable, public emitter {
public:
    projectile(uid id, object_desc const& desc, f64 radius, point_d direction, uid owner, std::optional<dynamic_light> light = std::nullopt);

    uid Owner {};

    static constexpr auto flying() -> bool { return true; }
};

class pickup final : public object_base, public solid, public emitter {
public:
    pickup(uid id, object_desc const& desc, f64 radius, i32 item, std::optional<dynamic_light> light = std::nullopt);

    i32 Item {0};

    static constexpr auto flying() -> bool { return false; }
};

// a plain thing in the world: prop, decoration, torch
class object final : public object_base, public solid, public emitter {
public:
    object(uid id, object_desc const& desc, f64 radius, std::optional<dynamic_light> light = std::nullopt);

    static constexpr auto flying() -> bool { return false; }
};

////////////////////////////////////////////////////////////

using level_object = std::variant<enemy, pickup, object, projectile>;

auto base_of(level_object& o) -> object_base&;
auto base_of(level_object const& o) -> object_base const&;

auto is_flying(level_object const& o) -> bool;

// the object's light placed in the world: Position is the owner's world position plus the light's offset; empty for kinds without a light
auto world_light(level_object const& o) -> std::optional<dynamic_light>;

// whether the move from -> to of a circle with `radius` runs into the object; voxel objects are tested against their voxels at height z
auto overlaps(level_object const& o, point_d from, point_d to, f64 radius, std::optional<f64> z) -> bool;
