// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "Sources.hpp"

namespace solitaire {

class script_host {
public:
    script_host(std::shared_ptr<sources> sources);

    void register_game(game_info const& info, reg_game_func&& game);
    auto run_file(path const& path) -> bool;

private:
    void load_scripts();

    scripting::script                                 _luaScript;
    std::vector<scripting::native_closure_shared_ptr> _luaFunctions;

    std::shared_ptr<sources> _sources;
};

}
