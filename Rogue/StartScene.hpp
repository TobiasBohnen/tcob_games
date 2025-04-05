// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"
#include "Level.hpp"
#include "MainMenu.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

class start_scene : public scene {

public:
    start_scene(game& game);
    ~start_scene() override;

protected:
    void on_start() override;

    void on_draw_to(gfx::render_target& target) override;

    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_key_down(input::keyboard::event const& ev) override;

private:
    std::shared_ptr<main_menu> _mainForm {};
    action_queue               _actionQueue;

    level _level;
};

}
