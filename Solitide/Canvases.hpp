// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"      // IWYU pragma: keep

#include "games/Games.hpp" // IWYU pragma: keep

namespace solitaire {

////////////////////////////////////////////////////////////

class foreground_canvas {
public:
    foreground_canvas(card_table& parent, assets::group& resGrp);

    void draw(gfx::render_target& target);
    void update(milliseconds deltaTime);

    void show_hint();
    void disable_hint();

    void mark_dirty();

private:
    void draw_hint(gfx::render_target& target);
    void draw_state();

    card_table&    _parent;
    assets::group& _resGrp;

    isize _currentHint {-1};
    bool  _showHint {false};
    timer _hintTimer;

    bool                 _canvasDirty {true};
    gfx::canvas          _canvas;
    rect_i               _bounds;
    gfx::canvas_renderer _renderer;

    game_status _lastStatus {game_status::Initial};
};

////////////////////////////////////////////////////////////

class background_canvas {
public:
    background_canvas(card_table& parent, assets::group& resGrp);

    void draw(gfx::render_target& target);
    void update(milliseconds deltaTime);

    void set_background_colors(color a, color b);
    void mark_dirty();

private:
    card_table&    _parent;
    assets::group& _resGrp;

    bool                 _canvasDirty {true};
    gfx::canvas          _canvas;
    rect_i               _bounds;
    gfx::canvas_renderer _renderer;

    color _colorA;
    color _colorB;
};

}
