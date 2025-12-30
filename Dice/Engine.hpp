// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep
#include "EngineHelper.hpp"

////////////////////////////////////////////////////////////

class engine {
public:
    struct init {
        shared_state& State;
        event_bus&    Events;

        dice_game* Game {nullptr};
        sockets*   Sockets {nullptr};
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
    void create_die_wrapper();
    void create_engine_wrapper();
    void create_dmd_wrapper();
    void create_screen_wrapper();

    template <typename R = void>
    auto call(callback<R> const& func, auto&&... args) -> R;

    void create_backgrounds(std::unordered_map<u32, bg_def> const& bgMap);
    void create_textures(std::unordered_map<u32, tex_def>& texMap);

    scripting::script                    _script;
    scripting::table                     _table;
    scripting::native_closure_shared_ptr _require;
    callbacks                            _callbacks;

    init        _init;
    game_status _gameStatus {game_status::TurnEnded};

    dmd_proxy    _dmdProxy;
    screen_proxy _fgProxy;

    std::unordered_map<u32, texture> _textures;
    static constexpr i32             TEX_PAD {1};
    point_i                          _texturePen {TEX_PAD, TEX_PAD};
    gfx::image                       _textureImage {gfx::image::CreateEmpty(TEXTURE_SIZE, gfx::image::format::RGBA)};

    std::unordered_map<u32, string> _backgrounds;
    u32                             _backgroundLevel {0};

    std::unordered_map<u32, audio::buffer>       _soundBank;
    std::array<std::unique_ptr<audio::sound>, 8> _sounds;
    u32                                          _soundIdx {0};

    f64 _turnTime {0};
};
