// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"
#include "actors/Player.hpp"
#include "dungeon/Dungeon.hpp"
#include "ui/Renderer.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

class master_control {
public:
    master_control();

    void draw(ui::terminal& term);
    void update(milliseconds deltaTime, action_queue& queue);

    auto current_dungeon() const -> dungeon const&;

    void log(string const& message);

private:
    void set_view_center(point_i pos);

    void mark_dirty();

    void end_turn();

    void handle_action_queue(action_queue& queue);

    void do_execute();

    void do_pickup();
    auto do_move(point_i target) -> bool;
    void do_look(point_i target);
    auto do_interact(point_i target, bool failMessage) -> bool;

    std::vector<dungeon> _dungeons {};
    usize                _currentDungeon {0};

    player  _player {*this};
    point_i _viewCenter;

    std::vector<log_message> _log;

    mode _mode {mode::Move};

    mfd_mode _mfdMode {mfd_mode::Character};

    bool     _redraw {true};
    renderer _renderer {};

    i32 _turn {0};
};

}
