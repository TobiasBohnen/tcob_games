// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Raycaster.hpp"

#include <algorithm>
#include <cmath>

#include "Textures.hpp"

constexpr f64 fogFloor {0.2};
constexpr f64 fogDistance {8.0};

raycaster::raycaster(cache& cache, std::vector<sprite>& sprites, size_i screenSize, f64 horizontalFovDegrees)
    : _cache {cache}
    , _sprites {sprites}
    , _screenSize {screenSize}
{
    _zBuffer.resize(_screenSize.Width);

    _rowDist.resize(_screenSize.Height);
    for (i32 y {0}; y < _screenSize.Height; y++) {
        if (2.0 * y == _screenSize.Height) {
            _rowDist[y] = std::numeric_limits<f64>::infinity();
            continue;
        }
        _rowDist[y] = _screenSize.Height / ((2.0 * y) - _screenSize.Height);
    }

    f64 const fov {horizontalFovDegrees * TAU / 360.0};
    _dir   = {-1, 0};
    _plane = {0, std::tan(fov / 2.0)};

    _projPlaneDist = (_screenSize.Width / 2.0) / std::tan(fov / 2.0);
}

auto raycaster::get_player_position() const -> point_d
{
    return _pos;
}

void raycaster::set_player_position(point_d pos)
{
    _pos = pos;
}

void raycaster::set_world_map(world_map_t const& worldMap)
{
    _worldMap = &worldMap;
}

auto raycaster::move(f64 forwardAmount, f64 strafeAmount, f64 rotateAmount) -> bool
{
    bool retValue {false};

    // Move Forward/Backward
    if (forwardAmount != 0) {
        point_d const newPos {_pos + (_dir * forwardAmount)};

        if (is_position_clear(newPos, PlayerRadius)) {
            _pos     = newPos;
            retValue = true;
        } else if (is_position_clear({_pos.X, newPos.Y}, PlayerRadius)) {
            _pos.Y   = newPos.Y;
            retValue = true;
        } else if (is_position_clear({newPos.X, _pos.Y}, PlayerRadius)) {
            _pos.X   = newPos.X;
            retValue = true;
        }
    }

    // Strafe Left/Right (perpendicular to _dir)
    if (strafeAmount != 0) {
        point_d const strafe {_dir.as_perpendicular()};
        point_d const newPos {_pos + (strafe * strafeAmount)};

        if (is_position_clear(newPos, PlayerRadius)) {
            _pos     = newPos;
            retValue = true;
        } else if (is_position_clear({_pos.X, newPos.Y}, PlayerRadius)) {
            _pos.Y   = newPos.Y;
            retValue = true;
        } else if (is_position_clear({newPos.X, _pos.Y}, PlayerRadius)) {
            _pos.X   = newPos.X;
            retValue = true;
        }
    }

    // Rotate (both direction and plane vectors must rotate together)
    if (rotateAmount != 0) {
        f64 const old_dirX {_dir.X};
        _dir.X = (_dir.X * std::cos(rotateAmount)) - (_dir.Y * std::sin(rotateAmount));
        _dir.Y = (old_dirX * std::sin(rotateAmount)) + (_dir.Y * std::cos(rotateAmount));

        f64 const oldPlaneX {_plane.X};
        _plane.X = (_plane.X * std::cos(rotateAmount)) - (_plane.Y * std::sin(rotateAmount));
        _plane.Y = (oldPlaneX * std::sin(rotateAmount)) + (_plane.Y * std::cos(rotateAmount));

        retValue = true;
    }

    return retValue;
}

void raycaster::cast(i32 x, u32* screenBuf)
{
    // WALL CASTING
    // calculate ray position and direction
    f64 const     cameraX {(2.0 * x / _screenSize.Width) - 1.0}; // x-coordinate in camera space
    point_d const rayDir {_dir + (_plane * cameraX)};

    // which box of the map we're in
    point_i map {static_cast<point_i>(_pos)};

    // length of ray from one x or y-side to next x or y-side
    point_d const deltaDist {(rayDir.X == 0) ? 1e30 : std::abs(1 / rayDir.X), (rayDir.Y == 0) ? 1e30 : std::abs(1 / rayDir.Y)};

    // what direction to step in x or y-direction (either +1 or -1)
    point_i step {};

    // length of ray from current position to next x or y-side
    point_d sideDist {};

    // calculate step and initial sideDist
    if (rayDir.X < 0) {
        step.X     = -1;
        sideDist.X = (_pos.X - map.X) * deltaDist.X;
    } else {
        step.X     = 1;
        sideDist.X = (map.X + 1.0 - _pos.X) * deltaDist.X;
    }
    if (rayDir.Y < 0) {
        step.Y     = -1;
        sideDist.Y = (_pos.Y - map.Y) * deltaDist.Y;
    } else {
        step.Y     = 1;
        sideDist.Y = (map.Y + 1.0 - _pos.Y) * deltaDist.Y;
    }

    bool side {false}; // was a NS or a EW wall hit?

    // perform DDA
    for (;;) {
        // jump to next map square, either in x-direction, or in y-direction
        if (sideDist.X < sideDist.Y) {
            sideDist.X += deltaDist.X;
            map.X += step.X;
            side = false;
        } else {
            sideDist.Y += deltaDist.Y;
            map.Y += step.Y;
            side = true;
        }
        // Check if ray has hit a wall
        if ((*_worldMap)[map] > 0) { break; }
    }

    // Calculate distance projected on camera direction (Euclidean distance would give fisheye effect!)
    f64 const perpWallDist {!side ? sideDist.X - deltaDist.X : sideDist.Y - deltaDist.Y};

    _zBuffer[x] = perpWallDist;

    // Calculate height of line to draw on screen
    i32 const lineHeight {static_cast<i32>(_projPlaneDist / perpWallDist)};

    // calculate lowest and highest pixel to fill in current stripe
    i32 const drawStart {std::max((-lineHeight / 2) + (_screenSize.Height / 2), 0)};
    i32 const drawEnd {std::min((lineHeight / 2) + (_screenSize.Height / 2), _screenSize.Height - 1)};

    // calculate value of wallX
    f64 wallX {!side ? _pos.Y + (perpWallDist * rayDir.Y) : _pos.X + (perpWallDist * rayDir.X)}; // where exactly the wall was hit
    wallX -= std::floor(wallX);
    {
        i32 const    texNum {(*_worldMap)[map] - 1};
        auto const*  tex {_cache.texture(texNum, 0)};
        size_i const texSize {wallSize};

        // x coordinate on the texture
        i32 texX {static_cast<i32>(wallX * static_cast<f64>(texSize.Width))};
        if ((!side && rayDir.X > 0) || (side && rayDir.Y < 0)) {
            texX = texSize.Width - texX - 1;
        }

        // How much to increase the texture coordinate per screen pixel
        f64 const texStep {1.0 * texSize.Height / lineHeight};
        // Starting texture coordinate
        f64       texPos {(drawStart - (_screenSize.Height / 2) + (lineHeight / 2)) * texStep};

        f64 const wallFaceFactor {side ? 0.5 : 1.0};
        f64 const wallFogFactor {std::max(1.0 - (perpWallDist / fogDistance), fogFloor)};
        f64 const wallDarkenFactor {wallFaceFactor * wallFogFactor};

        for (i32 y {drawStart}; y < drawEnd; y++) {
            // Cast the texture coordinate to integer, and mask with (texSize.Height - 1) in case of
            // overflow — requires texSize.Height to be a power of two
            i32 const texY {static_cast<i32>(texPos) & (texSize.Height - 1)};
            texPos += texStep;
            cache::copy(screenBuf + x + ((_screenSize.Height - y - 1) * _screenSize.Width), tex, (texX + (texY * texSize.Width)) * textureBPP, wallDarkenFactor);
        }
    }

    // FLOOR CASTING (vertical version, directly after drawing the vertical wall stripe for the current x)
    point_d floorWall {}; // x, y position of the floor texel at the bottom of the wall

    // 4 different wall directions possible
    if (!side && rayDir.X > 0) {
        floorWall.X = map.X;
        floorWall.Y = map.Y + wallX;
    } else if (!side && rayDir.X < 0) {
        floorWall.X = map.X + 1.0;
        floorWall.Y = map.Y + wallX;
    } else if (side && rayDir.Y > 0) {
        floorWall.X = map.X + wallX;
        floorWall.Y = map.Y;
    } else {
        floorWall.X = map.X + wallX;
        floorWall.Y = map.Y + 1.0;
    }

    // draw the floor from drawEnd to the bottom of the screen
    f64 const    invPerpWallDist {1.0 / perpWallDist};
    auto const*  floorTex {_cache.texture(floorTexture, 0)};
    size_i const floorSize {wallSize};
    auto const*  ceilTex {_cache.texture(ceilingTexture, 0)};

    for (i32 y {drawEnd}; y < _screenSize.Height; y++) {
        f64 const weight {std::min(_rowDist[y] * invPerpWallDist, 1.0)};

        point_d const currentFloor {(weight * floorWall.X) + ((1.0 - weight) * _pos.X), (weight * floorWall.Y) + ((1.0 - weight) * _pos.Y)};

        point_d const delta {currentFloor.X - _pos.X, currentFloor.Y - _pos.Y};
        f64 const     floorDist {std::sqrt((delta.X * delta.X) + (delta.Y * delta.Y))};
        f64 const     fogFactor {std::max(1.0 - (floorDist / fogDistance), fogFloor)};

        i32 const texelX {static_cast<i32>(currentFloor.X * floorSize.Width) & (floorSize.Width - 1)};
        i32 const texelY {static_cast<i32>(currentFloor.Y * floorSize.Height) & (floorSize.Height - 1)};
        i32 const texelOffset {(texelX + (texelY * floorSize.Width)) * textureBPP};

        cache::copy(screenBuf + x + ((_screenSize.Height - y - 1) * _screenSize.Width), floorTex, texelOffset, fogFactor);
        cache::copy(screenBuf + x + (y * _screenSize.Width), ceilTex, texelOffset, fogFactor);
    }
}

static auto is_magenta(u8 const* tex, i32 offset) -> bool
{
    return tex[offset + 0] == 0x98
        && tex[offset + 1] == 0x00
        && tex[offset + 2] == 0x88;
}

static auto sprite_facing_index(degree_d spriteFacing, point_d spritePos, point_d cameraPos) -> i32
{
    degree_d const viewAngle {spritePos.angle_to(cameraPos)};

    degree_d const relativeAngle {
        (viewAngle - spriteFacing).as_normalized(angle_normalize::PositiveFullTurn)};

    constexpr f64 wedge {360.0 / 8.0};
    i32 const     index {static_cast<i32>((relativeAngle.Value + (wedge / 2.0)) / wedge) % 8};

    return index;
}

void raycaster::draw_sprites(u32* screenBuf)
{
    // sort sprites from far to near
    std::vector<isize> order(_sprites.size());
    std::vector<f64>   spriteDist(_sprites.size());
    for (isize i {0}; i < std::ssize(_sprites); ++i) {
        order[i] = i;
        point_d const d {_pos.X - _sprites[i].Pos.X, _pos.Y - _sprites[i].Pos.Y};
        spriteDist[i] = (d.X * d.X) + (d.Y * d.Y); // squared distance is enough for sorting
    }
    std::ranges::sort(order, [&](isize a, isize b) { return spriteDist[a] > spriteDist[b]; });

    // camera plane determinant, used to "unrotate" sprite position into camera space
    f64 const invDet {1.0 / ((_plane.X * _dir.Y) - (_dir.X * _plane.Y))};

    for (isize idx : order) {
        sprite const& spr {_sprites[idx]};

        // translate sprite position relative to camera
        point_d const relPos {spr.Pos - _pos};

        // transform with the inverse camera matrix
        f64 const transformX {invDet * relPos.cross(_dir)};
        f64 const transformY {invDet * _plane.cross(relPos)};
        if (transformY <= 0) { continue; } // behind camera

        i32 const spriteScreenX {static_cast<i32>((_screenSize.Width / 2.0) * (1.0 + (transformX / transformY)))};

        f64 const scale {_projPlaneDist / transformY}; // size of "one world unit" in screen pixels at this depth

        size_i const spriteSize {static_cast<i32>(std::abs(scale)) * size_i {spr.Size}};

        point_i const drawStart {std::max((-spriteSize.Width / 2) + spriteScreenX, 0),
                                 std::max((-spriteSize.Height / 2) + (_screenSize.Height / 2), 0)};
        point_i const drawEnd {std::min((spriteSize.Width / 2) + spriteScreenX, _screenSize.Width - 1),
                               std::min((spriteSize.Height / 2) + (_screenSize.Height / 2), _screenSize.Height - 1)};

        i32 const    facing {sprite_facing_index(spr.Facing, spr.Pos, _pos)};
        auto const*  tex {_cache.texture(spr.Texture, facing)};
        size_i const texSize {_cache.texture_size(spr.Texture, facing)};

        i32 const spriteLeft {spriteScreenX - (spriteSize.Width / 2)};
        i32 const spriteTop {(_screenSize.Height / 2) - (spriteSize.Height / 2)};

        f64 const texStepY {1.0 * texSize.Height / spriteSize.Height};
        f64 const texPosYStart {(drawStart.Y - spriteTop) * texStepY};

        f64 const spriteFogFactor {std::max(1.0 - (transformY / fogDistance), fogFloor)};

        for (i32 stripe {drawStart.X}; stripe < drawEnd.X; ++stripe) {
            i32 const texX {((stripe - spriteLeft) * texSize.Width) / spriteSize.Width};

            if (transformY >= _zBuffer[stripe]) { continue; }

            f64 texPos {texPosYStart};
            for (i32 y {drawStart.Y}; y < drawEnd.Y; ++y) {
                i32 texY {static_cast<i32>(texPos) % texSize.Height};
                if (texY < 0) { texY += texSize.Height; }
                texPos += texStepY;

                i32 const offset {(texX + (texY * texSize.Width)) * textureBPP};
                if (is_magenta(tex, offset)) { continue; }

                cache::copy(screenBuf + stripe + ((_screenSize.Height - y - 1) * _screenSize.Width), tex, offset, spriteFogFactor);
            }
        }
    }
}

auto raycaster::is_position_clear(point_d pos, f64 radius) const -> bool
{
    i32 const tileX {static_cast<i32>(std::floor(pos.X))};
    i32 const tileY {static_cast<i32>(std::floor(pos.Y))};

    f64 const localX {pos.X - tileX};
    f64 const localY {pos.Y - tileY};

    i32 const minTileX {static_cast<i32>(std::floor(pos.X - radius))};
    i32 const maxTileX {static_cast<i32>(std::floor(pos.X + radius))};
    i32 const minTileY {static_cast<i32>(std::floor(pos.Y - radius))};
    i32 const maxTileY {static_cast<i32>(std::floor(pos.Y + radius))};

    for (i32 ty {minTileY}; ty <= maxTileY; ++ty) {
        for (i32 tx {minTileX}; tx <= maxTileX; ++tx) {
            if (tx < 0 || tx >= world_map_t::Size.Width || ty < 0 || ty >= world_map_t::Size.Height) {
                return false;
            }
            if ((*_worldMap)[point_i {tx, ty}] == 0) {
                continue;
            }

            f64 const closestX {std::clamp(pos.X, static_cast<f64>(tx), static_cast<f64>(tx) + 1.0)};
            f64 const closestY {std::clamp(pos.Y, static_cast<f64>(ty), static_cast<f64>(ty) + 1.0)};

            f64 const dx {pos.X - closestX};
            f64 const dy {pos.Y - closestY};

            if ((dx * dx) + (dy * dy) < radius * radius) {
                return false;
            }
        }
    }

    return std::ranges::all_of(_sprites, [&](sprite const& spr) {
        if (!spr.Solid) {
            return true;
        }

        f64 const combinedRadius {radius + (spr.Size.Width / 2.0)};
        f64 const dx {pos.X - spr.Pos.X};
        f64 const dy {pos.Y - spr.Pos.Y};

        return (dx * dx) + (dy * dy) >= combinedRadius * combinedRadius;
    });
}
