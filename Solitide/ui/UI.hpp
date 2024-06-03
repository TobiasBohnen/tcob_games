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
    std::shared_ptr<button> BtnWizard;

    std::shared_ptr<button> BtnNewGame;

    std::shared_ptr<button> BtnHint;
    std::shared_ptr<button> BtnCollect;
    std::shared_ptr<button> BtnUndo;

    std::shared_ptr<button> BtnQuit;

    std::shared_ptr<canvas_widget> Canvas;

    void set_pile_labels(pile_description const& str);
    void set_game_labels(base_game* game);

private:
    std::shared_ptr<label> _lblPile;
    std::shared_ptr<label> _lblPileLabel;

    std::shared_ptr<label> _lblGameName;

    std::shared_ptr<label> _lblDescription;
    std::shared_ptr<label> _lblDescriptionLabel;

    std::shared_ptr<label> _lblMove;
    std::shared_ptr<label> _lblMoveLabel;

    std::shared_ptr<label> _lblBase;
    std::shared_ptr<label> _lblBaseLabel;

    std::shared_ptr<label> _lblCardCount;
    std::shared_ptr<label> _lblCardCountLabel;

    std::shared_ptr<label> _lblTurns;
    std::shared_ptr<label> _lblTurnsLabel;

    std::shared_ptr<label> _lblScore;
    std::shared_ptr<label> _lblScoreLabel;

    std::shared_ptr<label> _lblTime;
    std::shared_ptr<label> _lblTimeLabel;
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

struct menu_sources {
    settings Settings;

    game_map           Games;
    signal<>           GameAdded; // wizard
    prop<game_history> CurrentHistory;

    theme_map   Themes;
    cardset_map Cardsets;
};

class form_menu : public form {
public:
    form_menu(gfx::window* window, assets::group& resGrp, std::shared_ptr<menu_sources> sources);

    signal<std::string const> StartGame;
    signal<>                  VideoSettingsChanged;

    void submit_settings(data::config::object& obj);

private:
    void create_section_games();
    void create_section_settings();
    void create_section_themes();
    void create_section_cardset();

    void create_menubar();

    std::shared_ptr<tab_container> _tabSettings;
    std::shared_ptr<text_box>      _txbSeed;

    std::shared_ptr<tooltip> _tooltip;

    assets::group&                _resGrp;
    std::shared_ptr<menu_sources> _sources;
};

}
