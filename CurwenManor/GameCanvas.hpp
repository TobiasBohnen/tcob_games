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
    canvas(assets& assets);

    signal<canvas> Draw;

    void begin_draw(color clearColor);
    void end_draw();

    void fade_to_white();
    void fade_to_black();

    void draw_to(gfx::render_target& target);
    void request_draw();

    void draw_image(std::string const& image, string const& region, rect_f const& rect);
    void draw_image(gfx::texture* image, string const& region, rect_f const& rect);

    auto get_context() -> gfx::canvas&;
    auto get_last_frame() -> gfx::image&;

    void snap();

private:
    assets& _assets;

    gfx::canvas          _canvas;
    gfx::canvas_renderer _renderer {_canvas};
    bool                 _canvasDirty {false};

    gfx::image _lastFrame;
};

}
