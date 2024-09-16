// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp" // IWYU pragma: keep
#include "BaseScene.hpp"

namespace stn {

/////////////////////////////////////////////////////

class start_scene : public base_scene {
public:
    start_scene(game& game, std::shared_ptr<canvas> canvas, std::shared_ptr<assets> assets);
    ~start_scene() override;

protected:
    void on_start() override;

    void on_update(milliseconds deltaTime) override;

    void on_controller_button_down(input::controller::button_event&) override;
    void on_controller_button_up(input::controller::button_event&) override;

private:
    void on_canvas_draw(canvas& canvas);

    std::unique_ptr<gfx::frame_animation_tween> _titleAnimation;

    scoped_connection _canvasDraw;
};

}
