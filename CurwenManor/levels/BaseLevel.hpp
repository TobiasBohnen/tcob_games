// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp" // IWYU pragma: keep

namespace stn {

enum class level_event_status {
    Hidden,
    Visible,
    Finished
};

using level_event_func = std::function<level_event_status(game_scene&, i32)>;
using level_event_draw = std::function<void(canvas&, i32)>;

struct level_event {
    i32                Step {-1};
    level_event_status State {level_event_status::Hidden};

    level_event_func Func;
    level_event_draw Draw;
};

/////////////////////////////////////////////////////

class base_level {
public:
    base_level(game_scene* parent);
    virtual ~base_level() = default;

    void virtual update(milliseconds deltaTime);

    void virtual draw(canvas& canvas);

protected:
    auto get_parent() -> game_scene&;
    void draw_text(canvas& canvas, i32 level, i32 step);

    auto is_event_running() const -> bool;
    void set_event(level_event_func const& event, level_event_draw const& draw);
    void virtual on_event_finished(i32 numEvents) = 0;

private:
    void on_turn();

    game_scene* _parent;

    milliseconds _currentTime {0};

    level_event _currentEvent;
    i32         _numEvents {0};
};

}
