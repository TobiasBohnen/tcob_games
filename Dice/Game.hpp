// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "Die.hpp"
#include "Engine.hpp"
#include "Socket.hpp"
#include "SpriteManager.hpp"
#include "UI.hpp"

////////////////////////////////////////////////////////////

class dice_game : public gfx::entity {
public:
    struct init {
        std::vector<game_def::dice> Dice;
        assets::group&              Group;
        size_f                      RealWindowSize;
        event_bus&                  Events;
    };

    explicit dice_game(init init);
    ~dice_game();

    void run(string const& file);

protected:
    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_draw_to(gfx::render_target& target, transform const& xform) override;
    auto can_draw() const -> bool override;

    void on_key_down(input::keyboard::event const& ev) override;
    void on_mouse_button_up(input::mouse::button_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;
    void on_mouse_motion(input::mouse::motion_event const& ev) override;

private:
    auto get_die_position(usize count, usize idx) const -> point_f;

    gfx::shape_batch           _diceBatch;
    std::unique_ptr<game_form> _form0;

    asset_owner_ptr<gfx::material>       _screenMaterial;
    asset_owner_ptr<gfx::render_texture> _screenTexture;
    gfx::renderer                        _screenRenderer {gfx::buffer_usage_hint::StaticDraw};

    ui_state _uiState;

    rng            _rng;
    init           _init;
    sprite_manager _sprites;
    sockets        _sockets;
    dice           _dice;
    engine         _engine;

    die* _hoverDie {nullptr};
};
