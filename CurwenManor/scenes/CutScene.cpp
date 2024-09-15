// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "CutScene.hpp"

#include <utility>

namespace stn {

constexpr auto BREAK {"-break-"};

cut_scene::cut_scene(i32 number, game& game, std::shared_ptr<canvas> canvas, std::shared_ptr<assets> assets)
    : base_scene {game, std::move(canvas), std::move(assets)}
    , _number {number}
{
}

cut_scene::~cut_scene()
{
}

void cut_scene::on_start()
{
    _canvasDraw = get_canvas().Draw.connect([&](gfx::canvas& canvas) { on_canvas_draw(canvas); });
}

void cut_scene::on_update(milliseconds deltaTime)
{
    _inputTimeout -= deltaTime;
    base_scene::on_update(deltaTime);
}

void cut_scene::on_controller_button_down(input::controller::button_event& ev)
{
    if (_inputTimeout.count() > 0) { return; }

    _inputTimeout = 250ms;
    if (ev.Button == input::controller::button::Start) {
        next_step();
        _step = 99999;
    } else if (ev.Button == input::controller::button::A) {
        next_step();
    }
}

void cut_scene::on_controller_button_up(input::controller::button_event&)
{
}

void cut_scene::on_canvas_draw(gfx::canvas& canvas)
{
    std::string text;
    if (!get_assets().get_cutscene_texts().try_get(text, std::to_string(_number), std::to_string(_step))) {
        fade_out(false, []() {});
        text = _lastText;
    } else {
        if (text == BREAK) {
            _lastText = "";
            ++_step;
            get_assets().get_cutscene_texts().try_get(text, std::to_string(_number), std::to_string(_step));
        }

        text = _lastText + "\n\n" + text;
    }

    canvas.begin_path();
    canvas.set_stroke_style(COLOR3);
    canvas.set_stroke_width(5);
    canvas.rounded_rect({{5, 5}, CANVAS_SIZE_F - size_f {10, 10}}, 5);
    canvas.stroke();

    canvas.set_fill_style(COLOR3);
    canvas.set_font(get_assets().get_default_font());
    canvas.draw_textbox({{8, 10}, CANVAS_SIZE_F - size_f {15, 15}}, text);
    canvas.draw_textbox({{142, 125}, size_f {15, 15}}, "A");
    _lastText = text;
}

void cut_scene::next_step()
{
    if (is_fading()) { return; }

    get_canvas().request_draw();
    ++_step;
}
}
