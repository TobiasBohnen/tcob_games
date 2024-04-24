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
    form_controls(gfx::window* window);

    std::shared_ptr<button> BtnNewGame;
    std::shared_ptr<button> BtnMenu;
    std::shared_ptr<button> BtnHint;
    std::shared_ptr<button> BtnUndo;
    std::shared_ptr<button> BtnQuit;

    std::shared_ptr<label> LblPile;
    std::shared_ptr<label> LblPileLabel;

    std::shared_ptr<label> LblGameName;

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
    form_menu(gfx::window* window, start_scene const& scene);

    prop<std::string> SelectedGame;
    prop<std::string> SelectedTheme;
    prop<std::string> SelectedCardset;

    std::shared_ptr<button> BtnApplySettings;

    void submit_settings(data::config::object& obj);
    void set_game_stats(game_history const& stats);

private:
    void create_section_games(std::vector<game_info> const& games);
    void create_section_settings();
    void create_section_themes(std::vector<std::string> const& colorThemes);
    void create_section_cardset(std::vector<std::string> const& cardSets);

    void create_menubar();

    std::shared_ptr<panel> _panelSettings;

    std::shared_ptr<list_box> _lbxGamesByName;
    std::shared_ptr<list_box> _lbxThemes;
    std::shared_ptr<list_box> _lbxCardsets;

    std::shared_ptr<grid_view> _gvWL;
    std::shared_ptr<grid_view> _gvTT;
    std::shared_ptr<grid_view> _gvHistory;
};

}
