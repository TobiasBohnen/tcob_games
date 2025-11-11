// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

////////////////////////////////////////////////////////////

struct sprite {
    gfx::rect_shape* Shape {};
    gfx::rect_shape* WrapCopy {};

    string Type;
    usize  Index {0};

    u32  TexID {0};
    bool IsCollisionEnabled {false};
};

struct texture {
    size_i   Size;
    string   Region;
    grid<u8> Alpha;
};

////////////////////////////////////////////////////////////

struct script_assets {
    std::unordered_map<u32, texture>     Textures;
    asset_owner_ptr<gfx::material>       SpriteMaterial;
    std::vector<std::unique_ptr<sprite>> Sprites;

    gfx::rect_shape*               Background;
    asset_owner_ptr<gfx::material> BackgroundMaterial;
};

////////////////////////////////////////////////////////////

class engine {
public:
    engine(base_game& game, script_assets& assets);

    void run(string const& file);

    auto update(milliseconds deltaTime) -> bool;
    auto end_turn() -> bool;
    void collision(sprite* a, sprite* b);

private:
    void create_env(string const& path);
    void create_wrappers();
    auto create_gfx() -> bool;
    auto create_sfx() -> bool;

    template <typename R = void>
    auto call(string const& name, auto&&... args) -> R;

    rng _rng;

    scripting::script                                 _script;
    scripting::table                                  _table;
    std::vector<scripting::native_closure_shared_ptr> _funcs;

    gfx::canvas _canvas;

    base_game&     _game;
    script_assets& _assets;
};
