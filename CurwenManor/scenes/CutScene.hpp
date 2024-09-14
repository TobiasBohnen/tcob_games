// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp" // IWYU pragma: keep
#include "BaseScene.hpp"

namespace stn {

class cut_scene : public base_scene {

public:
    cut_scene(i32 number, game& game, std::shared_ptr<canvas> canvas, std::shared_ptr<assets> assets);
    ~cut_scene() override;

protected:
    void on_start() override;

    void on_update(milliseconds deltaTime) override;

    void on_controller_button_down(input::controller::button_event&) override;
    void on_controller_button_up(input::controller::button_event&) override;

private:
    void on_canvas_draw(gfx::canvas& canvas);
    void next_step();

    i32          _number;
    i32          _step {0};
    milliseconds _inputTimeout {};
    std::string  _lastText;

    scoped_connection _canvasDraw;
};

}
