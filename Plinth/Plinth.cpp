// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Plinth.hpp"

#include <algorithm>

using gfx_cache = cache<{800, 600}, {256, 256}>;

constexpr i32 mapWidth {24};
constexpr i32 mapHeight {24};

static_grid<u8, mapWidth, mapHeight> levelMap {
    {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 7, 7, 7, 7, 7, 7, 7, 7},
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 7},
    {4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7},
    {4, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7},
    {4, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 7},
    {4, 0, 4, 0, 0, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 0, 7, 7, 7, 7, 7},
    {4, 0, 5, 0, 0, 0, 0, 5, 0, 5, 0, 5, 0, 5, 0, 5, 7, 0, 0, 0, 7, 7, 7, 1},
    {4, 0, 6, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 5, 7, 0, 0, 0, 0, 0, 0, 8},
    {4, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 1},
    {4, 0, 8, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 5, 7, 0, 0, 0, 0, 0, 0, 8},
    {4, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 5, 7, 0, 0, 0, 7, 7, 7, 1},
    {4, 0, 0, 0, 0, 0, 0, 5, 5, 5, 5, 0, 5, 5, 5, 5, 7, 7, 7, 7, 7, 7, 7, 1},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {6, 6, 6, 6, 6, 6, 0, 6, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {4, 4, 4, 4, 4, 4, 0, 4, 4, 4, 6, 0, 6, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3},
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2},
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 2, 0, 0, 5, 0, 0, 2, 0, 0, 0, 2},
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 0, 0, 0, 2, 2, 0, 2, 2},
    {4, 0, 6, 0, 6, 0, 0, 0, 0, 4, 6, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 2},
    {4, 0, 0, 5, 0, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 0, 0, 0, 2, 2, 0, 2, 2},
    {4, 0, 6, 0, 6, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 5, 0, 0, 2, 0, 0, 0, 2},
    {4, 0, 0, 0, 0, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2},
    {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3}};

Plinth::Plinth(game& game)
    : scene {game}
    , _cache {std::make_unique<gfx_cache>()}
{
    _material->first_pass().Texture = _texture;

    _texture->resize(_cache->screen_size(), 1, gfx::texture::format::RGBA8);
    _texture->Filtering = gfx::texture::filtering::Linear;

    _screenWidth  = _texture->info().Size.Width;
    _screenHeight = _texture->info().Size.Height;
    _texWidth     = _cache->tex_size().Width;
    _texHeight    = _cache->tex_size().Height;
    _texBpp       = _cache->tex_bpp();

    _zBuffer.resize(_screenWidth);

    _rowDist.resize(_screenHeight);
    for (i32 y {0}; y < _screenHeight; y++) {
        if (2.0 * y == _screenHeight) {
            _rowDist[y] = std::numeric_limits<f64>::infinity();
            continue;
        }
        _rowDist[y] = _screenHeight / ((2.0 * y) - _screenHeight);
    }

    _pos = {5, 5};
    _sprites.push_back({.Pos = {7, 4}, .Texture = sprite1Texture});

    // camera / projection setup
    f64 const fov {66.0 * TAU / 360.0}; // horizontal FOV in radians
    _dir   = {-1, 0};
    _plane = {0, std::tan(fov / 2.0)};

    _projPlaneDist = (_screenWidth / 2.0) / std::tan(fov / 2.0);
}

Plinth::~Plinth() = default;

void Plinth::on_start()
{
    _cache->load();
}

void Plinth::on_draw_to(gfx::render_target& target, transform const& xform)
{
    // aspect ratio correction
    size_i const size {*target.Size};

    f32 const srcAspect {static_cast<f32>(_screenWidth) / static_cast<f32>(_screenHeight)};
    f32 const dstAspect {static_cast<f32>(size.Width) / static_cast<f32>(size.Height)};

    f32 destWidth {static_cast<f32>(size.Width)};
    f32 destHeight {static_cast<f32>(size.Height)};

    if (dstAspect > srcAspect) {
        destWidth = destHeight * srcAspect;
    } else {
        destHeight = destWidth / srcAspect;
    }

    f32 const offsetX {(static_cast<f32>(size.Width) - destWidth) / 2.0f};
    f32 const offsetY {(static_cast<f32>(size.Height) - destHeight) / 2.0f};

    gfx::quad q {};
    gfx::geometry::set_color(q, colors::White);
    gfx::geometry::set_position(q, {offsetX, offsetY, destWidth, destHeight});
    gfx::geometry::set_texcoords(q, {.UVRect = gfx::render_texture::UVRect(), .Level = 0});
    _renderer.set_geometry({.Vertices = q, .Indices = gfx::QuadIndicies, .Type = gfx::primitive_type::Triangles}, &_material->first_pass());
    _renderer.render_to_target(target, transform::Identity);
}

void Plinth::on_fixed_update(milliseconds deltaTime)
{
    auto const& stats {locate_service<gfx::render_system>().statistics()};
    auto const& mouse {locate_service<input::system>().mouse().get_position()};
    window().Title = std::format("Plinth | FPS avg:{:.2f} best:{:.2f} worst:{:.2f} | x:{} y:{} ",
                                 stats.average_FPS(), stats.best_FPS(), stats.worst_FPS(),
                                 mouse.X, mouse.Y);
}

void Plinth::on_update(milliseconds deltaTime)
{
    _update = move(deltaTime) || _update;

    if (_update) {
        draw();
        _update = false;
        _texture->update_data(_cache->screen(), 0);
    }
}

void Plinth::draw()
{
    locate_service<task_manager>().run_parallel(
        [&](par_task const& ctx) {
            for (isize x {ctx.Start}; x < ctx.End; x++) {
                cast(static_cast<i32>(x));
            }
        },
        _screenWidth);

    draw_sprites();
}

void Plinth::cast(i32 x)
{
    // WALL CASTING
    // calculate ray position and direction
    f64 const     cameraX {(2.0 * x / _screenWidth) - 1.0}; // x-coordinate in camera space
    point_d const rayDir {_dir + (_plane * cameraX)};

    // which box of the map we're in
    point_i map {static_cast<point_i>(_pos)};

    // length of ray from one x or y-side to next x or y-side
    point_d const deltaDist {(rayDir.X == 0) ? 1e30 : std::abs(1 / rayDir.X), (rayDir.Y == 0) ? 1e30 : std::abs(1 / rayDir.Y)};

    // what direction to step in x or y-direction (either +1 or -1)
    point_i step {};

    bool side {false}; // was a NS or a EW wall hit?

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
        if (levelMap[map] > 0) {
            break;
        }
    }

    // Calculate distance projected on camera direction (Euclidean distance would give fisheye effect!)
    f64 const perpWallDist {!side ? sideDist.X - deltaDist.X : sideDist.Y - deltaDist.Y};

    _zBuffer[x] = perpWallDist;

    // Calculate height of line to draw on screen
    i32 const lineHeight {static_cast<i32>(_projPlaneDist / perpWallDist)};

    // calculate lowest and highest pixel to fill in current stripe
    i32 const drawStart {std::max((-lineHeight / 2) + (_screenHeight / 2), 0)};
    i32 const drawEnd {std::min((lineHeight / 2) + (_screenHeight / 2), _screenHeight - 1)};

    // calculate value of wallX
    f64 wallX {!side ? _pos.Y + (perpWallDist * rayDir.Y) : _pos.X + (perpWallDist * rayDir.X)}; // where exactly the wall was hit
    wallX -= std::floor(wallX);
    u32* const screenBuf {_cache->screen()};
    {                                                                                            // texturing calculations
        i32 const   texNum {levelMap[map] - 1};                                                  // 1 subtracted from it so that texture 0 can be used!
        auto const* tex {_cache->texture(texNum)};

        // x coordinate on the texture
        i32 texX {static_cast<i32>(wallX * static_cast<f64>(_texWidth))};
        if ((!side && rayDir.X > 0) || (side && rayDir.Y < 0)) {
            texX = _texWidth - texX - 1;
        }

        // How much to increase the texture coordinate per screen pixel
        f64 const texStep {1.0 * _texHeight / lineHeight};
        // Starting texture coordinate
        f64       texPos {(drawStart - (_screenHeight / 2) + (lineHeight / 2)) * texStep};
        for (i32 y {drawStart}; y < drawEnd; y++) {
            // Cast the texture coordinate to integer, and mask with (cache::TexSize.Height - 1) in case of overflow
            i32 const texY {static_cast<i32>(texPos) & (_texHeight - 1)};
            texPos += texStep;
            cache_base::copy(screenBuf + x + ((_screenHeight - y - 1) * _screenWidth), tex, (texX + (texY * _texWidth)) * _texBpp);
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
    f64 const   invPerpWallDist {1.0 / perpWallDist};
    auto const* floorTex {_cache->texture(floorTexture)};
    auto const* ceilTex {_cache->texture(ceilingTexture)};
    for (i32 y {drawEnd}; y < _screenHeight; y++) {
        f64 const weight {std::min(_rowDist[y] * invPerpWallDist, 1.0)};

        point_d const currentFloor {(weight * floorWall.X) + ((1.0 - weight) * _pos.X), (weight * floorWall.Y) + ((1.0 - weight) * _pos.Y)};

        i32 const texelX {static_cast<i32>(currentFloor.X * _texWidth) & (_texWidth - 1)};
        i32 const texelY {static_cast<i32>(currentFloor.Y * _texHeight) & (_texHeight - 1)};
        i32 const texelOffset {(texelX + (texelY * _texWidth)) * _texBpp};

        cache_base::copy(screenBuf + x + ((_screenHeight - y - 1) * _screenWidth), floorTex, texelOffset);
        cache_base::copy(screenBuf + x + (y * _screenWidth), ceilTex, texelOffset);
    }
}

void Plinth::draw_sprites()
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

    u32* const screenBuf {_cache->screen()};

    // camera plane determinant, used to "unrotate" sprite position into camera space
    f64 const invDet {1.0 / ((_plane.X * _dir.Y) - (_dir.X * _plane.Y))};

    for (isize idx : order) {
        sprite const& spr {_sprites[idx]};

        // translate sprite position relative to camera
        point_d const relPos {spr.Pos - _pos};

        // transform with the inverse camera matrix
        //   [ planeX   dirX ] -1                                       [ dirY      -dirX ]
        //   [ planeY   dirY ]       =  1/(planeX*dirY-dirX*planeY) *   [ -planeY  planeX ]
        f64 const transformX {invDet * ((_dir.Y * relPos.X) - (_dir.X * relPos.Y))};
        f64 const transformY {invDet * ((-_plane.Y * relPos.X) + (_plane.X * relPos.Y))}; // depth inside screen

        if (transformY <= 0) {
            continue;                                                                     // behind camera
        }

        i32 const spriteScreenX {static_cast<i32>((_screenWidth / 2.0) * (1.0 + (transformX / transformY)))};

        // size of "one world unit" in screen pixels at this depth
        f64 const scale {_projPlaneDist / transformY};

        i32 const spriteHeight {static_cast<i32>(std::abs(scale * spr.Size.Height))};
        i32 const spriteWidth {static_cast<i32>(std::abs(scale * spr.Size.Width))};

        i32 const drawStartY {std::max((-spriteHeight / 2) + (_screenHeight / 2), 0)};
        i32 const drawEndY {std::min((spriteHeight / 2) + (_screenHeight / 2), _screenHeight - 1)};
        i32 const drawStartX {std::max((-spriteWidth / 2) + spriteScreenX, 0)};
        i32 const drawEndX {std::min((spriteWidth / 2) + spriteScreenX, _screenWidth - 1)};

        auto const* tex {_cache->texture(spr.Texture)};

        for (i32 stripe {drawStartX}; stripe < drawEndX; ++stripe) {
            i32 const texX {static_cast<i32>(((stripe - ((-spriteWidth / 2) + spriteScreenX)) * _texWidth) / spriteWidth)};

            // only draw if in front of camera and in front of the wall z-buffer for this column
            if (stripe < 0 || stripe >= _screenWidth || transformY >= _zBuffer[stripe]) {
                continue;
            }

            f64 const texStep {1.0 * _texHeight / spriteHeight};
            f64       texPos {(drawStartY - ((-spriteHeight / 2) + (_screenHeight / 2))) * texStep};

            for (i32 y {drawStartY}; y < drawEndY; ++y) {
                i32 const texY {static_cast<i32>(texPos) & (_texHeight - 1)};
                texPos += texStep;

                i32 const offset {(texX + (texY * _texWidth)) * _texBpp};

                // skip transparent texels — assumes alpha channel at byte offset 3
                if (tex[offset + 3] == 0) {
                    continue;
                }

                cache_base::copy(screenBuf + stripe + ((_screenHeight - y - 1) * _screenWidth), tex, offset);
            }
        }
    }
}

auto Plinth::move(milliseconds deltaTime) -> bool
{
    bool retValue {false};

    f64 const moveSpeed {deltaTime.count() / 1000 * 4.0}; // the constant value is in squares/second
    f64 const rotSpeed {deltaTime.count() / 1000 * 3.0};  // the constant value is in radians/second

    auto const is_position_clear {[&](point_d pos) -> bool {
        i32 const tileX {static_cast<i32>(pos.X)};
        i32 const tileY {static_cast<i32>(pos.Y)};

        if (tileX < 0 || tileX >= mapWidth || tileY < 0 || tileY >= mapHeight) {
            return false;
        }

        return (levelMap[tileX, tileY] == 0);
    }};

    auto const& keyboard {locate_service<input::system>().keyboard()};
    f64 const   margin {0.4}; // extra margin for checking

    // Forward/Backward
    i32 forwardDir {0};
    if (keyboard.is_key_down(input::scan_code::W) || keyboard.is_key_down(input::scan_code::UP)) { forwardDir += 1; }
    if (keyboard.is_key_down(input::scan_code::S) || keyboard.is_key_down(input::scan_code::DOWN)) { forwardDir -= 1; }

    // Move Forward/Backward
    if (forwardDir != 0) {
        f64 const     moveAmount {forwardDir * moveSpeed};
        point_d const newPos {_pos + (_dir * moveAmount)};
        point_d const checkPos {_pos + (_dir * (moveSpeed + margin) * forwardDir)};

        if (is_position_clear(checkPos)) {
            _pos     = newPos;
            retValue = true;
        } else {
            if (is_position_clear({_pos.X, checkPos.Y})) {
                _pos.Y   = newPos.Y;
                retValue = true;
            } else if (is_position_clear({checkPos.X, _pos.Y})) {
                _pos.X   = newPos.X;
                retValue = true;
            }
        }
    }

    // Strafe Left/Right
    i32 strafeDir {0};
    if (keyboard.is_key_down(input::scan_code::D)) { strafeDir -= 1; }
    if (keyboard.is_key_down(input::scan_code::A)) { strafeDir += 1; }

    // Strafe Left/Right (perpendicular to _dir)
    if (strafeDir != 0) {
        // Perpendicular vector to (_dir.X, _dir.Y) is (-_dir.Y, _dir.X)
        point_d const strafe {_dir.as_perpendicular()};

        f64 const     moveAmount {strafeDir * moveSpeed};
        point_d const newPos {_pos + (strafe * moveAmount)};
        point_d const checkPos {_pos + (strafe * (moveSpeed + margin) * strafeDir)};

        if (is_position_clear(checkPos)) {
            _pos     = newPos;
            retValue = true;
        } else {
            if (is_position_clear({_pos.X, checkPos.Y})) {
                _pos.Y   = newPos.Y;
                retValue = true;
            } else if (is_position_clear({checkPos.X, _pos.Y})) {
                _pos.X   = newPos.X;
                retValue = true;
            }
        }
    }

    i32 rotateDir {0};
    if (keyboard.is_key_down(input::scan_code::LEFT)) { rotateDir += 1; }
    if (keyboard.is_key_down(input::scan_code::RIGHT)) { rotateDir -= 1; }

    // rotate
    if (rotateDir != 0) {
        f64 const rotateAmount {rotateDir * rotSpeed};

        // both camera direction and camera plane must be rotated
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

void Plinth::on_key_down(input::keyboard::event const& ev)
{
    switch (ev.ScanCode) { // NOLINT
    case input::scan_code::BACKSPACE:
        parent().pop_current_scene();
        break;
    default:

        break;
    }
}
