// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Walls.hpp"

auto empty::intersect(point_i, point_d, point_d, bool, f64) const -> wall_hit
{
    return {.Hit = false};
}

auto normal_wall::intersect(point_i cell, point_d rayOrigin, point_d rayDir, bool side, f64 dist) const -> wall_hit
{
    // wallX: fractional hit position on the wall face
    f64 wallX {!side
                   ? rayOrigin.Y + (dist * rayDir.Y)
                   : rayOrigin.X + (dist * rayDir.X)};
    wallX -= std::floor(wallX);

    // mirror texture on specific faces to avoid backwards-looking textures
    bool const mirror {(!side && rayDir.X > 0) || (side && rayDir.Y < 0)};
    f64 const  segmentT {mirror ? 1.0 - wallX : wallX};

    return {.Hit = true, .Distance = dist, .SegmentT = segmentT, .Texture = Texture, .Side = side};
}

auto door_wall::intersect(point_i cell, point_d rayOrigin, point_d rayDir, bool side, f64 dist) const -> wall_hit
{
    bool const isNS {Orientation == orientation::BlocksNorthSouth};

    f64 const rdLeaf {isNS ? rayDir.Y : rayDir.X};
    f64 const roLeaf {isNS ? rayOrigin.Y : rayOrigin.X};
    f64 const cLeaf {static_cast<f64>(isNS ? cell.Y : cell.X)};

    f64 const rdCross {isNS ? rayDir.X : rayDir.Y};
    f64 const roCross {isNS ? rayOrigin.X : rayOrigin.Y};
    f64 const cCross {static_cast<f64>(isNS ? cell.X : cell.Y)};

    wall_hit closestHit {};
    f64      minT {1e30};

    if (rdLeaf != 0.0) {
        f64 const t = (cLeaf + 0.5 - roLeaf) / rdLeaf;
        if (t >= 0.0 && t < minT) {
            f64 const hitCross {roCross + (rdCross * t)};
            f64 const segStart {cCross + Timer};
            f64 const segEnd {cCross + 1.0};

            if (hitCross >= segStart && hitCross <= segEnd) {
                closestHit = {.Hit = true, .Distance = t, .SegmentT = hitCross - segStart, .Texture = Texture, .Side = side};
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
                    closestHit = {.Hit = true, .Distance = t, .SegmentT = hitLeaf - cLeaf, .Texture = FrameTexture, .Side = !side};
                    minT       = t;
                }
            }
        }
    }

    return closestHit;
}

auto push_wall::intersect(point_i cell, point_d rayOrigin, point_d rayDir, bool side, f64 dist) const -> wall_hit
{
    if (State == wall_state::Open) { return {}; }

    if (PushDirection.X != 0) {
        if ((rayDir.X > 0 && PushDirection.X < 0) || (rayDir.X < 0 && PushDirection.X > 0)) { return {}; } // backface culling

        f64 const nearEdgeX {PushDirection.X > 0 ? static_cast<f64>(cell.X) : static_cast<f64>(cell.X) + 1.0};
        f64 const faceX {nearEdgeX + (PushDirection.X * Timer)};

        if (rayDir.X == 0.0) { return {}; }

        f64 const t {(faceX - rayOrigin.X) / rayDir.X};
        if (t < 0.0) { return {}; }

        f64 const hitY {rayOrigin.Y + (rayDir.Y * t)};
        f64 const segStart {static_cast<f64>(cell.Y)};
        f64 const segEnd {static_cast<f64>(cell.Y) + 1.0};

        if (hitY < segStart || hitY > segEnd) { return {}; }

        f64 const segmentT {hitY - segStart};
        return wall_hit {.Hit = true, .Distance = t, .SegmentT = segmentT, .Texture = Texture, .Side = side};
    }

    if (PushDirection.Y != 0) {
        if ((rayDir.Y > 0 && PushDirection.Y < 0) || (rayDir.Y < 0 && PushDirection.Y > 0)) { return {}; } // backface culling

        f64 const nearEdgeY {PushDirection.Y > 0 ? static_cast<f64>(cell.Y) : static_cast<f64>(cell.Y) + 1.0};
        f64 const faceY {nearEdgeY + (PushDirection.Y * Timer)};

        if (rayDir.Y == 0.0) { return {}; }

        f64 const t {(faceY - rayOrigin.Y) / rayDir.Y};
        if (t < 0.0) { return {}; }

        f64 const hitX {rayOrigin.X + (rayDir.X * t)};
        f64 const segStart {static_cast<f64>(cell.X)};
        f64 const segEnd {static_cast<f64>(cell.X) + 1.0};

        if (hitX < segStart || hitX > segEnd) { return {}; }

        f64 const segmentT {hitX - segStart};
        return wall_hit {.Hit = true, .Distance = t, .SegmentT = segmentT, .Texture = Texture, .Side = side};
    }

    return {};
}

auto half_wall::intersect(point_i cell, point_d rayOrigin, point_d rayDir, bool side, f64 dist) const -> wall_hit
{
    f64 const minX {cell.X + LocalBounds.left()};
    f64 const minY {cell.Y + LocalBounds.top()};
    f64 const maxX {minX + LocalBounds.width()};
    f64 const maxY {minY + LocalBounds.height()};

    f64 tMinX {0}, tMaxX {0};
    if (rayDir.X != 0.0) {
        f64 const t1 {(minX - rayOrigin.X) / rayDir.X};
        f64 const t2 {(maxX - rayOrigin.X) / rayDir.X};
        tMinX = std::min(t1, t2);
        tMaxX = std::max(t1, t2);
    } else {
        if (rayOrigin.X < minX || rayOrigin.X > maxX) { return {}; }
        tMinX = -std::numeric_limits<f64>::infinity();
        tMaxX = std::numeric_limits<f64>::infinity();
    }

    f64 tMinY {0}, tMaxY {0};
    if (rayDir.Y != 0.0) {
        f64 const t1 {(minY - rayOrigin.Y) / rayDir.Y};
        f64 const t2 {(maxY - rayOrigin.Y) / rayDir.Y};
        tMinY = std::min(t1, t2);
        tMaxY = std::max(t1, t2);
    } else {
        if (rayOrigin.Y < minY || rayOrigin.Y > maxY) { return {}; }
        tMinY = -std::numeric_limits<f64>::infinity();
        tMaxY = std::numeric_limits<f64>::infinity();
    }

    f64 const tEnter {std::max(tMinX, tMinY)};
    f64 const tExit {std::min(tMaxX, tMaxY)};

    if (tEnter > tExit || tExit < 0.0) { return {}; } // no overlap, or box is entirely behind the ray

    f64 const t {tEnter >= 0.0 ? tEnter : tExit};     // if ray origin is inside the box, use the exit point

    f64 segmentT {0};
    if (tMinX > tMinY) {
        f64 const hitY {rayOrigin.Y + (rayDir.Y * t)};
        segmentT = (hitY - minY) / (maxY - minY);
    } else {
        f64 const hitX {rayOrigin.X + (rayDir.X * t)};
        segmentT = (hitX - minX) / (maxX - minX);
    }

    bool const hitSide {tMinX > tMinY};
    return wall_hit {.Hit = true, .Distance = t, .SegmentT = segmentT, .Texture = Texture, .Side = hitSide};
}

auto diagonal_wall::intersect(point_i cell, point_d rayOrigin, point_d rayDir, bool side, f64 dist) const -> wall_hit
{
    f64 const cX {static_cast<f64>(cell.X)};
    f64 const cY {static_cast<f64>(cell.Y)};

    f64 t {-1.0};

    if (Orientation == orientation::NorthWestToSouthEast) {
        f64 const denom = rayDir.X - rayDir.Y;
        if (std::abs(denom) > 1e-6) {
            t = ((cX - cY) - (rayOrigin.X - rayOrigin.Y)) / denom;
        }
    } else {
        f64 const denom = rayDir.X + rayDir.Y;
        if (std::abs(denom) > 1e-6) {
            t = ((cX + cY + 1.0) - (rayOrigin.X + rayOrigin.Y)) / denom;
        }
    }

    if (t < 0.0) { return {}; }

    f64 const hitX {rayOrigin.X + (rayDir.X * t)};
    f64 const hitY {rayOrigin.Y + (rayDir.Y * t)};

    if (hitX < cX || hitX > cX + 1.0 || hitY < cY || hitY > cY + 1.0) {
        return {};
    }

    f64 segmentT {0.0};
    if (Orientation == orientation::NorthWestToSouthEast) {
        segmentT = hitX - cX;
    } else {
        segmentT = 1.0 - (hitX - cX);
    }

    return wall_hit {
        .Hit      = true,
        .Distance = t,
        .SegmentT = segmentT,
        .Texture  = Texture,
        .Side     = Orientation == orientation::SouthWestToNorthEast};
}
