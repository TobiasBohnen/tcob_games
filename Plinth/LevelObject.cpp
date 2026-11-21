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

    f64 const r {radius / Scale};
    f64 const r2 {r * r};

    auto const a {t.to_local(point3_d::Lift(from, z))};
    auto const b {t.to_local(point3_d::Lift(to, z))};
    auto const d {b - a};

    i32 const steps {std::max(1, static_cast<i32>(std::ceil(std::sqrt(d.dot(d)) / 0.5)))};

    auto const axis_gap {[](f64 c, i32 v) {
        f64 const g {std::clamp(c, static_cast<f64>(v), v + 1.0) - c};
        return g * g;
    }};

    for (i32 i {0}; i <= steps; ++i) {
        auto const c {a + (d * (static_cast<f64>(i) / steps))};

        i32 const minX {static_cast<i32>(std::floor(c.X - r))}, maxX {static_cast<i32>(std::floor(c.X + r))};
        i32 const minY {static_cast<i32>(std::floor(c.Y - r))}, maxY {static_cast<i32>(std::floor(c.Y + r))};
        i32 const minZ {static_cast<i32>(std::floor(c.Z - r))}, maxZ {static_cast<i32>(std::floor(c.Z + r))};

        for (i32 vz {minZ}; vz <= maxZ; ++vz) {
            for (i32 vy {minY}; vy <= maxY; ++vy) {
                for (i32 vx {minX}; vx <= maxX; ++vx) {
                    if (!Grid->occupied(vx, vy, vz)) { continue; }

                    if (axis_gap(c.X, vx) + axis_gap(c.Y, vy) + axis_gap(c.Z, vz) < r2) { return true; }
                }
            }
        }
    }
    return false;
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

auto level_object::flying() const -> bool { return _flying; }

auto level_object::center_z() const -> f64
{
    if (auto const* s {sprite()}) { return s->BaseZ + (s->Size.Height * 0.5); }
    if (auto const* v {voxel()}) { return v->BaseZ + (v->Grid->Size.Z * v->Scale * 0.5); }
    if (_light) { return _light->Z; }

    return 0.0;
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

    auto const c {position()};
    auto const s {to - from};
    f64 const  len2 {s.length_squared()};
    f64 const  t {len2 > 0.0 ? std::clamp((c - from).dot(s) / len2, 0.0, 1.0) : 0.0};
    f64 const  combined {radius + _radius};
    if ((c - (from + (s * t))).length_squared() >= combined * combined) { return false; }

    if (z) {
        if (auto const* v {voxel()}) { return v->segment_hits(from, to, *z, radius); }
    }
    return true;
}

auto level_object::sprite() const -> sprite_object const* { return std::get_if<sprite_object>(&_visual); }
auto level_object::sprite() -> sprite_object* { return std::get_if<sprite_object>(&_visual); }
auto level_object::voxel() const -> voxel_object const* { return std::get_if<voxel_object>(&_visual); }
auto level_object::voxel() -> voxel_object* { return std::get_if<voxel_object>(&_visual); }

auto level_object::light() const -> dynamic_light const* { return _light ? &*_light : nullptr; }
auto level_object::light() -> dynamic_light* { return _light ? &*_light : nullptr; }
