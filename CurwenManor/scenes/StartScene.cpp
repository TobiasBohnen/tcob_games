// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "StartScene.hpp"

#include <utility>

#include "GameScene.hpp"

namespace stn {

start_scene::start_scene(game& game, std::shared_ptr<canvas> canvas, std::shared_ptr<assets> assets)
    : base_scene {game, std::move(canvas), std::move(assets)}
{
    gfx::frame_animation ani {};
    ani.Frames = {
        {"title0", 1000ms},
        {"title1", 500ms},
        {"title2", 500ms},
        {"title3", 500ms},
        {"title4", 500ms},
        {"title5", 500ms},
        {"title6", 500ms},
        {"title7", 500ms},
    };
    _titleAnimation = std::make_shared<gfx::frame_animation_tween>(ani.get_duration(), ani);
    _titleAnimation->Value.Changed.connect([&] { get_canvas().request_draw(); });
}

start_scene::~start_scene() = default;

void start_scene::on_start()
{
    _titleAnimation->start();
    _canvasDraw = get_canvas().Draw.connect([&](gfx::canvas& canvas) { on_canvas_draw(canvas); });
}

void start_scene::on_update(milliseconds deltaTime)
{
    if (_titleAnimation) { _titleAnimation->update(deltaTime); }
    base_scene::on_update(deltaTime);
}

void start_scene::on_controller_button_down(input::controller::button_event& ev)
{
    if (ev.Button == input::controller::button::Start) {
        fade_out(true, [&] { push_scene<game_scene>(); });
        _titleAnimation = nullptr;
    }
}

void start_scene::on_controller_button_up(input::controller::button_event& ev)
{
}

void start_scene::on_canvas_draw(gfx::canvas& canvas)
{
    if (_titleAnimation) {
        canvas.set_fill_style(colors::White);
        canvas.draw_image(get_assets().get_texture(_titleAnimation->Value), "default", {point_f::Zero, CANVAS_SIZE_F});

        if (_titleAnimation->get_progress() >= 0.75f) {
            canvas.set_fill_style(COLOR1);
            canvas.set_font(get_assets().get_default_font());
            canvas.draw_textbox({105, 55, 200, 200}, "-start-");
        }
    }
}
}
