// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "FieldItems.hpp"
#include "Snake.hpp" // IWYU pragma: keep

////////////////////////////////////////////////////////////
namespace Snake {

class field : public gfx::entity {
public:
    field(asset_ptr<gfx::material> const& material, i32 windowHeight);

    prop<i32> Score;

    void start();
    void fail();

    auto state() const -> game_state;

    void play_sound(audio::sound_wave const& wave);
    void set_tile(point_i pos, gfx::tile_index_t idx);
    auto get_random_tile() -> point_i;

protected:
    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_draw_to(gfx::render_target& target) override;
    auto can_draw() const -> bool override;

    void on_key_down(input::keyboard::event const& ev) override;
    void on_controller_button_down(input::controller::button_event const& ev) override;

private:
    auto get_tile_size() const -> size_f;
    void tick();

    static constexpr milliseconds TICK {250ms};

    game_state _state {game_state::Initial};

    size_i const _gridSize {10, 10};
    i32          _windowHeight {0};

    food  _food {*this};
    star  _star {*this};
    bomb  _bomb {*this};
    snake _snake {*this, _gridSize};

    milliseconds _currentTime {0ms};

    rng                           _rng {};
    std::shared_ptr<audio::sound> _sound;

    gfx::orthogonal_tilemap _map;
    uid                     _layerBack {0};
    uid                     _layerFront {1};
};

}