// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Plinth.hpp"

#include "Common.hpp"
#include "Level.hpp"
#include "Raycaster.hpp"

constexpr size_i screenSize {640, 480};

Plinth::Plinth(game& game)
    : scene {game}
    , _cache {std::make_unique<cache>(screenSize)}
{
    _material->first_pass().Texture = _texture;

    _texture->resize(screenSize, 1, gfx::texture::format::RGBA8);
    _texture->Filtering = gfx::texture::filtering::NearestNeighbor;

    _screen    = _texture->info().Size;
    _raycaster = std::make_unique<raycaster>(*_cache, _screen);
    _level     = std::make_unique<level>();

    _player.Pos = {6, 5};
    f64 const fov {66.0 * TAU / 360.0};
    _player.Direction     = {-1, 0};
    _player.Plane         = {0, std::tan(fov / 2.0)};
    _player.ProjPlaneDist = (screenSize.Width / 2.0) / std::tan(fov / 2.0);
}

Plinth::~Plinth() = default;

void Plinth::on_start()
{
    _cache->load();
}

void Plinth::on_draw_to(gfx::render_target& target, transform const& xform)
{
    if (_update) {
        draw();
        _update = false;
        _texture->update_data(_cache->screen(), 0);
    }

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
    _update = _level->update(deltaTime) || _update;
}

void Plinth::draw()
{
    _raycaster->prepare_draw();

    locate_service<task_manager>().run_parallel(
        [&](par_task const& ctx) {
            _raycaster->draw(*_level, _player, _cache->screen(), static_cast<i32>(ctx.Start), static_cast<i32>(ctx.End));
        },
        _screen.Width);
}

auto Plinth::move(milliseconds deltaTime) -> bool
{
    auto const& input {locate_service<input::system>()};

    f64 const moveSpeed {deltaTime.count() / 1000 * 4.0}; // squares/second
    f64 const rotSpeed {deltaTime.count() / 1000 * 3.0};  // radians/second

    f64 forwardAmount {0};
    f64 strafeAmount {0};
    f64 rotateAmount {0};

    if (input.InputMode == input::mode::KeyboardMouse) {
        auto const& keyboard {input.keyboard()}; // TODO: remap in ui

        if (keyboard.is_key_down(input::scan_code::W) || keyboard.is_key_down(input::scan_code::UP)) { forwardAmount += moveSpeed; }
        if (keyboard.is_key_down(input::scan_code::S) || keyboard.is_key_down(input::scan_code::DOWN)) { forwardAmount -= moveSpeed; }

        if (keyboard.is_key_down(input::scan_code::D)) { strafeAmount -= moveSpeed; }
        if (keyboard.is_key_down(input::scan_code::A)) { strafeAmount += moveSpeed; }

        if (keyboard.is_key_down(input::scan_code::LEFT)) { rotateAmount += rotSpeed; }
        if (keyboard.is_key_down(input::scan_code::RIGHT)) { rotateAmount -= rotSpeed; }
    } else {
        auto const& controller {input.first_controller()}; // TODO: controller select in ui

        if (controller.is_button_pressed(input::controller::button::DPadUp)) { forwardAmount += moveSpeed; }
        if (controller.is_button_pressed(input::controller::button::DPadDown)) { forwardAmount -= moveSpeed; }

        if (controller.is_button_pressed(input::controller::button::DPadLeft)) { strafeAmount += moveSpeed; }
        if (controller.is_button_pressed(input::controller::button::DPadRight)) { strafeAmount -= moveSpeed; }

        constexpr f64 deadzone {5000.0}; // TODO: controller deadzone in ui
        constexpr f64 maxAxisValue {32768.0};

        auto const leftY {controller.get_axis_value(input::controller::axis::LeftY)};
        if (std::abs(leftY) > deadzone) {
            f64 const normalizedY {leftY / maxAxisValue};
            forwardAmount += -normalizedY * moveSpeed;
        }

        auto const leftX {controller.get_axis_value(input::controller::axis::LeftX)};
        if (std::abs(leftX) > deadzone) {
            f64 const normalizedX {leftX / maxAxisValue};
            strafeAmount += -normalizedX * moveSpeed;
        }

        auto const rightX {controller.get_axis_value(input::controller::axis::RightX)};
        if (std::abs(rightX) > deadzone) {
            f64 const normalizedRightX {rightX / maxAxisValue};
            rotateAmount += -normalizedRightX * rotSpeed;
        }
    }

    return _player.move(*_level, forwardAmount, strafeAmount, rotateAmount);
}

void Plinth::on_key_down(input::keyboard::event const& ev)
{
    switch (ev.ScanCode) { // NOLINT
    case input::scan_code::BACKSPACE:
        parent().pop_current_scene();
        break;
    case input::scan_code::Q: {
        auto& spr {_level->Sprites[0]};
        spr.Facing = spr.Pos.angle_to(_player.Pos);
        spr.Pos    = spr.Pos.moved_along(degree_d {spr.Facing.Value}, 0.1);
        _update    = true;
    } break;
    case input::scan_code::R: {
        locate_service<gfx::render_system>().statistics().reset();
    } break;
    case input::scan_code::F: {
        auto const fileName {[]() {
            for (i32 i {0};; ++i) {
                auto const name {std::format("screen{:02}.png", i)};
                if (!io::exists(name)) { return name; }
            }
        }()};
        std::ignore = window().copy_to_image().save(fileName);
    } break;
    case input::scan_code::SPACE: {
        _level->toggle_wall(point_i {_player.Pos + _player.Direction});
    } break;
    default:

        break;
    }
}

void Plinth::on_controller_button_down(input::controller::button_event const& ev)
{
    switch (ev.Button) {
    case tcob::input::controller::button::A:
        _level->toggle_wall(point_i {_player.Pos + _player.Direction});
        break;
    default: break;
    }
}
