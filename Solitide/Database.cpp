// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Database.hpp"

namespace solitaire {

static char const* DB_NAME {"profile.db"};

database::database()
    : _database {*db::database::Open(DB_NAME)}
{
    _database.set_journal_mode(db::journal_mode::Off);

    _tabGames   = _database.create_table("games",
                                         db::int_column<db::primary_key> {.Name = "ID", .NotNull = false},
                                         db::text_column<db::unique> {.Name = "Name", .NotNull = true});
    _tabHistory = _database.create_table("history",
                                         db::int_column<db::primary_key> {.Name = "ID", .NotNull = false},
                                         db::int_column<> {.Name = "GameID"},
                                         db::text_column<> {.Name = "Seed", .NotNull = true},
                                         db::int_column<> {.Name = "Won"},
                                         db::int_column<> {.Name = "Turns"},
                                         db::int_column<> {.Name = "Score"},
                                         db::int_column<> {.Name = "Undos"},
                                         db::int_column<> {.Name = "Hints"},
                                         db::real_column<> {.Name = "Time"},
                                         db::unique {"GameID", "Seed"},
                                         db::foreign_key {.Column = "GameID", .ForeignTable = "games", .ForeignColumn = "ID"});
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
    i64 const    id {_tabGames->select_from<i64>("ID").where(db::equal {"Name", name})()[0]};
    string const seed {std::to_string(rng.seed())};

    [[maybe_unused]] bool check {
        _tabHistory->insert_into(db::ignore, "GameID", "Seed") //
        (std::tuple {id, seed})};
    assert(check);
    check = _tabHistory->update("Won", "Turns", "Score", "Undos", "Hints", "Time")
                .where(db::equal {"GameID", id} && db::equal {"Seed", seed}) //
            (status == game_status::Success, state.Turns, state.Score, state.Undos, state.Hints, state.Time.count());
    assert(check);
}

auto database::get_history(std::string const& name) const -> game_history
{
    i64 const  id {_tabGames->select_from<i64>("ID").where(db::equal {"Name", name})()[0]};
    auto const entries {
        _tabHistory->select_from<i64, rng::seed_type, i64, i64, i64, i64, i64, bool>("ID", "Seed", "Turns", "Score", "Undos", "Hints", "Time", "Won")
            .where(db::equal {"GameID", id})
            .order_by(db::ordering {"Seed"})
            .exec<game_history::entry>()};
    isize const won {std::ranges::count_if(entries, [](auto const& entry) { return entry.Won; })};
    return {.Won = won, .Lost = std::ssize(entries) - won, .Entries = entries};
}

}
