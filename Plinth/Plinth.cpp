// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Plinth.hpp"

#include "Raycaster.hpp"

constexpr i32 mapWidth {24};
constexpr i32 mapHeight {24};

using world_map_t = static_grid<u8, mapWidth, mapHeight>;

world_map_t worldMap {
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

    _screen    = _texture->info().Size;
    _raycaster = std::make_unique<raycaster<gfx_cache, world_map_t>>(*_cache, _screen, 66.0);
    _raycaster->set_world_map(worldMap);
    _raycaster->set_player_position({5, 5});
    _raycaster->sprites().push_back({.Pos = {7, 4}, .Size = {1, 1}, .Texture = sprite1Texture});
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

    f32 const srcAspect {static_cast<f32>(_screen.Width) / static_cast<f32>(_screen.Height)};
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
                _raycaster->cast(static_cast<i32>(x), _cache->screen());
            }
        },
        _screen.Width);

    _raycaster->draw_sprites(_cache->screen());
}

auto Plinth::move(milliseconds deltaTime) -> bool
{
    f64 const moveSpeed {deltaTime.count() / 1000 * 4.0}; // squares/second
    f64 const rotSpeed {deltaTime.count() / 1000 * 3.0};  // radians/second

    auto const& keyboard {locate_service<input::system>().keyboard()};

    f64 forwardAmount {0};
    if (keyboard.is_key_down(input::scan_code::W) || keyboard.is_key_down(input::scan_code::UP)) { forwardAmount += moveSpeed; }
    if (keyboard.is_key_down(input::scan_code::S) || keyboard.is_key_down(input::scan_code::DOWN)) { forwardAmount -= moveSpeed; }

    f64 strafeAmount {0};
    if (keyboard.is_key_down(input::scan_code::D)) { strafeAmount -= moveSpeed; }
    if (keyboard.is_key_down(input::scan_code::A)) { strafeAmount += moveSpeed; }

    f64 rotateAmount {0};
    if (keyboard.is_key_down(input::scan_code::LEFT)) { rotateAmount += rotSpeed; }
    if (keyboard.is_key_down(input::scan_code::RIGHT)) { rotateAmount -= rotSpeed; }

    return _raycaster->move(forwardAmount, strafeAmount, rotateAmount);
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
