// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "games/GameInfo.hpp"
#include "ui/Translator.hpp"

namespace solitaire {

struct menu_sources {
    settings Settings;

    game_map     Games;
    theme_map    Themes;
    card_set_map CardSets;

    translator Translator;

    // wizard
    signal<> GameAdded;

    // controls
    signal<> ShowMenu;
    signal<> ShowWizard;
    signal<> RestartGame;
    signal<> ShowHint;
    signal<> Collect;
    signal<> Undo;
    signal<> Quit;

    // menu
    signal<std::optional<u64> const> StartGame;
    signal<>                         VideoSettingsChanged;

    prop<std::string>  SelectedGame;
    prop<game_history> SelectedHistory;
    prop<data::object> SelectedRules;
};

}
