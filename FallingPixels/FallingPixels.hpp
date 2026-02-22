// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "ElementSystem.hpp"
#include "UI.hpp"

////////////////////////////////////////////////////////////

class elements_entity : public gfx::entity { // TODO: rename
public:
    elements_entity(std::vector<element_def> const& elementsDefs);

    bool DrawHeatMap {false};

    auto system() const -> element_system&
    {
        return *_elementSystem;
    }

    void center_camera(gfx::camera& cam) const
    {
        cam.Zoom = {3.f, 3.f};
        cam.look_at(_shape->Bounds->center());
    }

protected:
    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    auto can_draw() const -> bool override;
    void on_draw_to(gfx::render_target& target, transform const& xform) override;

private:
    void update_image();

    stopwatch _hourGlass0;

    gfx::shape_batch _layer0;

    asset_owner_ptr<gfx::material> _sandMat;
    asset_owner_ptr<gfx::texture>  _sandTex;

    std::shared_ptr<element_system> _elementSystem;
    gfx::rect_shape*                _shape {nullptr};
};

////////////////////////////////////////////////////////////

using script_element_vec = std::vector<std::tuple<u16, std::string, table>>;

class main_scene : public scene {
public:
    main_scene(game& game);
    ~main_scene() override;

protected:
    void on_start() override;
    void on_finish() override;

    void on_draw_to(gfx::render_target& target, transform const& xform) override;

    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_key_down(input::keyboard::event const& ev) override;
    void on_mouse_motion(input::mouse::motion_event const& ev) override;
    void on_mouse_button_up(input::mouse::button_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;
    void on_mouse_wheel(input::mouse::wheel_event const& ev) override;

private:
    auto load_script() -> script_element_vec;

    script                                            _script;
    std::vector<scripting::native_closure_shared_ptr> _funcs;

    i32                  _spawnElement {0};
    i32                  _leftBtnElement {0};
    input::mouse::button _mouseDown {input::mouse::button::None};
    i32                  _zoomStage {1};

    std::shared_ptr<elements_form>   _form;
    std::shared_ptr<elements_entity> _entity;
};
