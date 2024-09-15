// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

namespace stn {

/////////////////////////////////////////////////////

class canvas {
public:
    canvas();

    signal<gfx::canvas> Draw;

    void draw_to(gfx::render_target& target);

    void fade_to_white();
    void fade_to_black();

    void request_draw();

private:
    void begin_draw();
    void end_draw();

    gfx::canvas          _canvas;
    gfx::canvas_renderer _renderer {_canvas};
    bool                 _canvasDirty {false};

    gfx::image   _lastFrame;
    gfx::texture _scratchTex;
};

}
