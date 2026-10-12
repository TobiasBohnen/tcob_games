// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "LevelObject.hpp"

auto sprite_object::facing_index(point_d cameraPos) const -> i32
{
    auto const viewAngle {Position.angle_to(cameraPos)};
    auto const relativeAngle {(Facing - viewAngle).as_normalized(angle_normalize::PositiveFullTurn)};

    constexpr f64 wedge {360.0 / NUM_FACINGS};
    return static_cast<i32>((relativeAngle.Value + (wedge / 2.0)) / wedge) % NUM_FACINGS;
}

auto sprite_object::screen_size(f64 scale) const -> size_i { return static_cast<size_i>(Size * std::abs(scale)); }

auto sprite_object::screen_bottom(f64 scale, i32 screenCenterY, f64 eyeHeight) const -> i32 { return screenCenterY + static_cast<i32>((eyeHeight - BaseZ) * scale); }

auto voxel_object::make_transform() const -> voxel_transform { return {.Rotation = make_rotation(), .Position = Position, .Pivot = Grid->pivot(), .BaseZ = BaseZ, .Scale = Scale}; }

auto voxel_object::world_diagonal() const -> f64 { return Grid->diagonal() * Scale; }

auto voxel_object::make_rotation() const -> mat3_d
{
    f64 const cy {Yaw.cos()}, sy {Yaw.sin()};
    f64 const cp {Pitch.cos()}, sp {Pitch.sin()};
    f64 const cr {Roll.cos()}, sr {Roll.sin()};
    return {cy * cp, (cy * sp * sr) - (sy * cr), (cy * sp * cr) + (sy * sr),
            sy * cp, (sy * sp * sr) + (cy * cr), (sy * sp * cr) - (cy * sr),
            -sp, cp * sr, cp * cr};
}

level_object::level_object(uid id, level_object_desc desc)
    : _id {id}
    , _radius {desc.Radius}
    , _mapMarker {desc.MapMarker}
    , _visual {desc.Visual}
    , _light {desc.Light}
{
    set_position(desc.Position);
}

auto level_object::id() const -> uid { return _id; }
auto level_object::radius() const -> f64 { return _radius; }
auto level_object::map_marker() const -> color { return _mapMarker; }

auto level_object::dead() const -> bool { return _dead; }
void level_object::kill() { _dead = true; }

auto level_object::position() const -> point_d
{
    if (auto const* s {sprite()}) { return s->Position; }
    if (auto const* v {voxel()}) { return v->Position; }
    if (_light) { return _light->Position; };

    return {};
}

void level_object::set_position(point_d p)
{
    if (auto* s {sprite()}) { s->Position = p; }
    if (auto* v {voxel()}) { v->Position = p; }
    if (_light) { _light->Position = p; }
}

auto level_object::overlaps(point_d pos, f64 radius) const -> bool
{
    if (_radius <= 0.0) { return false; } // radius 0 = not solid
    f64 const     combined {radius + _radius};
    point_d const d {position() - pos};
    return d.dot(d) < combined * combined;
}

auto level_object::sprite() const -> sprite_object const* { return std::get_if<sprite_object>(&_visual); }
auto level_object::sprite() -> sprite_object* { return std::get_if<sprite_object>(&_visual); }
auto level_object::voxel() const -> voxel_object const* { return std::get_if<voxel_object>(&_visual); }
auto level_object::voxel() -> voxel_object* { return std::get_if<voxel_object>(&_visual); }

auto level_object::light() const -> dynamic_light const* { return _light ? &*_light : nullptr; }
auto level_object::light() -> dynamic_light* { return _light ? &*_light : nullptr; }
