// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "FallingPixels.hpp"

#include <iomanip>
#include <iostream>

using namespace std::chrono_literals;
using namespace tcob::literals;

elements_entity::elements_entity()
{
    _sandTex->create(GRID_SIZE, 1, gfx::texture::format::RGBA8);
    //_sandTex->Filtering = gfx::texture::filtering::Linear;

    Shape                    = _layer0.create_shape<gfx::rect_shape>();
    Shape->Bounds            = {point_f::Zero, size_f {GRID_SIZE}};
    Shape->Material          = _sandMat;
    Shape->Material->Texture = _sandTex;

    _hourGlass0.restart();
}

void elements_entity::on_update(milliseconds deltaTime)
{
    _layer0.update(deltaTime);

    if (_hourGlass0.elapsed_milliseconds() > 20) {
        ElementSystem->update();
        update_image();
        _hourGlass0.restart();
    }
}

void elements_entity::on_fixed_update(milliseconds)
{
}

auto elements_entity::can_draw() const -> bool
{
    return ElementSystem != nullptr;
}

void elements_entity::on_draw_to(gfx::render_target& target)
{
    _layer0.draw_to(target);
}

void elements_entity::update_image()
{
    if (DrawHeatMap) {
        ElementSystem->draw_heatmap(*_sandTex.ptr());
    } else {
        ElementSystem->draw_elements(*_sandTex.ptr());
    }
}

////////////////////////////////////////////////////////////

main_scene::main_scene(game& game)
    : scene(game)
{

    ////
    script_element_vec const elements {load_script()};
    auto const               name_to_id {[&](std::string const& f) -> u16 {
        if (f == "Any") { return ANY_ELEMENT; }

        for (auto const& [id, name, table] : elements) {
            if (name == f) { return id; }
        }
        return EMPTY_ELEMENT;
    }};

    std::vector<element_def> elementsDefs;
    for (auto const& [id, name, table] : elements) {
        element_def& element {elementsDefs.emplace_back()};
        element.Element.ID = id;
        element.Name       = name;

        // required
        element.Element.Gravity = table["Gravity"].as<i8>();
        element.Element.Density = table["Density"].as<f32>();
        element.Element.Type    = table["Type"].as<element_type>();

        // optional
        std::vector<std::string> colors;
        table.try_get(colors, "Colors");
        for (auto const& color : colors) {
            element.Colors.push_back(color::FromString(color));
        }

        table.try_get(element.Temperature, "Temperature");
        table.try_get(element.Element.ThermalConductivity, "ThermalConductivity");
        table.try_get(element.Element.Dispersion, "Dispersion");
        table.try_get(element.Element.Dissolvable, "Dissolvable");

        lua::table rulesTable;
        if (table.try_get(rulesTable, "Rules")) {
            auto const keys {rulesTable.get_keys<i32>()};
            for (auto const& key : keys) {
                lua::table ruleTable {rulesTable[key].as<lua::table>()};

                if (ruleTable.has("Temperature")) {
                    temp_rule  val;
                    lua::table table {ruleTable["Temperature"].as<lua::table>()};

                    if (table.try_get(val.Temperature, "Above")) {
                        val.Op = comp_op::GreaterThan;
                    } else if (table.try_get(val.Temperature, "Below")) {
                        val.Op = comp_op::LessThan;
                    }
                    val.Result = name_to_id(table["Result"].as<std::string>());
                    element.Rules.emplace_back(val);
                } else if (ruleTable.has("Neighbor")) {
                    neighbor_rule val;
                    lua::table    table {ruleTable["Neighbor"].as<lua::table>()};

                    val.Element        = name_to_id(table["Element"].as<std::string>());
                    val.NeighborResult = name_to_id(table["NeighborResult"].as<std::string>());
                    val.Result         = name_to_id(table["Result"].as<std::string>());
                    element.Rules.emplace_back(val);
                } else if (ruleTable.has("Dissolve")) {
                    dissolve_rule val;
                    lua::table    table {ruleTable["Dissolve"].as<lua::table>()};

                    val.Element = name_to_id(table["Element"].as<std::string>());
                    val.Result  = name_to_id(table["Result"].as<std::string>());
                    element.Rules.emplace_back(val);
                }
            }
        }
    }

    _entity                = std::make_shared<elements_entity>();
    _entity->ElementSystem = std::make_unique<element_system>(elementsDefs);

    ////
    auto&      win {window()};
    auto const winSize {win.Size()};
    f32 const  height {static_cast<f32>(winSize.Height)};
    _form = std::make_shared<elements_form>(&win, rect_f {height, 0, winSize.Width - height, height}, elementsDefs);
    _form->SelectedElement.connect([&](i32 t) { _leftBtnElement = t; });

    win.get_camera().Zoom = {3.f, 3.f};
    win.get_camera().look_at(_entity->Shape->Bounds->center());
}

main_scene::~main_scene() = default;

void main_scene::on_start()
{
    window().ClearColor = colors::SlateGray;

    root_node()->create_child()->Entity = _entity;
    root_node()->create_child()->Entity = _form;
}

void main_scene::on_finish()
{
}

void main_scene::on_draw_to(gfx::render_target& target)
{
}

void main_scene::on_update(milliseconds deltaTime)
{
    if (_mouseDown == input::mouse::button::Left) {
        auto const ev {point_i {window().get_camera().convert_screen_to_world(locate_service<input::system>().mouse().get_position())}};
        _entity->ElementSystem->spawn(ev, _spawnElement);
    }
}

void main_scene::on_fixed_update(milliseconds deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    auto const& stats {locate_service<gfx::render_system>().stats()};
    stream << "avg FPS:" << stats.average_FPS();
    stream << " best FPS:" << stats.best_FPS();
    stream << " worst FPS:" << stats.worst_FPS();

    auto const ev {point_i {window().get_camera().convert_screen_to_world(locate_service<input::system>().mouse().get_position())}};
    stream << "| name:" << _entity->ElementSystem->info_name(ev);
    stream << "| heat:" << _entity->ElementSystem->info_heat(ev);

    window().Title = "FallingPixels " + stream.str();
}

void main_scene::on_key_down(input::keyboard::event const& ev)
{
    if (ev.ScanCode == input::scan_code::BACKSPACE) {
        parent().pop_current_scene();
    } else if (ev.ScanCode == input::scan_code::H) {
        _entity->DrawHeatMap = !_entity->DrawHeatMap;
    } else if (ev.ScanCode == input::scan_code::C) {
        _entity->ElementSystem->clear();
    } else if (ev.ScanCode == input::scan_code::S) {
        io::ofstream stream {"grid.bin"};
        _entity->ElementSystem->save(stream);
    } else if (ev.ScanCode == input::scan_code::L) {
        io::ifstream stream {"grid.bin"};
        _entity->ElementSystem->load(stream);
    }
}

void main_scene::on_mouse_motion(input::mouse::motion_event const& ev)
{
    if (_mouseDown == input::mouse::button::Right) {
        auto&      camera {window().get_camera()};
        auto const zoom {camera.Zoom};
        camera.move_by(-(point_f {ev.RelativeMotion} / point_f {zoom.Width, zoom.Height}));
    }
}

void main_scene::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
    _zoomStage = std::clamp(_zoomStage - ev.Scroll.Y, 0, 6);
    constexpr std::array<size_f, 7> zoomLevels {{{5.f, 5.f}, {3.f, 3.f}, {2.f, 2.f}, {1.f, 1.f}, {0.75f, 0.75f}, {0.5f, 0.5f}, {0.25f, 0.25f}}};
    window().get_camera().Zoom = zoomLevels[_zoomStage];
}

void main_scene::on_mouse_button_up(input::mouse::button_event const& ev)
{
    _spawnElement = 0;
    _mouseDown    = input::mouse::button::None;
}

void main_scene::on_mouse_button_down(input::mouse::button_event const& ev)
{
    if (ev.Button == input::mouse::button::Left) {
        _spawnElement = _leftBtnElement;
    }
    _mouseDown = ev.Button;
}

auto main_scene::load_script() -> script_element_vec
{
    using namespace scripting::lua;

    _script.open_libraries(library::Table, library::String, library::Math, library::Coroutine);
    auto& global {_script.global_table()};
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

    auto const make_func {[&](auto&& func) {
        return _funcs.emplace_back(make_shared_closure(std::function {func})).get();
    }};

    script_element_vec elements;
    env["element"] = make_func([&](std::string const& name, table const& table) {
        elements.emplace_back(elements.size(), name, table);
    });
    _script.set_environment(env);

    std::ignore = _script.run_file("elements.lua");

    return elements;
}

////////////////////////////////////////////////////////////
