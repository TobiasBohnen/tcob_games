// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

////////////////////////////////////////////////////////////

class dmd_proxy {
public:
    dmd_proxy(shared_state& state);

    void clear();

    void blit(rect_i const& rect, string const& dotStr);

private:
    shared_state& _sharedState;
};

////////////////////////////////////////////////////////////

class engine {
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

public:
    engine(base_game& game, shared_state& state);

    void run(string const& file);

    auto update(milliseconds deltaTime) -> bool;
    auto start_turn() -> bool;

private:
    void create_env(string const& path);

    void create_wrappers();
    void create_sprite_wrapper();
    void create_slot_wrapper();
    void create_engine_wrapper();
    void create_dmd_wrapper();

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

    base_game&    _game;
    shared_state& _sharedState;
    callbacks     _callbacks;

    bool _running {false};

    dmd_proxy                        _dmdProxy;
    std::unordered_map<u32, texture> _textures;
};
