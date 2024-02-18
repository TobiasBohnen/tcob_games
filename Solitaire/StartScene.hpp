// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep
#include "Field.hpp"
#include "Games.hpp"
#include "ui/UI.hpp"

#include <functional>

namespace solitaire {

class start_scene : public scene {
public:
    using func = std::function<std::shared_ptr<games::base_game>(field&)>;

    start_scene(game& game);
    ~start_scene() override;

    void register_game(games::game_info const& info, func&& game);

protected:
    void on_start() override;

    void on_draw_to(gfx::render_target& target) override;

    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_key_down(input::keyboard::event& ev) override;

    void on_mouse_motion(input::mouse::motion_event& ev) override;
    void on_mouse_wheel(input::mouse::wheel_event& ev) override;

private:
    void load_scripts();

    void connect_ui_events();

    scripting::lua::script                                 _script;
    std::vector<scripting::lua::native_closure_shared_ptr> _functions;

    input::mouse::button       _buttonDown {input::mouse::button::None};
    std::shared_ptr<field>     _playField {};
    std::shared_ptr<main_menu> _mainForm {};
    std::shared_ptr<game_list> _gameForm {};

    std::map<std::string, std::pair<games::game_info, func>> _games {};
};

}
