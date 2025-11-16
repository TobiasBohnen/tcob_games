// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

////////////////////////////////////////////////////////////

struct shared_assets {
    std::unordered_map<u32, texture>     Textures;
    asset_owner_ptr<gfx::material>       SpriteMaterial;
    std::vector<std::unique_ptr<sprite>> Sprites;

    gfx::rect_shape*               Background;
    asset_owner_ptr<gfx::material> BackgroundMaterial;

    rect_f DiceArea {0.00f, 0.00f, 1.00f, 0.25f};

    rng Rng;
};

////////////////////////////////////////////////////////////

class engine {
public:
    engine(base_game& game, shared_assets& assets);

    void run(string const& file);

    auto update(milliseconds deltaTime) -> bool;
    auto start_turn() -> bool;

private:
    void create_env(string const& path);
    void create_wrappers();
    auto create_gfx() -> bool;
    auto create_sfx() -> bool;

    template <typename R = void>
    auto call(string const& name, auto&&... args) -> R;

    scripting::script                                 _script;
    scripting::table                                  _table;
    std::vector<scripting::native_closure_shared_ptr> _funcs;

    gfx::canvas _canvas;

    base_game&     _game;
    shared_assets& _assets;
};
