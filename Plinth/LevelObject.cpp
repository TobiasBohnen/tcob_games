#include "LevelObject.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <optional>
#include <variant>

#include "Common.hpp"
#include "Voxel.hpp"

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

auto voxel_object::segment_hits(point_d from, point_d to, f64 z, f64 radius) const -> bool
{
    auto const t {make_transform()};

    point_d const s {to - from};
    f64 const     len {std::sqrt(s.length_squared())};
    point_d const perp {len > 1e-9 ? point_d {-s.Y / len, s.X / len} : point_d {1.0, 0.0}};

    auto const cast {[&](f64 side, f64 up) {
        f64 const ox {perp.X * side * radius};
        f64 const oy {perp.Y * side * radius};
        f64 const oz {up * radius};

        point3_d const a {t.to_local({.X = from.X + ox, .Y = from.Y + oy, .Z = z + oz})};
        point3_d const b {t.to_local({.X = to.X + ox, .Y = to.Y + oy, .Z = z + oz})};
        return Grid->raycast(a, b - a, 1.0).Hit;
    }};

    return cast(0, 0) || cast(1, 0) || cast(-1, 0) || cast(0, 1) || cast(0, -1);
}

auto voxel_object::filled_at(point_d pos, f64 z) const -> bool
{
    auto const l {make_transform().to_local({.X = pos.X, .Y = pos.Y, .Z = z})};
    return Grid->occupied(static_cast<i32>(std::floor(l.X)),
                          static_cast<i32>(std::floor(l.Y)),
                          static_cast<i32>(std::floor(l.Z)));
}

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
    , _flying {desc.Flying}
    , _mapMarker {desc.MapMarker}
    , _direction {desc.Direction}
    , _visual {desc.Visual}
    , _light {desc.Light}
{
    set_position(desc.Position);
}

auto level_object::id() const -> uid { return _id; }
auto level_object::radius() const -> f64 { return _radius; }
auto level_object::is_solid() const -> bool { return _radius > 0; }

auto level_object::map_marker() const -> color { return _mapMarker; }

auto level_object::dead() const -> bool { return _dead; }
void level_object::kill() { _dead = true; }

auto level_object::is_seen() const -> bool
{
    if (auto const* s {sprite()}) { return s->Seen; }
    if (auto const* v {voxel()}) { return v->Seen; }

    return false;
}

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

auto level_object::direction() const -> point_d
{
    return _direction;
}

void level_object::set_direction(point_d p)
{
    _direction = p;
}

auto level_object::flight_z() const -> std::optional<f64>
{
    if (!_flying) { return std::nullopt; }

    auto const* s {sprite()};
    if (!s) { return std::nullopt; }

    return s->BaseZ + (s->Size.Height * 0.5); // BaseZ is the bottom of the sprite
}

auto level_object::blocked() const -> bool
{
    return _blocked;
}

void level_object::set_blocked(bool b)
{
    _blocked = b;
}

auto level_object::overlaps(point_d from, point_d to, f64 radius, std::optional<f64> z) const -> bool
{
    if (!is_solid()) { return false; }

    // broad phase: closest point of the segment to our centre
    auto const c {position()};
    auto const s {to - from};
    f64 const  len2 {s.length_squared()};
    f64 const  t {len2 > 0.0 ? std::clamp((c - from).dot(s) / len2, 0.0, 1.0) : 0.0};
    f64 const  combined {radius + _radius};
    if ((c - (from + (s * t))).length_squared() >= combined * combined) { return false; }

    if (z) {
        if (auto const* v {voxel()}) { return v->segment_hits(from, to, *z, radius); }
    }
    return true; // sprites, and callers without a height
}

auto level_object::sprite() const -> sprite_object const* { return std::get_if<sprite_object>(&_visual); }
auto level_object::sprite() -> sprite_object* { return std::get_if<sprite_object>(&_visual); }
auto level_object::voxel() const -> voxel_object const* { return std::get_if<voxel_object>(&_visual); }
auto level_object::voxel() -> voxel_object* { return std::get_if<voxel_object>(&_visual); }

auto level_object::light() const -> dynamic_light const* { return _light ? &*_light : nullptr; }
auto level_object::light() -> dynamic_light* { return _light ? &*_light : nullptr; }
