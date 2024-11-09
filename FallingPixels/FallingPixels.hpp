// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "ElementSystem.hpp"
#include "UI.hpp"

////////////////////////////////////////////////////////////

using script_element_vec = std::vector<std::tuple<i32, std::string, lua::table>>;

class main_scene : public scene {
public:
    main_scene(game& game);
    ~main_scene() override;

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

    auto load_script() -> script_element_vec;

    lua::script                    _script;
    lua::native_closure_shared_ptr _registerElement;

    stopwatch _hourGlass0;

    std::unique_ptr<element_system> _elementSystem;

    i32  _spawnElement {0};
    i32  _leftBtnElement {2};
    i32  _middleBtnElement {1};
    i32  _rightBtnElement {0};
    bool _mouseDown {false};

    gfx::shape_batch                 _layer0;
    std::shared_ptr<gfx::rect_shape> _shape0;

    assets::manual_asset_ptr<gfx::material> _sandMat;
    assets::manual_asset_ptr<gfx::texture>  _sandTex;
    gfx::image                              _sandImg;

    std::shared_ptr<elements_form> _form;

    bool _drawHeatMap {false};
};
