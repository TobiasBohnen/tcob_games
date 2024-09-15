// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GameScene.hpp"

#include "CutScene.hpp"

#include <utility>

namespace stn {

/////////////////////////////////////////////////////

game_scene::game_scene(game& game, std::shared_ptr<canvas> canvas, std::shared_ptr<assets> assets)
    : base_scene {game, std::move(canvas), std::move(assets)}
    , _tileMap {get_assets()}
    , _player {_tileMap, get_canvas(), get_assets()}
{
}

game_scene::~game_scene()
{
}

void game_scene::on_start()
{
    push_scene<cut_scene>(0);
}

void game_scene::on_wake_up()
{
    auto& canvas {get_canvas()};
    canvas.Draw.connect([&](gfx::canvas& canvas) { on_canvas_draw(canvas); });
    canvas.request_draw();
}

void game_scene::on_update(milliseconds deltaTime)
{
    _player.update(deltaTime);

    _downButtonTimer += deltaTime;
    if (_downButtonTimer > 300ms) {
        input::controller::button_event ev;
        ev.Controller = locate_service<input::system>().get_controller(0);
        ev.Button     = _downButton;
        ev.Pressed    = true;
        on_controller_button_down(ev);
    }

    base_scene::on_update(deltaTime);
}

void game_scene::on_controller_button_down(input::controller::button_event& ev)
{
    switch (ev.Button) {
    case input::controller::button::DPadUp: _player.move(direction::Up, ev.Controller->is_button_pressed(input::controller::button::A)); break;
    case input::controller::button::DPadDown: _player.move(direction::Down, ev.Controller->is_button_pressed(input::controller::button::A)); break;
    case input::controller::button::DPadLeft: _player.move(direction::Left, ev.Controller->is_button_pressed(input::controller::button::A)); break;
    case input::controller::button::DPadRight: _player.move(direction::Right, ev.Controller->is_button_pressed(input::controller::button::A)); break;

    case input::controller::button::A:
    case input::controller::button::B:
    case input::controller::button::Start:
    case input::controller::button::Back:
        break;
    default: break;
    }

    _downButton      = ev.Button;
    _downButtonTimer = 0ms;
}

void game_scene::on_controller_button_up(input::controller::button_event& ev)
{
    if (ev.Button == _downButton) {
        _downButton = input::controller::button::Invalid;
    }
}

void game_scene::on_canvas_draw(gfx::canvas& canvas)
{
    canvas.set_fill_style(colors::White);

    _tileMap.draw(canvas);
    _player.draw(canvas);
}

}
