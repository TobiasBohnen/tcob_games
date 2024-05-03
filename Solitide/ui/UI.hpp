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
    form_controls(gfx::window* window, assets::group& resGrp);

    std::shared_ptr<button> BtnMenu;
    std::shared_ptr<button> BtnNewGame;

    std::shared_ptr<button> BtnHint;
    std::shared_ptr<button> BtnCollect;
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

    std::shared_ptr<label> LblTurns;
    std::shared_ptr<label> LblTurnsLabel;

    std::shared_ptr<label> LblScore;
    std::shared_ptr<label> LblScoreLabel;

    std::shared_ptr<label> LblTime;
    std::shared_ptr<label> LblTimeLabel;

    std::shared_ptr<canvas_widget> Canvas;
};

////////////////////////////////////////////////////////////

struct menu_sources {
    std::vector<game_info>   Games;
    std::vector<std::string> Themes;
    std::vector<std::string> Cardsets;
};

class form_menu : public form {
public:
    form_menu(gfx::window* window, assets::group& resGrp, menu_sources const& source);

    prop<std::string> SelectedGame;
    prop<std::string> SelectedTheme;
    prop<std::string> SelectedCardset;

    prop<std::deque<std::string>> RecentGames;

    std::shared_ptr<button> BtnStartGame;
    std::shared_ptr<button> BtnApplySettings;

    void submit_settings(data::config::object& obj);
    void set_game_stats(game_history const& stats);

private:
    void create_section_games(assets::group& resGrp, std::vector<game_info> const& games);
    void create_section_settings(assets::group& resGrp);
    void create_section_themes(std::vector<std::string> const& colorThemes);
    void create_section_cardset(std::vector<std::string> const& cardSets);

    void create_menubar(assets::group& resGrp);

    std::shared_ptr<panel> _panelSettings;

    std::shared_ptr<grid_view> _gvWL;
    std::shared_ptr<grid_view> _gvTT;
    std::shared_ptr<grid_view> _gvHistory;

    std::shared_ptr<tooltip> _tooltip;
};

}
