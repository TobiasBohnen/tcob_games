// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp" // IWYU pragma: keep
#include "../Player.hpp"
#include "../Tiles.hpp"
#include "BaseScene.hpp"

namespace stn {

/////////////////////////////////////////////////////

class game_scene : public base_scene {
public:
    game_scene(game& game, std::shared_ptr<canvas> canvas, std::shared_ptr<assets> assets);
    ~game_scene() override;

    auto get_map() -> tilemap&;
    auto get_player() -> player&;

protected:
    void on_start() override;
    void on_wake_up() override;

    void on_update(milliseconds deltaTime) override;

    void on_controller_button_down(input::controller::button_event&) override;
    void on_controller_button_up(input::controller::button_event&) override;

private:
    void on_canvas_draw(canvas& canvas);

    input::controller::button _downButton {};
    milliseconds              _downButtonTimer {};

    tilemap _tileMap;
    player  _player;

    scoped_connection _canvasDraw;

    std::shared_ptr<base_level> _currentLevel;
};

}
