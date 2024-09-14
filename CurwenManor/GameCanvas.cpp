// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GameCanvas.hpp"

namespace stn {

canvas::canvas()
{
    _scratchTex.create(CANVAS_SIZE, 1, gfx::texture::format::RGBA8);
    _scratchTex.add_region("default", {{0, 0, 1, 1}, 0});
}

void canvas::begin_draw()
{
    _canvas.begin_frame(CANVAS_SIZE, 1);

    _canvas.set_edge_antialias(false);
    _canvas.set_shape_antialias(false);

    _canvas.set_fill_style(COLOR0);
    _canvas.begin_path();
    _canvas.rect({point_f::Zero, CANVAS_SIZE_F});
    _canvas.fill();
}

void canvas::end_draw()
{
    _canvas.end_frame();

    _lastFrame = _canvas.get_texture(0)->copy_to_image(0);
    _lastFrame.flip_vertically();
    assert(_lastFrame.count_colors() <= 4);

    _canvasDirty = false;
}

void canvas::draw_to(gfx::render_target& target)
{
    if (_canvasDirty) {
        begin_draw();
        Draw(_canvas);
        end_draw();
    }

    _renderer.set_layer(0);
    _renderer.set_bounds({point_f::Zero, size_f {target.Size()}});
    _renderer.render_to_target(target);
}

void canvas::fade_to_white()
{
    begin_draw();

    gfx::image& img {_lastFrame};

    for (i32 y {0}; y < CANVAS_SIZE.Height; ++y) {
        for (i32 x {0}; x < CANVAS_SIZE.Width; ++x) {
            auto const col {img.get_pixel({x, y})};
            color      target {COLOR0};
            if (col == COLOR1) {
                target = COLOR0;
            } else if (col == COLOR2) {
                target = COLOR1;
            } else if (col == COLOR3) {
                target = COLOR2;
            } else if (col != COLOR0) {
                assert(false);
            }
            img.set_pixel({x, y}, target);
        }
    }

    _scratchTex.update_data(img.get_data(), 0);

    _canvas.set_fill_style(colors::White);
    _canvas.draw_image(&_scratchTex, "default", {point_f::Zero, CANVAS_SIZE_F});

    end_draw();
}

void canvas::fade_to_black()
{
    begin_draw();

    gfx::image& img {_lastFrame};

    for (i32 y {0}; y < CANVAS_SIZE.Height; ++y) {
        for (i32 x {0}; x < CANVAS_SIZE.Width; ++x) {
            auto const col {img.get_pixel({x, y})};
            color      target {COLOR3};
            if (col == COLOR0) {
                target = COLOR1;
            } else if (col == COLOR1) {
                target = COLOR2;
            } else if (col == COLOR2) {
                target = COLOR3;
            } else if (col != COLOR3) {
                assert(false);
            }
            img.set_pixel({x, y}, target);
        }
    }

    _scratchTex.update_data(img.get_data(), 0);

    _canvas.set_fill_style(colors::White);
    _canvas.draw_image(&_scratchTex, "default", {point_f::Zero, CANVAS_SIZE_F});

    end_draw();
}

void canvas::request_draw()
{
    _canvasDirty = true;
}

}