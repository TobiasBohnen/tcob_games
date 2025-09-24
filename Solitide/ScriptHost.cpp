// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ScriptHost.hpp"

#include <utility>

#include "games/Games.hpp"

namespace solitaire {
////////////////////////////////////////////////////////////

script_host::script_host(std::shared_ptr<sources> sources)
    : _sources {std::move(sources)}
{
    load_scripts();
}

auto script_host::run_file(path const& path) -> bool
{
    return _luaScript.run_file(path).has_value();
}

void script_host::register_game(game_info const& info, reg_game_func&& game)
{
    if (info.DeckCount > 24) { return; }                                                   // TODO: error
    if (info.DeckCount * info.DeckSuits.size() * info.DeckSuits.size() > 1500) { return; } // TODO: error
    if (_sources->Games.size() > 2500) { return; }                                         // TODO: error

    _sources->Games[info.Name] = {info, std::move(game)};
}

void script_host::load_scripts()
{
    lua_script_game::CreateAPI(this, _luaScript, _luaFunctions);
    auto const files {io::enumerate("/", {.String = "games.*.lua", .MatchWholePath = false}, true)};
    for (auto const& file : files) {
        std::ignore = _luaScript.run_file(file);
    }
}

}
