// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "Sprite.hpp"
#include "TexProxy.hpp"

////////////////////////////////////////////////////////////

template <typename T = void>
using callback = std::optional<scripting::function<T>>;

struct callbacks {
    callback<>    OnSetup;
    callback<i32> OnTurnUpdate;
    callback<>    OnTurnFinish;
    callback<>    OnTurnStart;
    callback<>    OnDrawHUD;
    callback<>    OnCollision;
};

////////////////////////////////////////////////////////////

enum game_status : u8 {
    Running  = 0,
    Waiting  = 1,
    GameOver = 2
};

////////////////////////////////////////////////////////////

class engine {
public:
    struct init {
        shared_state& State;
        event_bus&    Events;

        dice_game* Game {nullptr};
    };

    engine(init const& init);

    void run(string const& file);

    auto update(milliseconds deltaTime) -> bool;

private:
    auto start_turn() -> bool;

    void create_env();

    void create_wrappers();
    void create_sprite_wrapper();
    void create_socket_wrapper();
    void create_engine_wrapper();
    void create_tex_wrapper();

    template <typename R = void>
    auto call(callback<R> const& func, auto&&... args) -> R;

    void define_texture(u32 id, rect_i const& uv);
    void update_hud();

    scripting::script                                 _script;
    scripting::table                                  _table;
    std::vector<scripting::native_closure_unique_ptr> _funcs;
    callbacks                                         _callbacks;

    init        _init;
    game_status _gameStatus {game_status::Waiting};
    bool        _updateHUD {false};

    tex_proxy _hudProxy;
    tex_proxy _fgProxy;
    tex_proxy _bgProxy;
    tex_proxy _texProxy;

    std::unordered_map<u32, texture> _textures;

    std::unordered_map<u32, audio::buffer> _soundBank;
    std::array<audio::sound_channel, 8>    _soundChannels;

    f64 _turnTime {0};
};
