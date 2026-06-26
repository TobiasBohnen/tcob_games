// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Walls.hpp"

auto empty_cell::intersect(cell_intersect const& ci) const -> wall_hit
{
    return {.Hit = false};
}

auto cell::intersect(cell_intersect const& ci4) const -> wall_hit
{
    return {.Hit = false};
}

auto normal_wall::intersect(cell_intersect const& ci) const -> wall_hit
{
    // wallX: fractional hit position on the wall face
    f64 wallX {!ci.Side
                   ? ci.RayOrigin.Y + (ci.Distance * ci.RayDir.Y)
                   : ci.RayOrigin.X + (ci.Distance * ci.RayDir.X)};
    wallX -= std::floor(wallX);

    // mirror texture on specific faces to avoid backwards-looking textures
    bool const mirror {(!ci.Side && ci.RayDir.X > 0) || (ci.Side && ci.RayDir.Y < 0)};
    f64 const  segmentT {mirror ? 1.0 - wallX : wallX};

    return {.Distance = ci.Distance, .SegmentT = segmentT, .Texture = Texture, .Hit = true, .Shaded = ci.Side};
}

auto door_wall::intersect(cell_intersect const& ci) const -> wall_hit
{
    bool const isNS {Orientation == orientation::BlocksNorthSouth};

    f64 const rdLeaf {isNS ? ci.RayDir.Y : ci.RayDir.X};
    f64 const roLeaf {isNS ? ci.RayOrigin.Y : ci.RayOrigin.X};
    f64 const cLeaf {static_cast<f64>(isNS ? ci.Cell.Y : ci.Cell.X)};

    f64 const rdCross {isNS ? ci.RayDir.X : ci.RayDir.Y};
    f64 const roCross {isNS ? ci.RayOrigin.X : ci.RayOrigin.Y};
    f64 const cCross {static_cast<f64>(isNS ? ci.Cell.X : ci.Cell.Y)};

    wall_hit closestHit {};
    f64      minT {1e30};

    if (rdLeaf != 0.0) {
        f64 const t = (cLeaf + 0.5 - roLeaf) / rdLeaf;
        if (t >= 0.0 && t < minT) {
            f64 const hitCross {roCross + (rdCross * t)};
            f64 const segStart {isNS ? cCross + Timer : cCross};
            f64 const segEnd {isNS ? cCross + 1.0 : cCross + (1.0 - Timer)};
            if (hitCross >= segStart && hitCross <= segEnd) {
                f64 const segmentT {isNS ? hitCross - segStart : segEnd - hitCross};
                closestHit = {.Distance = t, .SegmentT = segmentT, .Texture = Texture, .Hit = true, .Shaded = ci.Side};
                minT       = t;
            }
        }
    }

    if (rdCross != 0.0) {
        for (f64 const offset : {0.0, 1.0}) {
            f64 const t {(cCross + offset - roCross) / rdCross};
            if (t >= 0.0 && t < minT) {
                f64 const hitLeaf {roLeaf + (rdLeaf * t)};
                if (hitLeaf >= cLeaf && hitLeaf <= cLeaf + 1.0) {
                    closestHit = {.Distance = t, .SegmentT = hitLeaf - cLeaf, .Texture = FrameTexture, .Hit = true, .Shaded = !ci.Side};
                    minT       = t;
                }
            }
        }
    }

    return closestHit;
}

auto door_wall::update(f64 dt) -> bool
{
    if (State == wall_state::Opening) {
        Timer += dt * OpenSpeed;
        if (Timer >= 1.0) {
            Timer = 1.0;
            State = wall_state::Open;
        }
        return true;
    }
    if (State == wall_state::Closing) {
        Timer -= dt * OpenSpeed;
        if (Timer <= 0.0) {
            Timer = 0;
            State = wall_state::Closed;
        }
        return true;
    }
    return false;
}

void door_wall::toggle()
{
    switch (State) {
    case wall_state::Closed:
    case wall_state::Closing:
        State = wall_state::Opening;
        break;
    case wall_state::Open:
    case wall_state::Opening:
        State = wall_state::Closing;
        break;
    }
}

auto push_wall::intersect(cell_intersect const& ci) const -> wall_hit
{
    if (State == wall_state::Open) { return {}; }

    if (PushDirection.X != 0) {
        if ((ci.RayDir.X > 0 && PushDirection.X < 0) || (ci.RayDir.X < 0 && PushDirection.X > 0)) { return {}; } // backface culling

        f64 const nearEdgeX {PushDirection.X > 0 ? static_cast<f64>(ci.Cell.X) : static_cast<f64>(ci.Cell.X) + 1.0};
        f64 const faceX {nearEdgeX + (PushDirection.X * Timer)};

        if (ci.RayDir.X == 0.0) { return {}; }

        f64 const t {(faceX - ci.RayOrigin.X) / ci.RayDir.X};
        if (t < 0.0) { return {}; }

        f64 const hitY {ci.RayOrigin.Y + (ci.RayDir.Y * t)};
        f64 const segStart {static_cast<f64>(ci.Cell.Y)};
        f64 const segEnd {static_cast<f64>(ci.Cell.Y) + 1.0};

        if (hitY < segStart || hitY > segEnd) { return {}; }

        f64 const segmentT {hitY - segStart};
        return wall_hit {.Distance = t, .SegmentT = segmentT, .Texture = Texture, .Hit = true, .Shaded = ci.Side};
    }

    if (PushDirection.Y != 0) {
        if ((ci.RayDir.Y > 0 && PushDirection.Y < 0) || (ci.RayDir.Y < 0 && PushDirection.Y > 0)) { return {}; } // backface culling

        f64 const nearEdgeY {PushDirection.Y > 0 ? static_cast<f64>(ci.Cell.Y) : static_cast<f64>(ci.Cell.Y) + 1.0};
        f64 const faceY {nearEdgeY + (PushDirection.Y * Timer)};

        if (ci.RayDir.Y == 0.0) { return {}; }

        f64 const t {(faceY - ci.RayOrigin.Y) / ci.RayDir.Y};
        if (t < 0.0) { return {}; }

        f64 const hitX {ci.RayOrigin.X + (ci.RayDir.X * t)};
        f64 const segStart {static_cast<f64>(ci.Cell.X)};
        f64 const segEnd {static_cast<f64>(ci.Cell.X) + 1.0};

        if (hitX < segStart || hitX > segEnd) { return {}; }

        f64 const segmentT {hitX - segStart};
        return wall_hit {.Distance = t, .SegmentT = segmentT, .Texture = Texture, .Hit = true, .Shaded = ci.Side};
    }

    return {};
}

auto push_wall::update(f64 dt) -> bool
{
    if (State == wall_state::Opening) {
        Timer += dt * OpenSpeed;
        if (Timer >= 1.0) {
            Timer = 1.0;
            State = wall_state::Open;
        }
        return true;
    }
    if (State == wall_state::Closing) {
        Timer -= dt * OpenSpeed;
        if (Timer <= 0.0) {
            Timer = 0;
            State = wall_state::Closed;
        }
        return true;
    }
    return false;
}

void push_wall::toggle()
{
    switch (State) {
    case wall_state::Closed:
    case wall_state::Closing:
        State = wall_state::Opening;
        break;
    case wall_state::Open:
    case wall_state::Opening:
        break;
    }
}

auto box_wall::intersect(cell_intersect const& ci) const -> wall_hit
{
    f64 const minX {ci.Cell.X + LocalBounds.left()};
    f64 const minY {ci.Cell.Y + LocalBounds.top()};
    f64 const maxX {minX + LocalBounds.width()};
    f64 const maxY {minY + LocalBounds.height()};

    f64 tMinX {0}, tMaxX {0};
    if (ci.RayDir.X != 0.0) {
        f64 const t1 {(minX - ci.RayOrigin.X) / ci.RayDir.X};
        f64 const t2 {(maxX - ci.RayOrigin.X) / ci.RayDir.X};
        tMinX = std::min(t1, t2);
        tMaxX = std::max(t1, t2);
    } else {
        if (ci.RayOrigin.X < minX || ci.RayOrigin.X > maxX) { return {}; }
        tMinX = -std::numeric_limits<f64>::infinity();
        tMaxX = std::numeric_limits<f64>::infinity();
    }

    f64 tMinY {0}, tMaxY {0};
    if (ci.RayDir.Y != 0.0) {
        f64 const t1 {(minY - ci.RayOrigin.Y) / ci.RayDir.Y};
        f64 const t2 {(maxY - ci.RayOrigin.Y) / ci.RayDir.Y};
        tMinY = std::min(t1, t2);
        tMaxY = std::max(t1, t2);
    } else {
        if (ci.RayOrigin.Y < minY || ci.RayOrigin.Y > maxY) { return {}; }
        tMinY = -std::numeric_limits<f64>::infinity();
        tMaxY = std::numeric_limits<f64>::infinity();
    }

    f64 const tEnter {std::max(tMinX, tMinY)};
    f64 const tExit {std::min(tMaxX, tMaxY)};

    if (tEnter > tExit || tExit < 0.0) { return {}; } // no overlap, or box is entirely behind the ray

    f64 const t {tEnter >= 0.0 ? tEnter : tExit};     // if ray origin is inside the box, use the exit point

    f64 segmentT {0};
    if (tMinX > tMinY) {
        f64 const hitY {ci.RayOrigin.Y + (ci.RayDir.Y * t)};
        segmentT = (hitY - minY) / (maxY - minY);
    } else {
        f64 const hitX {ci.RayOrigin.X + (ci.RayDir.X * t)};
        segmentT = (hitX - minX) / (maxX - minX);
    }

    bool const hitSide {tMinX <= tMinY};
    return wall_hit {.Distance = t, .SegmentT = segmentT, .Texture = Texture, .Hit = true, .Shaded = hitSide};
}

auto diagonal_wall::intersect(cell_intersect const& ci) const -> wall_hit
{
    f64 const cX {static_cast<f64>(ci.Cell.X)};
    f64 const cY {static_cast<f64>(ci.Cell.Y)};

    f64 t {-1.0};

    if (Orientation == orientation::NorthWestToSouthEast) {
        f64 const denom {ci.RayDir.X - ci.RayDir.Y};
        if (std::abs(denom) > 1e-6) {
            t = ((cX - cY) - (ci.RayOrigin.X - ci.RayOrigin.Y)) / denom;
        }
    } else {
        f64 const denom {ci.RayDir.X + ci.RayDir.Y};
        if (std::abs(denom) > 1e-6) {
            t = ((cX + cY + 1.0) - (ci.RayOrigin.X + ci.RayOrigin.Y)) / denom;
        }
    }

    if (t < 0.0) { return {}; }

    f64 const hitX {ci.RayOrigin.X + (ci.RayDir.X * t)};
    f64 const hitY {ci.RayOrigin.Y + (ci.RayDir.Y * t)};

    if (hitX < cX || hitX > cX + 1.0 || hitY < cY || hitY > cY + 1.0) {
        return {};
    }

    f64 segmentT {0.0};
    if (Orientation == orientation::NorthWestToSouthEast) {
        segmentT = hitX - cX;
    } else {
        segmentT = 1.0 - (hitX - cX);
    }

    return wall_hit {.Distance = t, .SegmentT = segmentT, .Texture = Texture, .Hit = true, .Shaded = Orientation == orientation::SouthWestToNorthEast};
}

auto round_pillar::intersect(cell_intersect const& ci) const -> wall_hit
{
    point_d const center {ci.Cell.X + 0.5, ci.Cell.Y + 0.5};
    point_d const oc {ci.RayOrigin.X - center.X, ci.RayOrigin.Y - center.Y};

    f64 const a {ci.RayDir.dot(ci.RayDir)};
    f64 const b {2.0 * oc.dot(ci.RayDir)};
    f64 const c {oc.dot(oc) - (Radius * Radius)};
    f64 const discriminant {(b * b) - (4.0 * a * c)};

    if (discriminant < 0.0) { return {}; }

    f64 const t {(-b - std::sqrt(discriminant)) / (2.0 * a)};
    if (t < 0.0) { return {}; }

    point_d const hit {ci.RayOrigin.X + (ci.RayDir.X * t), ci.RayOrigin.Y + (ci.RayDir.Y * t)};
    f64 const     angle {std::atan2(hit.Y - center.Y, hit.X - center.X)};
    f64 const     segmentT {(angle + std::numbers::pi) / (2.0 * std::numbers::pi)};

    // side based on which quadrant the hit normal faces
    point_d const normal {hit.X - center.X, hit.Y - center.Y};
    bool const    hitSide {std::abs(normal.Y) > std::abs(normal.X)};

    return wall_hit {.Distance = t, .SegmentT = segmentT, .Texture = Texture, .Hit = true, .Shaded = hitSide};
}

auto thin_wall::intersect(cell_intersect const& ci) const -> wall_hit
{
    bool const isNS {Orientation == orientation::BlocksNorthSouth};
    f64 const  rd {isNS ? ci.RayDir.Y : ci.RayDir.X};
    f64 const  ro {isNS ? ci.RayOrigin.Y : ci.RayOrigin.X};
    f64 const  c {static_cast<f64>(isNS ? ci.Cell.Y : ci.Cell.X)};
    f64 const  roCross {isNS ? ci.RayOrigin.X : ci.RayOrigin.Y};
    f64 const  rdCross {isNS ? ci.RayDir.X : ci.RayDir.Y};
    f64 const  cCross {static_cast<f64>(isNS ? ci.Cell.X : ci.Cell.Y)};

    if (rd == 0.0) { return {}; }

    f64 const t {(c + Offset - ro) / rd};
    if (t < 0.0) { return {}; }

    f64 const hitCross {roCross + (rdCross * t)};
    if (hitCross < cCross || hitCross > cCross + 1.0) { return {}; }

    f64 const segmentT {hitCross - cCross};

    return wall_hit {.Distance = t, .SegmentT = segmentT, .Texture = Texture, .Hit = true, .Transparent = true, .Shaded = !isNS};
}
