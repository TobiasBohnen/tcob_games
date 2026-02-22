// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

namespace solitaire {

////////////////////////////////////////////////////////////

class start_scene : public scene {
public:
    start_scene(game& game);

protected:
    void on_start() override;

    void on_draw_to(gfx::render_target& target, transform& xform) override;

    void on_update(milliseconds deltaTime) override;

private:
    bool                 _canvasDirty {true};
    gfx::canvas          _canvas;
    gfx::canvas_renderer _renderer;

    bool   _drawn {false};
    size_i _windowSize;
};

}
