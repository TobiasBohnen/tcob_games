// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "CardTable.hpp"
#include "Database.hpp"
#include "ui/UI.hpp"
#include "wizard/WizardScene.hpp"

namespace solitaire {

////////////////////////////////////////////////////////////

enum class start_reason {
    Restart,
    Resume
};

////////////////////////////////////////////////////////////

class main_scene : public scene {
public:
    main_scene(game& game);
    ~main_scene() override;

    void register_game(game_info const& info, reg_game_func&& game);
    auto call_lua(std::vector<std::string> const& funcs, lua_params const& args) -> lua_return;

protected:
    void on_start() override;

    void on_draw_to(gfx::render_target& target) override;

    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_key_down(input::keyboard::event const& ev) override;

private:
    void set_children_bounds(size_i size);
    void set_theme();
    void set_cardset();

    void start_game(std::string const& name, start_reason reason, std::optional<u64> seed);
    void start_wizard();

    void update_game_ui(std::string const& name) const;
    void update_recent(std::string const& name);

    auto generate_rule(std::string const& name) const -> data::config::object;

    void load_scripts();

    void connect_events();
    void save();

    scripting::lua::script                                      _luaScript;
    std::vector<scripting::lua::native_closure_shared_ptr>      _luaFunctions;
    scripting::squirrel::script                                 _sqScript;
    std::vector<scripting::squirrel::native_closure_shared_ptr> _sqFunctions;

    input::mouse::button           _buttonDown {input::mouse::button::None};
    std::shared_ptr<card_table>    _cardTable {};
    std::shared_ptr<form_controls> _formControls {};
    std::shared_ptr<form_menu>     _formMenu {};

    data::config::object _saveGame;
    data::config::object _currentRules;

    database _db;

    std::shared_ptr<wizard_scene> _wizard;
    std::shared_ptr<menu_sources> _sources;
};

}
