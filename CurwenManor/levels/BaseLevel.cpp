// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "BaseLevel.hpp"

#include "../GameCanvas.hpp"
#include "../assets/GameAssets.hpp"
#include "../scenes/GameScene.hpp"

namespace stn {

base_level::base_level(game_scene* parent)
    : _parent {parent}
{
    _parent->get_player().Turn.connect<&base_level::on_turn>(this);
}

void base_level::update(milliseconds deltaTime)
{
    _currentTime += deltaTime;
}

void base_level::draw(canvas& canvas)
{
    if (_currentEvent.State == level_event_status::Visible) {
        _currentEvent.Draw(canvas, _currentEvent.Step);
    }
}

auto base_level::get_parent() -> game_scene&
{
    return *_parent;
}

void base_level::draw_text(canvas& canvas, i32 level, i32 step)
{
    auto& ctx {canvas.get_context()};
    auto& assets {_parent->get_assets()};

    auto const& texts {_parent->get_assets().get_cutscene_texts()};
    std::string text;
    if (!texts.try_get(text, std::to_string(level), std::to_string(step))) {
        return;
    }

    //  canvas.draw_image("cutscenes0", "default", {point_f::Zero, CANVAS_SIZE_F});
    ctx.set_fill_style(COLOR0);
    ctx.begin_path();
    ctx.rect({{0, 0}, {CANVAS_SIZE_F.Width, TILE_SIZE_F.Height}});
    ctx.fill();

    ctx.set_fill_style(COLOR3);
    ctx.set_font(assets.get_default_font());
    ctx.draw_textbox({{15, 0}, CANVAS_SIZE_F}, text);
}

auto base_level::is_event_running() const -> bool
{
    return _currentEvent.State != level_event_status::Finished;
}

void base_level::set_event(level_event_func const& event, level_event_draw const& draw)
{
    _currentTime        = 0ms;
    _currentEvent.State = level_event_status::Hidden;
    _currentEvent.Step  = -1;
    _currentEvent.Func  = event;
    _currentEvent.Draw  = draw;

    _parent->request_draw();
}

void base_level::on_turn()
{
    if (is_event_running()) {
        _currentEvent.State = _currentEvent.Func(*_parent, ++_currentEvent.Step);
        switch (_currentEvent.State) {
        case level_event_status::Hidden:
            --_currentEvent.Step;
            break;
        case level_event_status::Visible:
            get_parent().request_draw();
            break;
        case level_event_status::Finished:
            on_event_finished(++_numEvents);
            break;
        }
    }
}

}
