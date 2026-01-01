// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

#include "Sources.hpp"
#include "Themes.hpp"      // IWYU pragma: keep
#include "games/Games.hpp"
#include "gfx/CardSet.hpp" // IWYU pragma: keep

namespace solitaire {
////////////////////////////////////////////////////////////

class form_controls : public form<dock_layout> {
public:
    form_controls(gfx::window& window, assets::group& resGrp, std::shared_ptr<sources> sources);

    void set_pile_labels(pile const* pile, data::object const& currentRules, game_state const& state);
    void set_game_labels(base_game* game);

private:
    void set_pile_labels(pile_description const& str);

    label* _lblPile;
    label* _lblPileLabel;

    label* _lblGameName;

    label* _lblDescription;
    label* _lblDescriptionLabel;

    label* _lblMove;
    label* _lblMoveLabel;

    label* _lblBase;
    label* _lblBaseLabel;

    label* _lblCardCount;
    label* _lblCardCountLabel;

    label* _lblTurns;
    label* _lblTurnsLabel;

    label* _lblScore;
    label* _lblScoreLabel;

    label* _lblTime;
    label* _lblTimeLabel;

    std::shared_ptr<sources> _sources;
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class form_menu : public form<dock_layout> {
public:
    form_menu(gfx::window& window, assets::group& resGrp, std::shared_ptr<sources> sources);

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

    text_box* _txbSeed {nullptr};

    std::shared_ptr<tooltip> _tooltip;

    assets::group&           _resGrp;
    std::shared_ptr<sources> _sources;
};

}
