// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"
#include "Player.hpp"
#include "Renderer.hpp"
#include "level/Level.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

class master_control {
public:
    master_control();

    void draw(ui::terminal& term);
    void update(milliseconds deltaTime, action_queue& queue);

private:
    void set_view_center(point_i pos);

    void mark_dirty();

    void log(string const& message);
    void end_turn();

    void handle_animation(milliseconds deltaTime);
    void handle_action_queue(action_queue& queue);

    void do_execute();

    level  _level {};
    player _player {};

    point_i _viewCenter;

    std::vector<log_message> _log;

    enum class mode {
        Move,
        Look,
        Target
    };
    mode _mode {mode::Move};

    animation_func _animation;         // TODO: MOVE
    milliseconds   _animationTimer {}; // TODO: MOVE

    bool     _redraw {true};
    renderer _renderer {};
};

}
