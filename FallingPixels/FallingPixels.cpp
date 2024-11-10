// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "FallingPixels.hpp"

#include <iomanip>
#include <iostream>

using namespace std::chrono_literals;
using namespace tcob::literals;

constexpr size_i GRID_SIZE {200, 200};

main_scene::main_scene(game& game)
    : scene(game)
{
    auto& window {get_window()};
    auto  winSize {window.Size()};

    _sandImg = gfx::image::CreateEmpty(GRID_SIZE, gfx::image::format::RGBA);
    _sandTex->create(_sandImg.get_info().Size, 1, gfx::texture::format::RGBA8);
    //_sandTex->Filtering = gfx::texture::filtering::Linear;

    _shape0 = _layer0.create_shape<gfx::rect_shape>();
    f32 const height {static_cast<f32>(winSize.Height)};
    _shape0->Bounds            = {point_f::Zero, {height, height}};
    _shape0->Material          = _sandMat;
    _shape0->Material->Texture = _sandTex;

    ////
    script_element_vec const elements {load_script()};
    auto                     name_to_id {[&](std::string const& f) {
        for (auto const& [id, name, table] : elements) {
            if (name == f) {
                return id;
            }
        }
        return 0;
    }};

    std::vector<element_def> elementsDefs;
    for (auto const& [id, name, table] : elements) {
        element_def& element {elementsDefs.emplace_back()};
        element.ID   = id;
        element.Name = name;

        std::vector<std::string> colors;
        table.try_get(colors, "Colors");
        for (auto const& color : colors) {
            element.Colors.push_back(color::FromString(color));
        }

        table.try_get(element.Temperature, "Temperature");
        table.try_get(element.Gravity, "Gravity");
        table.try_get(element.Dispersion, "Dispersion");

        element.Density = table["Density"].as<f32>();
        element.Type    = table["Type"].as<element_type>();

        lua::table transitions;
        if (table.try_get(transitions, "Transitions")) {
            auto const keys {transitions.get_keys<i32>()};
            for (auto const& key : keys) {
                lua::table transition {transitions[key].as<lua::table>()};

                if (transition.has("Temperature")) {
                    temp_transition val;
                    transition.try_get(val.Temperature, "Temperature");
                    transition.try_get(val.Op, "Op");
                    val.Target = name_to_id(transition["Target"].as<std::string>());
                    element.Transitions.emplace_back(val);
                } else if (transition.has("Neighbor")) {
                    neighbor_transition val;
                    val.Neighbor = name_to_id(transition["Neighbor"].as<std::string>());
                    val.Target   = name_to_id(transition["Target"].as<std::string>());
                    element.Transitions.emplace_back(val);
                }
            }
        }
    }

    _elementSystem = std::make_unique<element_system>(GRID_SIZE, elementsDefs);
    ////

    _form = std::make_shared<elements_form>(&window, rect_f {height, 0, winSize.Width - height, height}, elementsDefs);
    _form->LeftButtonElement.connect([&](i32 t) { _leftBtnElement = t; });
    _form->MiddleButtonElement.connect([&](i32 t) { _middleBtnElement = t; });
    _form->RightButtonElement.connect([&](i32 t) { _rightBtnElement = t; });
}

main_scene::~main_scene() = default;

void main_scene::on_start()
{
    get_window().ClearColor = colors::Black;
    _hourGlass0.restart();

    get_root_node()->Entity = _form;
}

void main_scene::on_finish()
{
}

void main_scene::on_draw_to(gfx::render_target& target)
{
    _layer0.draw_to(target);
}

void main_scene::on_update(milliseconds deltaTime)
{
    _layer0.update(deltaTime);

    if (_mouseDown) {
        point_i const ev {input::system::GetMousePosition()};
        f32 const     height {static_cast<f32>(get_window().Size().Height)};
        if (ev.X < height) {
            f32 const scale {height / GRID_SIZE.Height};
            _elementSystem->spawn(ev / scale, _spawnElement);
        }
    }

    if (_hourGlass0.get_elapsed_milliseconds() > 10) {
        _elementSystem->update();
        update_image();
        _hourGlass0.restart();
    }
}

void main_scene::on_fixed_update(milliseconds deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    auto const& stats {locate_service<gfx::render_system>().get_stats()};
    stream << "avg FPS:" << stats.average_FPS();
    stream << " best FPS:" << stats.best_FPS();
    stream << " worst FPS:" << stats.worst_FPS();

    point_i const ev {input::system::GetMousePosition()};
    f32 const     height {static_cast<f32>(get_window().Size().Height)};
    if (ev.X < height) {
        f32 const scale {height / GRID_SIZE.Height};
        stream << "| name:" << _elementSystem->info_name(ev / scale);
    }

    get_window().Title = "FallingPixels " + stream.str();
}

void main_scene::on_key_down(input::keyboard::event const& ev)
{
    if (ev.ScanCode == input::scan_code::BACKSPACE) {
        get_game().pop_current_scene();
    } else if (ev.ScanCode == input::scan_code::H) {
        _drawHeatMap = !_drawHeatMap;
    }
}

void main_scene::on_mouse_motion(input::mouse::motion_event const& ev)
{
}

void main_scene::on_mouse_button_up(input::mouse::button_event const& ev)
{
    _spawnElement = 0;
    _mouseDown    = false;
}

void main_scene::on_mouse_button_down(input::mouse::button_event const& ev)
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

void main_scene::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
}

void main_scene::update_image()
{
    if (_drawHeatMap) {
        _elementSystem->draw_heatmap(_sandImg);
    } else {
        _elementSystem->draw_image(_sandImg);
    }
    _sandTex->update_data(_sandImg.buffer().data(), 0);
}

auto main_scene::load_script() -> script_element_vec
{
    using namespace scripting::lua;

    _script.open_libraries(library::Table, library::String, library::Math, library::Coroutine);
    auto& global {_script.get_global_table()};
    table env {_script.create_table()};
    env["table"]     = global["table"];
    env["string"]    = global["string"];
    env["math"]      = global["math"];
    env["coroutine"] = global["coroutine"];

    env["pairs"]        = global["pairs"];
    env["ipairs"]       = global["ipairs"];
    env["print"]        = global["print"];
    env["type"]         = global["type"];
    env["tonumber"]     = global["tonumber"];
    env["tostring"]     = global["tostring"];
    env["setmetatable"] = global["setmetatable"];
    env["getmetatable"] = global["getmetatable"];

    script_element_vec elements;
    _registerElement        = make_shared_closure(std::function {[&](std::string const& name, table const& table) {
        elements.emplace_back(elements.size(), name, table);
    }});
    env["register_element"] = _registerElement.get();
    _script.set_environment(env);

    std::ignore = _script.run_file("elements.lua");

    return elements;
}

////////////////////////////////////////////////////////////
