// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"
#include "Renderer.hpp"
#include "level/Level.hpp"
#include "monsters/Player.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

class master_control {
public:
    master_control();

    void draw(ui::terminal& term);
    void update(milliseconds deltaTime, action_queue& queue);

private:
    auto current_level() -> level&;

    void set_view_center(point_i pos);

    void mark_dirty();

    void log(string const& message);
    void end_turn();

    void handle_animation(milliseconds deltaTime);
    void handle_action_queue(action_queue& queue);

    void do_execute();
    void do_move(action action);
    void do_look(action action);
    void do_interact(action action);

    void add_object(std::shared_ptr<object> const& object);
    void remove_object(std::shared_ptr<object> const& object);
    auto get_target(action action, point_i pos) const -> std::optional<point_i>;

    std::vector<level> _levels {};
    usize              _currentLevel {0};

    player  _player {};
    point_i _viewCenter;

    std::vector<log_message> _log;

    enum class mode {
        Move,
        Look,
        Interact,
    };
    mode _mode {mode::Move};

    animation_func _animation;
    milliseconds   _animationTimer {};

    bool     _redraw {true};
    renderer _renderer {};
};

}
