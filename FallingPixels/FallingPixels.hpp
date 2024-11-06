// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <tcob/tcob.hpp>

#include "Element.hpp"
#include "UI.hpp"

using namespace tcob;

////////////////////////////////////////////////////////////

class MainScene : public scene {
public:
    MainScene(game& game);
    ~MainScene() override;

protected:
    void on_start() override;
    void on_finish() override;

    void on_draw_to(gfx::render_target& target) override;

    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_key_down(input::keyboard::event const& ev) override;
    void on_mouse_motion(input::mouse::motion_event const& ev) override;
    void on_mouse_button_up(input::mouse::button_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;
    void on_mouse_wheel(input::mouse::wheel_event const& ev) override;

private:
    void update_image();

    stopwatch _hourGlass0;
    stopwatch _hourGlass1;

    element_system _elementSystem;
    element_type   _spawnElement {element_type::Empty};
    element_type   _leftBtnElement {element_type::Sand};
    element_type   _middleBtnElement {element_type::Wall};
    element_type   _rightBtnElement {element_type::Water};
    bool           _mouseDown {false};

    gfx::shape_batch                 _layer0;
    std::shared_ptr<gfx::rect_shape> _shape0;

    assets::manual_asset_ptr<gfx::material> _sandMat;
    assets::manual_asset_ptr<gfx::texture>  _sandTex;
    gfx::image                              _sandImg;

    std::shared_ptr<elements_form> _form;
};
