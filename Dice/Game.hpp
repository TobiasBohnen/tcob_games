// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "Engine.hpp"
#include "objects/Die.hpp"
#include "objects/Slot.hpp"

////////////////////////////////////////////////////////////

struct script_assets {
    std::unordered_map<u32, texture>     Textures;
    asset_owner_ptr<gfx::material>       SpriteMaterial;
    std::vector<std::unique_ptr<sprite>> Sprites;

    gfx::rect_shape*               Background;
    asset_owner_ptr<gfx::material> BackgroundMaterial;
};

////////////////////////////////////////////////////////////

class base_game : public gfx::entity {
    friend class engine;

public:
    base_game(gfx::window& window, assets::group const& grp);

    script_assets Assets;

    void run(string const& file);

    auto create_shape() -> gfx::rect_shape*;

protected:
    void on_update(milliseconds deltaTime) override;

    void on_draw_to(gfx::render_target& target) override;
    auto can_draw() const -> bool override;

    void on_key_down(input::keyboard::event const& ev) override;
    void on_mouse_button_up(input::mouse::button_event const& ev) override;
    void on_mouse_motion(input::mouse::motion_event const& ev) override;

private:
    void wrap_sprites();

    rng          _rng;
    gfx::window& _window;

    gfx::shape_batch _batch;

    slots _slots;
    slot* _hoverSlot {nullptr};
    dice  _dice;
    die*  _hoverDie {nullptr};

    engine _engine;
};
