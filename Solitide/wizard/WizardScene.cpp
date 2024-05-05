// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "WizardScene.hpp"

#include "games/Games.hpp"

namespace solitaire {

wizard_scene::wizard_scene(game& game, color_themes const& currentTheme)
    : scene {game}
    , _currentTheme {currentTheme}
{
}

void wizard_scene::on_start()
{
    auto& resMgr {locate_service<assets::library>()};
    auto& resGrp {resMgr.create_or_get_group("solitaire")};

    _formWizard = std::make_shared<form_wizard>(&get_window(), resGrp);
    get_root_node()->create_child()->attach_entity(_formWizard);
    create_styles(_currentTheme, resGrp, *_formWizard->Styles);

    _formWizard->BtnBack->Click.connect([&]() {
        get_game().pop_current_scene();
    });

    _formWizard->BtnGenerate->Click.connect([&]() {
        using namespace tcob::scripting::lua;

        script script;
        lua_script_game::CreateENV(script);

        auto func {script.run_file<function<std::pair<string, std::vector<string>>>>("wizard.lua")};
        if (!func) { return; } // ERROR loading wizard func

        table obj {script.create_table()};
        _formWizard->submit(obj);
        auto const name {obj["txtName"]["text"].as<string>()};
        auto const game {(*func)(obj)};

        _formWizard->set_log_messages(game.second);

        if (game.second.empty()) {
            io::create_folder("custom");
            auto const file {"custom/games.wizard_" + name + ".lua"};
            {
                io::ofstream str {file};
                str.write(game.first);
            }

            GameGenerated({.Name = "Wizard_" + name, .Path = file});
        }

        // get_game().pop_current_scene();
    });
}

void wizard_scene::on_draw_to(gfx::render_target&)
{
}

void wizard_scene::on_update(milliseconds)
{
}

void wizard_scene::on_fixed_update(milliseconds)
{
}

void wizard_scene::on_key_down(input::keyboard::event& ev)
{
    switch (ev.ScanCode) {
    case input::scan_code::BACKSPACE:
        get_game().pop_current_scene();
        break;
    default:
        break;
    }
}

}
