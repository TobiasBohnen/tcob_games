// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "games/Games.hpp"

namespace solitaire {

class database {
public:
    database();

    void insert_games(std::vector<game_info> const& games) const;

    void insert_history_entry(string const& name, game_state const& state, game_rng const& rng, game_status status) const;
    auto get_history(string const& name) const -> game_history;

private:
    data::sqlite::database             _database;
    std::optional<data::sqlite::table> _tabGames;
    std::optional<data::sqlite::table> _tabHistory;
};

}
