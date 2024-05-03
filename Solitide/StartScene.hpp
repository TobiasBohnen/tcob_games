// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "CardTable.hpp"
#include "Cardset.hpp"
#include "Database.hpp"
#include "Games.hpp"
#include "ui/UI.hpp"
#include "ui/UIHelper.hpp"

namespace solitaire {

enum class start_reason {
    Restart,
    Resume
};

class start_scene : public scene {
public:
    start_scene(game& game);
    ~start_scene() override;

    void register_game(game_info const& info, reg_game_func&& game);
    auto call_lua(std::vector<std::string> const& funcs, lua_params const& args) -> lua_return;

protected:
    void on_start() override;

    void on_draw_to(gfx::render_target& target) override;

    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_key_down(input::keyboard::event& ev) override;

private:
    void set_children_bounds(size_i size);
    void start_game(string const& name, start_reason reason);

    void update_stats(string const& name) const;
    void update_recent(string const& name);

    void generate_rule(base_game const& game) const;

    void load_scripts();

    void connect_ui_events();

    scripting::lua::script                                      _luaScript;
    std::vector<scripting::lua::native_closure_shared_ptr>      _luaFunctions;
    scripting::squirrel::script                                 _sqScript;
    std::vector<scripting::squirrel::native_closure_shared_ptr> _sqFunctions;

    input::mouse::button           _buttonDown {input::mouse::button::None};
    std::shared_ptr<card_table>    _cardTable {};
    std::shared_ptr<form_controls> _formControls {};
    std::shared_ptr<form_menu>     _formMenu {};

    game_map                                        _games {};
    std::map<std::string, color_themes>             _themes {};
    std::map<std::string, std::shared_ptr<cardset>> _cardSets;

    data::config::object _saveGame;

    database _db;
};

}
