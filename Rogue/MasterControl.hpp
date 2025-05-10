// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"
#include "actors/Player.hpp"
#include "dungeon/Dungeon.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

class master_control {
public:
    master_control();

    void draw(renderer& renderer);
    void update(milliseconds deltaTime, action_queue& queue);

    auto current_dungeon() -> dungeon&;

    auto rand() -> f32;

private:
    void log(log_message const& message);

    void set_view_center(point_i pos);
    void set_mfd_mode(mfd_mode mode);

    void mark_dirty();

    void end_turn();

    void handle_action_queue(action_queue& queue);

    void do_get(point_i target);
    auto do_move(point_i target) -> bool;
    void do_look(point_i target);
    auto do_interact(point_i target, bool failMessage) -> bool;

    void draw_log(renderer& renderer);
    void draw_mfd(renderer& renderer);
    void draw_detail(renderer& renderer);

    std::vector<dungeon> _dungeons {};
    usize                _currentDungeon {0};

    std::unique_ptr<player> _player;

    point_i _viewCenter;

    using log_entry = std::pair<string, i32>;
    std::vector<log_entry> _log;

    mode     _mode {mode::Move};
    mfd_mode _mfdMode {mfd_mode::Character};

    bool _redraw {true};
    bool _newTurn {true};
    i32  _turn {0};

    rng _rng;
};

}
