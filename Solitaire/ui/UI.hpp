// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep
#include "Games.hpp"

namespace solitaire {
////////////////////////////////////////////////////////////

class form_controls : public form {
public:
    form_controls(gfx::window* window, rect_f bounds);

    std::shared_ptr<button> BtnNewGame;
    std::shared_ptr<button> BtnMenu;
    std::shared_ptr<label>  LblPile;
    std::shared_ptr<label>  LblRule;
    std::shared_ptr<label>  LblCardCount;
    std::shared_ptr<button> BtnUndo;
    std::shared_ptr<button> BtnQuit;
};

////////////////////////////////////////////////////////////

class form_menu : public form {
public:
    form_menu(gfx::window* window, rect_f bounds,
              std::vector<games::game_info> const& games, std::vector<std::string> const& colorThemes);

    prop<std::string> SelectedGame;
    prop<std::string> SelectedTheme;
};

}
