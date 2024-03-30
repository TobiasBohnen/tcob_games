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
    std::shared_ptr<button> BtnHint;
    std::shared_ptr<button> BtnUndo;
    std::shared_ptr<button> BtnQuit;

    std::shared_ptr<label> LblPile;
    std::shared_ptr<label> LblPileLabel;

    std::shared_ptr<label> LblDescription;
    std::shared_ptr<label> LblDescriptionLabel;

    std::shared_ptr<label> LblMove;
    std::shared_ptr<label> LblMoveLabel;

    std::shared_ptr<label> LblBase;
    std::shared_ptr<label> LblBaseLabel;

    std::shared_ptr<label> LblCardCount;
    std::shared_ptr<label> LblCardCountLabel;

    std::shared_ptr<label> LblTurn;
    std::shared_ptr<label> LblTurnLabel;

    std::shared_ptr<label> LblTime;
    std::shared_ptr<label> LblTimeLabel;

    std::shared_ptr<canvas_widget> Canvas;
};

////////////////////////////////////////////////////////////

class form_menu : public form {
public:
    form_menu(gfx::window* window, rect_f bounds,
              std::vector<games::game_info> const& games, std::vector<std::string> const& colorThemes, std::vector<std::string> const& cardSets);

    prop<std::string> SelectedGame;
    prop<std::string> SelectedTheme;
    prop<std::string> SelectedCardset;
};

}
