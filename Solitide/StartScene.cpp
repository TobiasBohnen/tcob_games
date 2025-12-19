// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "StartScene.hpp"

#include "MainScene.hpp"

namespace solitaire {

start_scene::start_scene(game& game)
    : scene {game}
    , _renderer {_canvas}
    , _windowSize {window().Size}
{
    auto& resMgr {library()};
    auto& resGrp {resMgr.create_or_get_group("solitaire")};
    resGrp.mount("./sol/assets.zip");
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
            _canvas.save();

            _canvas.begin_path();

            static std::string Text = R"(
     XX   XX  XXX   XXX XXXXX XXX XXX  XXXX
    X  X X  X  X     X  X X X  X  X  X X  X
    X    X  X  X     X    X    X  X  X X
XX   XX  X  X  X     X    X    X  X  X XXX   XX
       X X  X  X     X    X    X  X  X X
    X  X X  X  X  X  X    X    X  X  X X  X
     XX   XX  XXXXX XXX  XXX  XXX XXX  XXXX


 XX    XX    XX    XX    XX    XX    XX    XX
X  X  X  X  X  X  X  X  X  X  X  X  X  X  X  X
    XX    XX    XX    XX    XX    XX    XX   
)";
            i32 const          imgWidth {47};
            f32 const          width {std::floor(_windowSize.Width / 3.0f)};
            f32 const          pixWidth {std::floor(width / imgWidth)};
            f32 const          offX {std::floor((_windowSize.Width - (pixWidth * imgWidth)) / 2)};

            point_i pos {};
            for (auto it {Text.begin()}; it != Text.end(); ++it) {
                switch (*it) {
                case '\n': pos = {0, pos.Y + 1}; break;
                case ' ':  ++pos.X; break;
                case 'X':  {
                    rect_f const pixBounds {(pos.X++ * pixWidth) + offX, (pos.Y * pixWidth) + (_windowSize.Height / 4.f), pixWidth, pixWidth};
                    _canvas.rect(pixBounds);
                } break;
                }
            }

            _canvas.set_edge_antialias(false);
            _canvas.set_stroke_style(colors::DarkSlateBlue);
            _canvas.set_stroke_width(5);
            _canvas.stroke();
            _canvas.set_fill_style(colors::DodgerBlue);
            _canvas.fill();
            _canvas.set_edge_antialias(true);
            _canvas.restore();
        }

        // suits
        {
            f32 const suitWidth {_windowSize.Width / 15.f};
            f32 const off {(_windowSize.Width - suitWidth * 7) / 2};
            point_f   pos {off, _windowSize.Height / 2.f};

            auto const draw {[&](std::string_view path, suit_color sc) {
                auto ppath {gfx::path2d::Parse(path)};

                _canvas.save();

                _canvas.translate(pos);
                f32 const scale1 {suitWidth};
                _canvas.scale({scale1, scale1});

                _canvas.begin_path();
                _canvas.path_2d(*ppath);

                _canvas.set_stroke_style(colors::White);
                _canvas.set_stroke_width(5 / scale1);
                _canvas.stroke();
                switch (sc) {
                case suit_color::Red:   _canvas.set_fill_style(colors::Red); break;
                case suit_color::Black: _canvas.set_fill_style(colors::Black); break;
                }
                _canvas.fill();

                _canvas.restore();

                pos += point_f {suitWidth * 2, 0};
            }};

            draw("m 0.483 0.2 c 0 -0.123 -0.095 -0.2 -0.233 -0.2 -0.138 0 -0.25 0.099 -0.25 0.222 0.001 0.011 0 0.021 0 0.031 0 0.081 0.045 0.16 0.083 0.233 0.04 0.076 0.097 0.142 0.151 0.21 0.083 0.105 0.265 0.303 0.265 0.303 0 0 0.182 -0.198 0.265 -0.303 0.054 -0.068 0.111 -0.134 0.151 -0.21 0.039 -0.074 0.083 -0.152 0.083 -0.233 -0 -0.011 -0 -0.021 0 -0.031 0 -0.123 -0.112 -0.222 -0.25 -0.222 -0.138 0 -0.233 0.077 -0.233 0.2 0 0.022 -0.033 0.022 -0.033 0 z", suit_color::Red);
            draw("m 0.583 0.622 c 0.033 0.044 0.117 0.111 0.2 0.111 0.15 0 0.216 -0.089 0.216 -0.2 0 -0.111 -0.067 -0.211 -0.216 -0.211 -0.083 0 -0.133 0.056 -0.183 0.089 -0.017 0.011 -0.033 0 -0.017 -0.011 0.05 -0.033 0.133 -0.122 0.133 -0.2 0 -0.089 -0.083 -0.2 -0.216 -0.2 -0.133 0 -0.216 0.111 -0.216 0.2 0 0.078 0.083 0.166 0.133 0.2 0.017 0.011 0 0.022 -0.017 0.011 -0.05 -0.033 -0.1 -0.089 -0.183 -0.089 -0.15 0 -0.216 0.1 -0.216 0.211 0 0.111 0.067 0.2 0.216 0.2 0.083 0 0.166 -0.067 0.2 -0.111 0.017 -0.011 0.017 0 0.017 0.011 -0.017 0.089 -0.017 0.111 -0.033 0.178 -0.017 0.067 -0.067 0.189 -0.067 0.189 0.083 -0.022 0.25 -0.022 0.333 0 0 0 -0.05 -0.122 -0.067 -0.189 -0.017 -0.067 -0.017 -0.089 -0.033 -0.178 0 -0.011 0 -0.022 0.017 -0.011 z", suit_color::Black);
            draw("m 0.5 0 c 0 0 0.1 0.166 0.216 0.277 0.117 0.111 0.283 0.222 0.283 0.222 0 0 -0.166 0.111 -0.283 0.222 C 0.6 0.833 0.5 0.999 0.5 0.999 c 0 0 -0.1 -0.166 -0.216 -0.277 C 0.167 0.611 0 0.5 0 0.5 c 0 0 0.166 -0.111 0.283 -0.222 C 0.4 0.167 0.5 0 0.5 0", suit_color::Red);
            draw("m 0.583 0.622 c 0.017 0.044 0.083 0.156 0.2 0.156 C 0.933 0.778 1 0.689 1 0.578 1 0.5 0.957 0.449 0.917 0.389 0.874 0.325 0.81 0.268 0.75 0.211 0.673 0.138 0.5 0 0.5 0 c 0 0 -0.173 0.138 -0.25 0.211 C 0.19 0.268 0.126 0.325 0.083 0.389 0.043 0.449 0 0.5 0 0.578 c 0 0.111 0.067 0.2 0.217 0.2 0.117 0 0.183 -0.111 0.2 -0.156 0.017 -0.011 0.017 0 0.017 0.011 C 0.417 0.722 0.417 0.744 0.4 0.811 0.383 0.878 0.333 1 0.333 1 0.417 0.978 0.583 0.978 0.667 1 c 0 0 -0.05 -0.122 -0.067 -0.189 -0.017 -0.067 -0.017 -0.089 -0.033 -0.178 0 -0.011 0 -0.022 0.017 -0.011 z", suit_color::Black);
        }

        _canvas.end_frame();

        _canvasDirty = false;
        _drawn       = true;
    }

    _renderer.add_layer(0);
    _renderer.render_to_target(target);
}

void start_scene::on_update(milliseconds)
{
    if (_drawn && library().is_loading_complete()) {
        parent().pop_current_scene();
        parent().push_scene<main_scene>();
    }
}
}
