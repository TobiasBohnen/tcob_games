// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

namespace solitaire {

////////////////////////////////////////////////////////////

class card_table;

class foreground_canvas {
public:
    foreground_canvas(card_table& parent, gfx::window* window, gfx::ui::canvas_widget* canvas, assets::group& resGrp);

    void show_next_hint();

    void draw();
    void update(milliseconds deltaTime);

    void disable_hint();
    void mark_dirty();

private:
    void draw_hint();
    void draw_state();

    card_table&             _parent;
    gfx::window*            _window;
    gfx::ui::canvas_widget* _canvas;
    assets::group&          _resGrp;

    isize      _currentHint {-1};
    bool       _showHint {false};
    timer      _hintTimer;
    bool       _canvasDirty {true};
    game_state _lastState {game_state::Initial};
};

}
