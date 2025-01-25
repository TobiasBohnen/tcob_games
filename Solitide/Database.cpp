// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Database.hpp"

namespace solitaire {

static char const* DB_NAME {"profile.db"};
namespace db = tcob::data::sqlite;

database::database()
    : _database {*data::sqlite::database::Open(DB_NAME)}
{
    _database.set_journal_mode(data::sqlite::journal_mode::Off);

    _tabGames   = _database.create_table("games",
                                         db::column {"ID", db::type::Integer, false, db::primary_key {}},
                                         db::column {"Name", db::type::Text, true, db::unique {}});
    _tabHistory = _database.create_table("history",
                                         db::column {"ID", db::type::Integer, false, db::primary_key {}},
                                         db::column {"GameID", db::type::Integer},
                                         db::column {"Seed", db::type::Text, true},
                                         db::column {"Won", db::type::Integer},
                                         db::column {"Turns", db::type::Integer},
                                         db::column {"Score", db::type::Integer},
                                         db::column {"Undos", db::type::Integer},
                                         db::column {"Hints", db::type::Integer},
                                         db::column {"Time", db::type::Real},
                                         db::unique {"GameID", "Seed"},
                                         db::foreign_key {"GameID", "games", "ID"});
    assert(_tabGames && _tabHistory);
}

void database::insert_games(game_map const& games) const
{
    std::vector<std::tuple<string>> dbvalues;
    dbvalues.reserve(games.size());
    for (auto const& gi : games) {
        dbvalues.emplace_back(gi.first);
    }

    auto sp {_database.create_savepoint("sp1")};
    std::ignore = _tabGames->insert_into(db::ignore, "Name")(dbvalues);
    sp.release();
}

void database::insert_history_entry(std::string const& name, game_state const& state, game_rng const& rng, game_status status) const
{
    auto sp {_database.create_savepoint("sp1")};

    auto const id {_tabGames->select_from<i64>("ID").where("Name = ?")(name)};
    auto const seed {std::to_string(rng.seed())};

    std::ignore = _tabHistory->insert_into(db::ignore, "GameID", "Seed")(std::tuple<i64, string> {id[0], seed});
    std::ignore = _tabHistory->update("Won", "Turns", "Score", "Undos", "Hints", "Time")
                      .where("GameID = ? and Seed = ?")(status == game_status::Success, state.Turns, state.Score, state.Undos, state.Hints, state.Time.count(), id[0], seed);

    sp.release();
}

auto database::get_history(std::string const& name) const -> game_history
{
    auto const id {_tabGames->select_from<i64>("ID").where("Name = ?")(name)};
    auto const entries {
        _tabHistory->select_from<i64, rng::seed_type, i64, i64, i64, i64, i64, bool>("ID", "Seed", "Turns", "Score", "Undos", "Hints", "Time", "Won")
            .where("GameID = ?")
            .order_by("Seed")
            .exec<game_history::entry>(id)};
    isize const won {std::ranges::count_if(entries, [](auto const& entry) { return entry.Won; })};
    return {.Won = won, .Lost = std::ssize(entries) - won, .Entries = entries};
}

}
