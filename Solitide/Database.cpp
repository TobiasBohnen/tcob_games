// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Database.hpp"

#include "games/Games.hpp"

namespace solitaire {

static char const* DB_NAME {"profile.db"};
namespace db = tcob::data::sqlite;

auto static state_to_string(std::array<u64, 4> const& state) -> std::string // NOLINT
{
    std::vector<ubyte> bytes;
    bytes.reserve(state.size() * sizeof(u64));
    for (auto const& elem : state) {
        auto const* ptr = reinterpret_cast<ubyte const*>(&elem);
        bytes.insert(bytes.end(), ptr, ptr + sizeof(u64));
    }
    auto base64 {io::base64_filter {}.to(bytes)};
    return {reinterpret_cast<byte*>(base64->data()), base64->size()};
}

database::database()
    : _database {*data::sqlite::database::Open(DB_NAME)}
{
    _dbGames   = _database.create_table("games",
                                        db::column {"ID", db::type::Integer, false, db::primary_key {}},
                                        db::column {"Name", db::type::Text, true, db::unique {}},
                                        db::column {"Family", db::type::Text},
                                        db::column {"DeckCount", db::type::Integer});
    _dbHistory = _database.create_table("history",
                                        db::column {"ID", db::type::Integer, false, db::primary_key {}},
                                        db::column {"GameID", db::type::Integer},
                                        db::column {"Seed", db::type::Text, true},
                                        db::column {"Won", db::type::Integer},
                                        db::column {"Turns", db::type::Integer},
                                        db::column {"Score", db::type::Integer},
                                        db::column {"Time", db::type::Real},
                                        db::unique {"GameID", "Seed"});
    assert(_dbGames && _dbHistory);
}

void database::insert_games(game_map const& games) const
{
    std::vector<std::tuple<string, family, u8>> dbvalues;
    dbvalues.reserve(games.size());
    for (auto const& gi : games) {
        dbvalues.emplace_back(gi.first, gi.second.first.Family, gi.second.first.DeckCount);
    }

    std::ignore = _dbGames->insert_into(db::ignore, "Name", "Family", "DeckCount")(dbvalues);
}

void database::insert_history_entry(string const& name, game_state const& state, game_status status) const
{
    auto const id {_dbGames->select_from<i64>("ID").where("Name = ?")(name)};
    auto const seed {state_to_string(state.Seed)};
    std::ignore = _dbHistory->insert_into(db::ignore, "GameID", "Seed")(std::tuple<i64, string> {id[0], seed});
    std::ignore = _dbHistory->update("Won", "Turns", "Score", "Time").where("GameID = ? and Seed = ?")(status == game_status::Success, state.Turns, state.Score, state.Time.count(), id[0], seed);
}

auto database::get_history(string const& name) const -> game_history
{
    auto const  id {_dbGames->select_from<i64>("ID").where("Name = ?")(name)};
    usize const lost {_dbHistory->select_from<i64>("ID").where("GameID = ? and Won = 0")(id).size()};
    usize const won {_dbHistory->select_from<i64>("ID").where("GameID = ? and Won = 1")(id).size()};
    auto const  entries {_dbHistory->select_from<i64, i64, i64, i64, bool>("ID", "Turns", "Score", "Time", "Won").where("GameID = ?").order_by("ID").exec<game_history::entry>(id)};
    return {.Won = won, .Lost = lost, .Entries = entries};
}

}
