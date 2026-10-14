// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

////////////////////////////////////////////////////////////

class engine {
public:
    engine();
    ~engine();

    void run(string const& file);

    void generate_level(level& level, player& player);

    auto update(milliseconds deltaTime) -> bool;

private:
    void create_env();

    void create_wrappers();

    scripting::script                                 _script;
    scripting::table                                  _table;
    std::vector<scripting::native_closure_unique_ptr> _funcs;

    rng _rng;
};
