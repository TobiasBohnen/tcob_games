// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

////////////////////////////////////////////////////////////

template <typename T = void>
using callback = std::optional<scripting::function<T>>;

struct callbacks {
    callback<>     OnCollision;
    callback<>     OnSlotDieChanged;
    callback<>     OnSetup;
    callback<bool> OnRun;
    callback<>     OnFinish;
    callback<bool> CanStart;
    callback<>     OnStart;
};

////////////////////////////////////////////////////////////

class engine {
public:
    engine(base_game& game, shared_state& state);

    void run(string const& file);

    auto update(milliseconds deltaTime) -> bool;
    auto start_turn() -> bool;

private:
    void create_env(string const& path);
    void create_wrappers();
    void create_canvas_wrapper();
    void create_sprite_wrapper();
    void create_slot_wrapper();
    void create_engine_wrapper();
    auto create_gfx() -> bool;
    auto create_sfx() -> bool;

    template <typename R = void>
    auto call(callback<R> const& func, auto&&... args) -> R;

    auto normal_to_world(point_f pos) const -> point_f;
    auto world_to_normal(point_f pos) const -> point_f;

    void set_texture(sprite* sprite, u32 texID);

    scripting::script                                 _script;
    scripting::table                                  _table;
    std::vector<scripting::native_closure_shared_ptr> _funcs;

    gfx::canvas                      _canvas;
    asset_owner_ptr<gfx::material>   _spriteMaterial; // TODO: move to game
    std::unordered_map<u32, texture> _textures;       // TODO: move to game

    base_game&    _game;
    shared_state& _sharedState;
    callbacks     _callbacks;

    bool _running {false};
};
