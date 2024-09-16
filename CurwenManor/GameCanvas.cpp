// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "GameCanvas.hpp"

#include "assets/GameAssets.hpp"

namespace stn {

canvas::canvas(assets& assets)
    : _assets {assets}
{
}

void canvas::begin_draw(color clearColor)
{
    _canvas.begin_frame(CANVAS_SIZE, 1);

    _canvas.set_edge_antialias(false);
    _canvas.set_shape_antialias(false);

    if (clearColor.A > 0) {
        _canvas.set_fill_style(clearColor);
        _canvas.begin_path();
        _canvas.rect({point_f::Zero, CANVAS_SIZE_F});
        _canvas.fill();
    }
}

void canvas::end_draw()
{
    _canvas.end_frame();

    _lastFrame = _canvas.get_texture(0)->copy_to_image(0);
    _lastFrame.flip_vertically();

#if defined(TCOB_DEBUG)
    if (_lastFrame.count_colors() > 4) {
        auto _ = _lastFrame.save("test.png");
        assert(false);
    }
#endif

    _canvasDirty = false;
}

void canvas::fade_to_white()
{
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

    auto* scratchTex {_assets.get_scratch_texture()};
    scratchTex->update_data(img.get_data(), 0);

    draw_image(scratchTex, "default", {point_f::Zero, CANVAS_SIZE_F});
}

void canvas::fade_to_black()
{
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

    auto* scratchTex {_assets.get_scratch_texture()};
    scratchTex->update_data(img.get_data(), 0);

    draw_image(scratchTex, "default", {point_f::Zero, CANVAS_SIZE_F});
}

void canvas::draw_to(gfx::render_target& target)
{
    if (_canvasDirty) {
        Draw(*this);
    }

    _renderer.set_layer(0);
    _renderer.set_bounds({point_f::Zero, size_f {target.Size()}});
    _renderer.render_to_target(target);
}

void canvas::request_draw()
{
    _canvasDirty = true;
}

void canvas::draw_image(std::string const& image, string const& region, rect_f const& rect)
{
    draw_image(_assets.get_texture(image), region, rect);
}

void canvas::draw_image(gfx::texture* image, string const& region, rect_f const& rect)
{
    _canvas.set_fill_style(colors::White);
    _canvas.draw_image(image, region, rect);
}

auto canvas::get_context() -> gfx::canvas&
{
    return _canvas;
}

auto canvas::get_last_frame() -> gfx::image&
{
    return _lastFrame;
}

void canvas::snap()
{
    static std::vector<gfx::animated_image_encoder::frame> LastFrames;
    auto                                                   stats {locate_service<gfx::render_system>().get_stats()};
    LastFrames.push_back({_lastFrame, LastFrames.empty() ? 0ms : milliseconds {stats.get_time() - LastFrames[0].TimeStamp.count()}});
    if (LastFrames.size() > 300) {
        auto         enc {locate_service<gfx::animated_image_encoder::factory>().create(".webp")};
        io::ofstream fs {"test.webp"};
        enc->encode(LastFrames, fs);
        LastFrames.clear();
    }
}
}