// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "WizardScene.hpp"

#include "games/Games.hpp"
#include "ui/Styles.hpp"

namespace solitaire {

wizard_scene::wizard_scene(game& game)
    : scene {game}
{
    auto& resGrp {*library().get_group("solitaire")};

    _formWizard = std::make_shared<form_wizard>(&window(), resGrp);

    _formWizard->BtnBack->Click.connect([&]() {
        parent().pop_current_scene();
    });

    _formWizard->BtnGenerate->Click.connect([&]() {
        using namespace tcob::scripting::lua;

        script script;
        lua_script_game::CreateENV(script);

        auto func {script.run_file<function<std::tuple<std::string, std::vector<std::string>, std::string>>>("scripts/wizard.lua")};
        if (!func) { return; } // ERROR loading wizard func

        table obj {script.create_table()};
        _formWizard->submit(obj);

        auto const name {obj["Name"]["text"].as<std::string>()};
        auto const [gameStr, log, gameName] {(*func)(obj)};

        _formWizard->set_log_messages(log);

        if (log.empty()) {
            io::create_folder("custom");
            auto const file {"custom/games.wizard_" + name + ".lua"};
            {
                io::ofstream str {file};
                str.write(gameStr);
            }

            GameGenerated({.Name = gameName, .Path = file});
            parent().pop_current_scene();
        }
    });
}

void wizard_scene::update_theme(color_themes const& currentTheme)
{
    styles     styles {*library().get_group("solitaire")};
    auto const styleCollection {styles.load(currentTheme)};
    _formWizard->Styles = styleCollection;
}

void wizard_scene::on_start()
{
    root_node()->create_child()->Entity = _formWizard;
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

void wizard_scene::on_key_down(input::keyboard::event const& ev)
{
    switch (ev.ScanCode) {
    case input::scan_code::BACKSPACE:
        parent().pop_current_scene();
        break;
    default:
        break;
    }
}

}
