// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep
#include "Sprite.hpp"

////////////////////////////////////////////////////////////

class dmd_proxy {
public:
    dmd_proxy(prop<grid<u8>>& dmd);

    void clear();

    void blit(rect_i const& rect, string const& dotStr);

private:
    prop<grid<u8>>& _dmd;
};

////////////////////////////////////////////////////////////

class sfx_proxy {
public:
    auto pickup_coin(u64 seed) -> audio::sound_wave;
    auto laser_shoot(u64 seed) -> audio::sound_wave;
    auto explosion(u64 seed) -> audio::sound_wave;
    auto powerup(u64 seed) -> audio::sound_wave;
    auto hit_hurt(u64 seed) -> audio::sound_wave;
    auto jump(u64 seed) -> audio::sound_wave;
    auto blip_select(u64 seed) -> audio::sound_wave;
    auto random(u64 seed) -> audio::sound_wave;
};

////////////////////////////////////////////////////////////

class engine {
    template <typename T = void>
    using callback = std::optional<scripting::function<T>>;

    struct callbacks {
        callback<>     OnCollision;
        callback<>     OnDieChanged;
        callback<>     OnSetup;
        callback<>     OnTeardown;
        callback<i32>  Update;
        callback<>     Finish;
        callback<bool> CanStart;
        callback<>     Start;
    };

public:
    struct init {
        shared_state& State;
        event_bus&    Events;

        base_game*    Game {nullptr};
        gfx::texture* SpriteTexture {nullptr};
        gfx::texture* BackgroundTexture {nullptr};
        slots*        Slots {nullptr};
        dice*         Dice {nullptr};
    };

    engine(init const& init);

    void run(string const& file);

    auto update(milliseconds deltaTime) -> bool;

private:
    auto start_turn() -> bool;

    void create_env();

    void create_wrappers();
    void create_sprite_wrapper();
    void create_slot_wrapper();
    void create_engine_wrapper();
    void create_dmd_wrapper();
    void create_sfx_wrapper();

    template <typename R = void>
    auto call(callback<R> const& func, auto&&... args) -> R;

    void set_texture(sprite* sprite, u32 texID);

    scripting::script                    _script;
    scripting::table                     _table;
    scripting::native_closure_shared_ptr _require;
    callbacks                            _callbacks;

    init        _init;
    game_status _gameStatus {game_status::TurnEnded};

    dmd_proxy _dmdProxy;
    sfx_proxy _sfxProxy;

    std::unordered_map<u32, texture>                       _textures;
    std::unordered_map<u32, std::unique_ptr<audio::sound>> _sounds;
};
