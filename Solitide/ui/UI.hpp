// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

#include "Themes.hpp"      // IWYU pragma: keep
#include "games/Games.hpp"
#include "gfx/CardSet.hpp" // IWYU pragma: keep
#include "ui/Sources.hpp"

namespace solitaire {
////////////////////////////////////////////////////////////

class form_controls : public form<dock_layout> {
public:
    form_controls(gfx::window& window, assets::group& resGrp, std::shared_ptr<menu_sources> sources);

    void set_pile_labels(pile const* pile, data::object const& currentRules, game_state const& state);
    void set_game_labels(base_game* game);

private:
    void set_pile_labels(pile_description const& str);

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

    std::shared_ptr<menu_sources> _sources;
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class form_menu : public form<dock_layout> {
public:
    form_menu(gfx::window& window, assets::group& resGrp, std::shared_ptr<menu_sources> sources);

protected:
    void on_key_down(input::keyboard::event const& ev) override;

private:
    void create_section_games(tab_container& parent);
    void create_game_lists(dock_layout& panelLayout);
    void create_game_details(dock_layout& panelLayout);

    void create_section_settings(tab_container& parent);
    void create_settings_video(tab_container& tabContainer);
    void create_settings_hints(tab_container& tabContainer);

    void create_section_themes(tab_container& parent);
    void create_section_cardset(tab_container& parent);

    void create_menubar(tab_container& parent);

    void start_game();

    std::shared_ptr<text_box> _txbSeed;

    std::shared_ptr<tooltip> _tooltip;

    assets::group&                _resGrp;
    std::shared_ptr<menu_sources> _sources;
    gfx::window&                  _window;
};

}
