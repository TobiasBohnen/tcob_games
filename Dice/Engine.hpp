// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

struct sprite {
    gfx::rect_shape* Shape {};
    gfx::rect_shape* WrapCopy {};
};
struct texture {
    size_i Size;
    string Region;
};

class engine {
public:
    engine(base_game& game);

    void run(string const& file);

    void update(milliseconds deltaTime);

private:
    void create_env(string const& path);
    void create_wrappers();
    auto create_gfx() -> bool;
    auto create_sfx() -> bool;

    scripting::script                                 _script;
    scripting::table                                  _table;
    std::vector<scripting::native_closure_shared_ptr> _funcs;

    gfx::canvas _canvas;

    base_game& _game;
};