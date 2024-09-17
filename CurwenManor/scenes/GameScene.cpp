// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GameScene.hpp"

#include "CutScene.hpp"
#include <utility>

#include "../levels/BaseLevel.hpp"
#include "../levels/Level1.hpp"

namespace stn {

/////////////////////////////////////////////////////

game_scene::game_scene(game& game, std::shared_ptr<canvas> canvas, std::shared_ptr<assets> assets)
    : base_scene {game, std::move(canvas), std::move(assets)}
    , _tileMap {get_assets()}
    , _player {this}
{
}

game_scene::~game_scene()
{
}

auto game_scene::get_map() -> tilemap&
{
    return _tileMap;
}

auto game_scene::get_player() -> player&
{
    return _player;
}

void game_scene::on_start()
{
    push_scene<cut_scene>(0);
}

void game_scene::on_wake_up()
{
    _canvasDraw   = connect_draw([&](canvas& canvas) { on_canvas_draw(canvas); });
    _currentLevel = std::make_shared<level1>(this);
    request_draw();
}

void game_scene::on_update(milliseconds deltaTime)
{
    if (_currentLevel) { _currentLevel->update(deltaTime); }

    _player.update(deltaTime);
    _tileMap.set_offset(_player.get_position());

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

void game_scene::on_canvas_draw(canvas& canvas)
{
    canvas.begin_draw(COLOR0);
    _tileMap.draw(canvas);
    canvas.end_draw();

    canvas.begin_draw(COLOR0);
    _tileMap.draw_shadow(canvas, _player);
    _player.draw(canvas, _tileMap.get_offset());

    if (_currentLevel) { _currentLevel->draw(canvas); }

    canvas.end_draw();

    // canvas.snap();
}

}
