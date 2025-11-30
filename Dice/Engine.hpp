// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

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
        callback<bool> Update;
        callback<>     Finish;
        callback<bool> CanStart;
        callback<>     Start;
    };

public:
    struct init {
        base_game&    Game;
        shared_state& State;
    };

    engine(init const& init);

    void run(string const& file, gfx::texture* sprTexture, gfx::texture* bgTexture);

    auto update(milliseconds deltaTime) -> bool;
    auto start_turn() -> bool;

private:
    void create_env(string const& path);

    void create_wrappers();
    void create_sprite_wrapper();
    void create_slot_wrapper();
    void create_engine_wrapper();
    void create_dmd_wrapper();
    void create_sfx_wrapper();

    auto create_gfx(gfx::texture* sprTexture, gfx::texture* bgTexture) -> bool;
    auto create_sfx() -> bool;

    template <typename R = void>
    auto call(callback<R> const& func, auto&&... args) -> R;

    auto normal_to_world(point_f pos) const -> point_f;
    auto world_to_normal(point_f pos) const -> point_f;

    void set_texture(sprite* sprite, u32 texID);

    scripting::script                                 _script;
    scripting::table                                  _table;
    std::vector<scripting::native_closure_shared_ptr> _funcs;

    init      _init;
    callbacks _callbacks;

    bool _running {false};

    dmd_proxy _dmdProxy;
    sfx_proxy _sfxProxy;

    std::unordered_map<u32, texture>                       _textures;
    std::unordered_map<u32, std::unique_ptr<audio::sound>> _sounds;
};
