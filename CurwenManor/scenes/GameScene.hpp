// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp" // IWYU pragma: keep
#include "BaseScene.hpp"

namespace stn {

class game_scene : public base_scene {

public:
    game_scene(game& game, std::shared_ptr<canvas> canvas, std::shared_ptr<assets> assets);
    ~game_scene() override;

protected:
    void on_start() override;
    void on_wake_up() override;

    void on_controller_button_down(input::controller::button_event&) override;
    void on_controller_button_up(input::controller::button_event&) override;

private:
    void on_canvas_draw(gfx::canvas& canvas);
};

}
