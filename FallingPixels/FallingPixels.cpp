// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "FallingPixels.hpp"

#include <iomanip>
#include <iostream>

using namespace std::chrono_literals;
using namespace tcob::literals;

constexpr size_i GRID_SIZE {180, 180};

MainScene::MainScene(game& game)
    : scene(game)
    , _elementSystem {GRID_SIZE}
{
    auto& window {get_window()};
    auto  winSize {window.Size()};

    _sandImg = gfx::image::CreateEmpty(GRID_SIZE, gfx::image::format::RGBA);
    _sandTex->create(_sandImg.get_info().Size, 1, gfx::texture::format::RGBA8);

    _shape0 = _layer0.create_shape<gfx::rect_shape>();
    f32 const height {static_cast<f32>(winSize.Height)};
    _shape0->Bounds            = {point_f::Zero, {height, height}};
    _shape0->Material          = _sandMat;
    _shape0->Material->Texture = _sandTex;

    _form = std::make_shared<elements_form>(&window, rect_f {height, 0, winSize.Width - height, height});
    _form->LeftButtonElement.connect([&](element_type t) { _leftBtnElement = t; });
    _form->MiddleButtonElement.connect([&](element_type t) { _middleBtnElement = t; });
    _form->RightButtonElement.connect([&](element_type t) { _rightBtnElement = t; });
}

MainScene::~MainScene() = default;

void MainScene::on_start()
{
    get_window().ClearColor = colors::Black;
    _hourGlass0.restart();
    _hourGlass1.restart();

    get_root_node()->Entity = _form;
}

void MainScene::on_finish()
{
}

void MainScene::on_draw_to(gfx::render_target& target)
{
    _layer0.draw_to(target);
}

void MainScene::on_update(milliseconds deltaTime)
{
    if (_hourGlass1.get_elapsed_milliseconds() > 10) {
        if (_mouseDown) {
            rng           rand;
            point_i const ev {input::system::GetMousePosition()};
            f32 const     height {static_cast<f32>(get_window().Size().Height)};
            if (ev.X < height) {
                f32 const scale {height / GRID_SIZE.Height};
                i32 const spread {_spawnElement == element_type::Wall ? 100 : 1};
                for (i32 i {0}; i < spread; ++i) {
                    point_i const pos {static_cast<i32>((ev.X + rand(-10, 10)) / scale), static_cast<i32>((ev.Y + rand(-10, 10)) / scale)};
                    _elementSystem.force_set(pos, _spawnElement);
                }
                update_image();
            }
        }

        _hourGlass1.restart();
    }

    if (_hourGlass0.get_elapsed_milliseconds() > 20) {
        _layer0.update(deltaTime);
        _elementSystem.update();
        update_image();

        _hourGlass0.restart();
    }
}

void MainScene::on_fixed_update(milliseconds deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    auto const& stats {locate_service<gfx::render_system>().get_stats()};
    stream << "avg FPS:" << stats.get_average_FPS();
    stream << " best FPS:" << stats.get_best_FPS();
    stream << " worst FPS:" << stats.get_worst_FPS();

    get_window().Title = "FallingPixels " + stream.str();
}

void MainScene::on_key_down(input::keyboard::event const& ev)
{
    if (ev.ScanCode == input::scan_code::BACKSPACE) {
        get_game().pop_current_scene();
    }
}

void MainScene::on_mouse_motion(input::mouse::motion_event const& ev)
{
}

void MainScene::on_mouse_button_up(input::mouse::button_event const& ev)
{
    _spawnElement = element_type::Empty;
    _mouseDown    = false;
}

void MainScene::on_mouse_button_down(input::mouse::button_event const& ev)
{
    if (ev.Button == input::mouse::button::Left) {
        _spawnElement = _leftBtnElement;
    } else if (ev.Button == input::mouse::button::Right) {
        _spawnElement = _rightBtnElement;
    } else if (ev.Button == input::mouse::button::Middle) {
        _spawnElement = _middleBtnElement;
    }
    _mouseDown = true;
}

void MainScene::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
}

void MainScene::update_image()
{
    _elementSystem.update_image(_sandImg);
    _sandTex->update_data(_sandImg.get_data().data(), 0);
}

////////////////////////////////////////////////////////////
