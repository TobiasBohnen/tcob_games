// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep
#include "Sources.hpp"
#include "games/GameInfo.hpp"

namespace solitaire {

////////////////////////////////////////////////////////////

class database {
public:
    database();

    void insert_games(game_map const& games) const;

    void insert_history_entry(std::string const& name, game_state const& state, game_rng const& rng, game_status status) const;
    auto get_history(std::string const& name) const -> game_history;

private:
    db::database             _database;
    std::optional<db::table> _tabGames;
    std::optional<db::table> _tabHistory;
};

}
