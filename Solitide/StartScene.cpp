// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "StartScene.hpp"

#include "MainScene.hpp"

namespace solitaire {

start_scene::start_scene(game& game)
    : scene {game}
    , _renderer {_canvas}
    , _windowSize {game.get_window().Size}
{
    auto& resMgr {locate_service<assets::library>()};
    auto& resGrp {resMgr.create_or_get_group("solitaire")};
    resGrp.mount("./assets.zip");
    resMgr.load_all_groups();
}

void start_scene::on_start()
{
}

void start_scene::on_draw_to(gfx::render_target& target)
{
    if (_canvasDirty) {
        rect_f bounds {point_f::Zero, size_f {_windowSize}};
        _renderer.set_bounds(bounds);

        // background
        _canvas.begin_frame(_windowSize, 1.0f);
        _canvas.set_fill_style(colors::LightSkyBlue);
        _canvas.begin_path();
        _canvas.rect(bounds);
        _canvas.fill();

        // text
        {
            static std::string Text = R"(P1
43 9
0 0 0 0 0 0  0 0 0 0 0 0  0 0 0 0 0 0  0 0 0  0 0 0 0 0 0 0  0 0 0  0 0 0 0 0 0  0 0 0 0 0 0
0 1 1 1 1 0  0 1 1 1 1 0  0 1 0 0 0 0  0 1 0  0 1 1 1 1 1 0  0 1 0  0 1 1 1 0 0  0 1 1 1 1 0
0 1 0 0 0 0  0 1 0 0 1 0  0 1 0 0 0 0  0 1 0  0 0 0 1 0 0 0  0 1 0  0 1 0 0 1 0  0 1 0 0 0 0
0 1 0 0 0 0  0 1 0 0 1 0  0 1 0 0 0 0  0 1 0  0 0 0 1 0 0 0  0 1 0  0 1 0 0 1 0  0 1 0 0 0 0
0 1 1 1 1 0  0 1 0 0 1 0  0 1 0 0 0 0  0 1 0  0 0 0 1 0 0 0  0 1 0  0 1 0 0 1 0  0 1 1 1 0 0
0 0 0 0 1 0  0 1 0 0 1 0  0 1 0 0 0 0  0 1 0  0 0 0 1 0 0 0  0 1 0  0 1 0 0 1 0  0 1 0 0 0 0
0 0 0 0 1 0  0 1 0 0 1 0  0 1 0 0 0 0  0 1 0  0 0 0 1 0 0 0  0 1 0  0 1 0 0 1 0  0 1 0 0 0 0
0 1 1 1 1 0  0 1 1 1 1 0  0 1 1 1 1 0  0 1 0  0 0 0 1 0 0 0  0 1 0  0 1 1 1 0 0  0 1 1 1 1 0
0 0 0 0 0 0  0 0 0 0 0 0  0 0 0 0 0 0  0 0 0  0 0 0 0 0 0 0  0 0 0  0 0 0 0 0 0  0 0 0 0 0 0
)";
            io::isstream       istream {Text};
            auto const         img {gfx::image::Load(istream, ".pnm")};
            auto const         imgSize {img->get_info().Size};

            _canvas.set_fill_style(colors::DarkBlue);

            _canvas.begin_path();
            f32 const pixWidth {_windowSize.Width / 3.0f / imgSize.Width};

            for (i32 x {0}; x < imgSize.Width; ++x) {
                for (i32 y {0}; y < imgSize.Height; ++y) {
                    if (img->get_pixel({x, y}) == colors::Black) {
                        _canvas.rect({x * pixWidth + _windowSize.Width / 3.0f, y * pixWidth + _windowSize.Height / 4.f, pixWidth, pixWidth});
                    }
                }
            }

            _canvas.fill();
        }

        // suits
        {
            point_f pos {_windowSize.Width / 7.f, _windowSize.Height / 2.f};
            auto    draw {[&](std::string_view path) {
                auto ppath {gfx::canvas::path2d::Parse(path)};

                _canvas.save();

                _canvas.translate(pos);
                f32 const scale1 {_windowSize.Width / 15.f};
                _canvas.scale({scale1, scale1});

                _canvas.begin_path();
                _canvas.path_2d(*ppath);

                _canvas.set_fill_style(colors::DarkBlue);
                _canvas.fill();

                _canvas.restore();

                pos += point_f {_windowSize.Width / 5.f, 0};
            }};

            draw("m 0.483 0.2 c 0 -0.123 -0.095 -0.2 -0.233 -0.2 -0.138 0 -0.25 0.099 -0.25 0.222 0.001 0.011 0 0.021 0 0.031 0 0.081 0.045 0.16 0.083 0.233 0.04 0.076 0.097 0.142 0.151 0.21 0.083 0.105 0.265 0.303 0.265 0.303 0 0 0.182 -0.198 0.265 -0.303 0.054 -0.068 0.111 -0.134 0.151 -0.21 0.039 -0.074 0.083 -0.152 0.083 -0.233 -0 -0.011 -0 -0.021 0 -0.031 0 -0.123 -0.112 -0.222 -0.25 -0.222 -0.138 0 -0.233 0.077 -0.233 0.2 0 0.022 -0.033 0.022 -0.033 0 z");
            draw("m 0.5 0 c 0 0 0.1 0.166 0.216 0.277 0.117 0.111 0.283 0.222 0.283 0.222 0 0 -0.166 0.111 -0.283 0.222 C 0.6 0.833 0.5 0.999 0.5 0.999 c 0 0 -0.1 -0.166 -0.216 -0.277 C 0.167 0.611 0 0.5 0 0.5 c 0 0 0.166 -0.111 0.283 -0.222 C 0.4 0.167 0.5 0 0.5 0");
            draw("m 0.583 0.622 c 0.033 0.044 0.117 0.111 0.2 0.111 0.15 0 0.216 -0.089 0.216 -0.2 0 -0.111 -0.067 -0.211 -0.216 -0.211 -0.083 0 -0.133 0.056 -0.183 0.089 -0.017 0.011 -0.033 0 -0.017 -0.011 0.05 -0.033 0.133 -0.122 0.133 -0.2 0 -0.089 -0.083 -0.2 -0.216 -0.2 -0.133 0 -0.216 0.111 -0.216 0.2 0 0.078 0.083 0.166 0.133 0.2 0.017 0.011 0 0.022 -0.017 0.011 -0.05 -0.033 -0.1 -0.089 -0.183 -0.089 -0.15 0 -0.216 0.1 -0.216 0.211 0 0.111 0.067 0.2 0.216 0.2 0.083 0 0.166 -0.067 0.2 -0.111 0.017 -0.011 0.017 0 0.017 0.011 -0.017 0.089 -0.017 0.111 -0.033 0.178 -0.017 0.067 -0.067 0.189 -0.067 0.189 0.083 -0.022 0.25 -0.022 0.333 0 0 0 -0.05 -0.122 -0.067 -0.189 -0.017 -0.067 -0.017 -0.089 -0.033 -0.178 0 -0.011 0 -0.022 0.017 -0.011 z");
            draw("m 0.583 0.622 c 0.017 0.044 0.083 0.156 0.2 0.156 C 0.933 0.778 1 0.689 1 0.578 1 0.5 0.957 0.449 0.917 0.389 0.874 0.325 0.81 0.268 0.75 0.211 0.673 0.138 0.5 0 0.5 0 c 0 0 -0.173 0.138 -0.25 0.211 C 0.19 0.268 0.126 0.325 0.083 0.389 0.043 0.449 0 0.5 0 0.578 c 0 0.111 0.067 0.2 0.217 0.2 0.117 0 0.183 -0.111 0.2 -0.156 0.017 -0.011 0.017 0 0.017 0.011 C 0.417 0.722 0.417 0.744 0.4 0.811 0.383 0.878 0.333 1 0.333 1 0.417 0.978 0.583 0.978 0.667 1 c 0 0 -0.05 -0.122 -0.067 -0.189 -0.017 -0.067 -0.017 -0.089 -0.033 -0.178 0 -0.011 0 -0.022 0.017 -0.011 z");
        }

        _canvas.end_frame();

        _canvasDirty = false;
        _drawn       = true;
    }

    _renderer.set_layer(0);
    _renderer.render_to_target(target);
}

void start_scene::on_update(milliseconds)
{
    if (_drawn && locate_service<assets::library>().is_loading_complete()) {
        get_game().pop_current_scene();
        get_game().push_scene<main_scene>();
    }
}
}
