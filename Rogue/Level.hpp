// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"
#include "Player.hpp"
#include "Renderer.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

class level {
public:
    level();

    void draw(ui::terminal& term);
    void update(milliseconds deltaTime, action_queue& queue);

    auto is_passable(point_i pos) const -> bool;

    auto find_path(point_i target) const -> std::vector<point_i>;

    auto get_view_center() -> point_i;
    auto get_tiles() -> grid<tile>&;
    auto get_player() -> player&;
    auto get_log() const -> std::vector<string> const&;

private:
    void log_message(string const& message);
    void end_turn();

    void set_view_center(point_i pos);
    void mark_dirty();

    void handle_animation(milliseconds deltaTime);
    void handle_action_queue(action_queue& queue);

    void do_execute();

    grid<tile> _tiles;

    std::vector<std::shared_ptr<object>>  _objects;
    std::vector<std::shared_ptr<monster>> _monsters;
    player                                _player {*this};

    std::vector<string> _log;

    point_i _viewCenter;

    enum class mode {
        Move,
        Look,
        Target
    };
    mode _mode {mode::Move};

    animation_func _animation;
    milliseconds   _animationTimer {};

    bool     _redraw {true};
    renderer _renderer {*this};
};
}
