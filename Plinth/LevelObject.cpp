// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "LevelObject.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <optional>
#include <type_traits>
#include <variant>

#include "Common.hpp"
#include "Voxel.hpp"

////////////////////////////////////////////////////////////

auto sprite_object::facing_index(point_d objPos, point_d cameraPos) const -> i32
{
    auto const viewAngle {objPos.angle_to(cameraPos)};
    auto const relativeAngle {(Facing - viewAngle).as_normalized(angle_normalize::PositiveFullTurn)};

    constexpr f64 wedge {360.0 / NUM_FACINGS};
    return static_cast<i32>((relativeAngle.Value + (wedge / 2.0)) / wedge) % NUM_FACINGS;
}

auto sprite_object::screen_size(f64 scale) const -> size_i { return static_cast<size_i>(Size * std::abs(scale)); }

auto sprite_object::screen_bottom(f64 scale, i32 screenCenterY, f64 eyeHeight) const -> i32 { return screenCenterY + static_cast<i32>(eyeHeight * scale); }

////////////////////////////////////////////////////////////

static auto EulerRotation(radian_d yaw, radian_d pitch, radian_d roll) -> mat3_d
{
    f64 const cy {yaw.cos()}, sy {yaw.sin()};
    f64 const cp {pitch.cos()}, sp {pitch.sin()};
    f64 const cr {roll.cos()}, sr {roll.sin()};
    return {cy * cp, (cy * sp * sr) - (sy * cr), (cy * sp * cr) + (sy * sr),
            sy * cp, (sy * sp * sr) + (cy * cr), (sy * sp * cr) - (cy * sr),
            -sp, cp * sr, cp * cr};
}

static auto Multiply(mat3_d const& a, mat3_d const& b) -> mat3_d
{
    mat3_d r {};
    for (usize row {0}; row < 3; ++row) {
        for (usize col {0}; col < 3; ++col) {
            r[(row * 3) + col] = (a[row * 3] * b[col]) + (a[(row * 3) + 1] * b[3 + col]) + (a[(row * 3) + 2] * b[6 + col]);
        }
    }
    return r;
}

static auto Rotate(mat3_d const& m, point3_d v) -> point3_d
{
    return {.X = (m[0] * v.X) + (m[1] * v.Y) + (m[2] * v.Z),
            .Y = (m[3] * v.X) + (m[4] * v.Y) + (m[5] * v.Z),
            .Z = (m[6] * v.X) + (m[7] * v.Y) + (m[8] * v.Z)};
}

// A part turns around its own pivot (voxel_transform turns around Pivot), then the whole object turns around the
// object's Pivot: the part's own orientation is applied first, the object's second.
auto voxel_object::make_transform(point3_d worldPos, voxel_part const& part) const -> voxel_transform
{
    auto const objectRot {EulerRotation(Yaw, Pitch, Roll)};
    auto const rot {Multiply(objectRot, EulerRotation(part.Yaw, part.Pitch, part.Roll))};

    f64 const      partScale {Scale * part.Scale};
    point3_d const gridCenter {part.Grid->pivot()};
    point3_d const pivot {part.Pivot.value_or(gridCenter)};

    // where the part's joint sits in the object's space
    point3_d const joint {.X = part.Offset.X + ((pivot.X - gridCenter.X) * part.Scale),
                          .Y = part.Offset.Y + ((pivot.Y - gridCenter.Y) * part.Scale),
                          .Z = part.Offset.Z + (pivot.Z * part.Scale)};

    auto const world {worldPos + ((Pivot + Rotate(objectRot, joint - Pivot)) * Scale)};

    return {.Rotation = rot,
            .Position = world.xy(),
            .Pivot    = pivot,
            .BaseZ    = world.Z - (pivot.Z * partScale),
            .Scale    = partScale};
}

auto voxel_object::world_diagonal(voxel_part const& part) const -> f64 { return part.Grid->diagonal() * Scale * part.Scale; }

auto voxel_object::height() const -> f64
{
    f64 top {0.0};
    for (auto const& part : Parts) {
        if (part.Grid) { top = std::max(top, part.Offset.Z + (part.Grid->Size.Z * part.Scale)); }
    }
    return top * Scale;
}

auto voxel_object::segment_hits(point3_d worldPos, point_d from, point_d to, f64 z, f64 radius) const -> bool
{
    auto const part_hits {[&](voxel_transform const& t, voxel_grid const& grid, f64 scale) -> bool {
        f64 const r {radius / scale};
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
                        if (!grid.occupied(vx, vy, vz)) { continue; }

                        if (axis_gap(c.X, vx) + axis_gap(c.Y, vy) + axis_gap(c.Z, vz) < r2) { return true; }
                    }
                }
            }
        }
        return false;
    }};

    return std::ranges::any_of(Parts, [&](voxel_part const& part) {
        return part.Grid && part_hits(make_transform(worldPos, part), *part.Grid, Scale * part.Scale);
    });
}

auto voxel_object::filled_at(point3_d worldPos, point_d pos, f64 z) const -> bool
{
    return std::ranges::any_of(Parts, [&](voxel_part const& part) {
        if (!part.Grid) { return false; }

        auto const l {make_transform(worldPos, part).to_local({.X = pos.X, .Y = pos.Y, .Z = z})};
        return part.Grid->occupied(static_cast<i32>(std::floor(l.X)),
                                   static_cast<i32>(std::floor(l.Y)),
                                   static_cast<i32>(std::floor(l.Z)));
    });
}

////////////////////////////////////////////////////////////

object_base::object_base(uid id, object_desc const& desc)
    : _id {id}
    , _name {desc.Name}
    , _position {desc.Position}
    , _visual {desc.Visual}
    , _mapMarker {desc.MapMarker}
    , _lifeTime {desc.LifeTime}
{
}

auto object_base::id() const -> uid { return _id; }
auto object_base::name() const -> string const& { return _name; }

auto object_base::position() const -> point3_d { return _position; }
void object_base::set_position(point3_d p) { _position = p; }
void object_base::set_xy(point_d p) { _position = _position.with_xy(p); }

auto object_base::floor_z() const -> f64 { return _floorZ; }
void object_base::set_floor_z(f64 floorZ) { _floorZ = floorZ; }
auto object_base::world_z() const -> f64 { return _floorZ + _position.Z; }
auto object_base::world_position() const -> point3_d { return _position.with_z(world_z()); }

auto object_base::center_z() const -> f64
{
    if (auto const* s {sprite()}) { return world_z() + (s->Size.Height * 0.5); }
    if (auto const* v {voxel()}) { return world_z() + (v->height() * 0.5); }

    return world_z();
}

auto object_base::map_marker() const -> color { return _mapMarker; }

auto object_base::is_seen() const -> bool { return _seen; }
void object_base::set_seen(bool b) { _seen = b; }

auto object_base::dead() const -> bool { return _dead; }
void object_base::kill() { _dead = true; }

auto object_base::life_time() const -> std::optional<milliseconds> { return _lifeTime; }

void object_base::age(milliseconds dt)
{
    if (!_lifeTime) { return; }

    *_lifeTime -= dt;
    if (*_lifeTime <= milliseconds::zero()) { kill(); }
}

auto object_base::sprite() const -> sprite_object const* { return std::get_if<sprite_object>(&_visual); }
auto object_base::sprite() -> sprite_object* { return std::get_if<sprite_object>(&_visual); }
auto object_base::voxel() const -> voxel_object const* { return std::get_if<voxel_object>(&_visual); }
auto object_base::voxel() -> voxel_object* { return std::get_if<voxel_object>(&_visual); }

////////////////////////////////////////////////////////////

enemy::enemy(uid id, object_desc const& desc, f64 radius, bool flying, i32 health, point_d direction)
    : object_base {id, desc}
    , solid {.Radius = radius}
    , movable {.Direction = direction}
    , Health {health}
    , _flying {flying}
{
}

projectile::projectile(uid id, object_desc const& desc, f64 radius, point_d direction, uid owner, std::optional<dynamic_light> light)
    : object_base {id, desc}
    , solid {.Radius = radius}
    , movable {.Direction = direction}
    , Owner {owner}
{
    Light = light;
}

pickup::pickup(uid id, object_desc const& desc, f64 radius, i32 item, std::optional<dynamic_light> light)
    : object_base {id, desc}
    , solid {.Radius = radius}
    , Item {item}
{
    Light = light;
}

object::object(uid id, object_desc const& desc, f64 radius, std::optional<dynamic_light> light)
    : object_base {id, desc}
    , solid {.Radius = radius}
{
    Light = light;
}

////////////////////////////////////////////////////////////

auto base_of(level_object& o) -> object_base&
{
    return std::visit([](auto& x) -> object_base& { return x; }, o);
}

auto base_of(level_object const& o) -> object_base const&
{
    return std::visit([](auto const& x) -> object_base const& { return x; }, o);
}

auto is_flying(level_object const& o) -> bool
{
    return std::visit([](auto const& x) { return x.flying(); }, o);
}

auto world_light(level_object const& o) -> std::optional<dynamic_light>
{
    return std::visit([](auto const& x) -> std::optional<dynamic_light> {
        if constexpr (std::derived_from<std::remove_cvref_t<decltype(x)>, emitter>) {
            if (!x.Light) { return std::nullopt; }

            auto light {*x.Light};
            light.Position = x.world_position() + light.Position;
            return light;
        } else {
            return std::nullopt;
        }
    },
                      o);
}

static auto OverlapsSolid(object_base const& self, solid const& body, point_d from, point_d to, f64 radius, std::optional<f64> z) -> bool
{
    if (body.Radius <= 0) { return false; }

    auto const c {self.position().xy()};
    auto const s {to - from};
    f64 const  len2 {s.length_squared()};
    f64 const  t {len2 > 0.0 ? std::clamp((c - from).dot(s) / len2, 0.0, 1.0) : 0.0};
    f64 const  combined {radius + body.Radius};
    if ((c - (from + (s * t))).length_squared() >= combined * combined) { return false; }

    if (z) {
        if (auto const* v {self.voxel()}) { return v->segment_hits(self.world_position(), from, to, *z, radius); }
    }
    return true;
}

auto overlaps(level_object const& o, point_d from, point_d to, f64 radius, std::optional<f64> z) -> bool
{
    return std::visit([&](auto const& x) -> bool {
        if constexpr (std::derived_from<std::remove_cvref_t<decltype(x)>, solid>) {
            return OverlapsSolid(x, x, from, to, radius, z);
        } else {
            return false;
        }
    },
                      o);
}
