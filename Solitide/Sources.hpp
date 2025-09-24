// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "games/GameInfo.hpp"
#include "ui/Translator.hpp"

namespace solitaire {

struct video_settings {
    bool   FullScreen {false};
    bool   VSync {false};
    size_i Resolution;
};

struct ui_events {
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
    signal<video_settings const>     VideoSettingsChanged;
};

class settings {
public:
    settings() = default;

    std::string Version {"1.0.0"};

    prop<std::string> Theme {"default"};
    prop<std::string> Cardset {"default"};

    std::string                   LastGame;
    prop<std::deque<std::string>> Recent;

    bool HintMovable {true};
    bool HintTarget {true};

    static auto constexpr Members()
    {
        return std::tuple {
            member<&settings::Version> {"version"},
            member<&settings::Theme> {"theme"},
            member<&settings::Cardset> {"cardset"},
            member<&settings::LastGame> {"last_game"},
            member<&settings::Recent> {"recent"},
            member<&settings::HintMovable> {"hint_movable"},
            member<&settings::HintTarget> {"hint_target"}};
    }
};

using game_map     = std::map<std::string, std::pair<game_info, reg_game_func>>;
using theme_map    = std::map<std::string, color_themes>;
using card_set_map = std::map<std::string, std::shared_ptr<card_set>>;

struct sources {
    settings Settings;

    game_map     Games;
    theme_map    Themes;
    card_set_map CardSets;

    translator Translator;

    ui_events Events;

    prop<std::string>  SelectedGame;
    prop<game_history> SelectedHistory;
    prop<data::object> SelectedRules;
};

}
