// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "WizardScene.hpp"

#include "games/Games.hpp"
#include "ui/Styles.hpp"

namespace solitaire {

wizard_scene::wizard_scene(game& game, color_themes const& currentTheme)
    : scene {game}
    , _currentTheme {currentTheme}
{
    auto& resMgr {locate_service<assets::library>()};
    auto& resGrp {resMgr.create_or_get_group("solitaire")};

    _formWizard = std::make_shared<form_wizard>(&get_window(), resGrp);

    styles     styles {resGrp};
    auto const styleCollection {styles.load(_currentTheme)};
    _formWizard->Styles = styleCollection;

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

        auto const name {obj["Name"]["text"].as<string>()};
        auto const wizgame {(*func)(obj)};

        _formWizard->set_log_messages(wizgame.second);

        if (wizgame.second.empty()) {
            io::create_folder("custom");
            auto const file {"custom/games.wizard_" + name + ".lua"};
            {
                io::ofstream str {file};
                str.write(wizgame.first);
            }

            GameGenerated({.Name = "Wizard_" + name, .Path = file});
            // TODO: save settings
            get_game().pop_current_scene();
        }
    });
}

void wizard_scene::on_start()
{
    get_root_node()->create_child()->attach_entity(_formWizard);
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
