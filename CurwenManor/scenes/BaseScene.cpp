// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "BaseScene.hpp"

namespace stn {

base_scene::base_scene(game& game, std::shared_ptr<canvas> canvas, std::shared_ptr<assets> assets)
    : scene(game)
    , _canvas {std::move(canvas)}
    , _assets {std::move(assets)}
{
}

void base_scene::request_draw()
{
    _canvas->request_draw();
}

void base_scene::on_draw_to(gfx::render_target& target)
{
    _canvas->draw_to(target);
}

void base_scene::on_update(milliseconds deltaTime)
{
    if (_fadeOut) { _fadeOut->update(deltaTime); }
}

void base_scene::on_fixed_update(milliseconds deltaTime)
{
#if defined(TCOB_DEBUG)
    auto const& stats {locate_service<gfx::render_system>().get_stats()};
    get_window().Title = std::format("GBJAM12 | avg FPS: {:.2f} best FPS: {:.2f} worst FPS: {:.2f} | x:{};y:{}",
                                     stats.get_average_FPS(), stats.get_best_FPS(), stats.get_worst_FPS(),
                                     input::system::GetMousePosition().X / 4, input::system::GetMousePosition().Y / 4);
#endif
}

void base_scene::on_key_down(input::keyboard::event& ev)
{
    switch (ev.ScanCode) {
    case input::scan_code::BACKSPACE:
        get_game().pop_current_scene();
        break;
    case input::scan_code::S: {
        auto _ = locate_service<gfx::render_system>().get_window().copy_to_image().save("screen.png");
    } break;
    default:
        break;
    }
}

auto base_scene::get_assets() -> assets&
{
    return *_assets;
}

auto base_scene::is_fading() const -> bool
{
    return _fadeOut != nullptr;
}

}
