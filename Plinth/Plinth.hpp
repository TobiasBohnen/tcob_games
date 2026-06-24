// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <memory>

#include "Cache.hpp"
#include "Common.hpp"
#include "Player.hpp"
#include "Raycaster.hpp"

class Plinth final : public scene {
public:
    explicit Plinth(game& game);
    ~Plinth() override;

    void on_start() override;

    void on_draw_to(gfx::render_target& target, transform const& xform) override;
    void on_fixed_update(milliseconds deltaTime) override;
    void on_update(milliseconds deltaTime) override;

    void on_key_down(input::keyboard::event const& ev) override;
    void on_controller_button_down(input::controller::button_event const& ev) override;

private:
    void draw();
    auto move(milliseconds deltaTime) -> bool;

    std::unique_ptr<cache>     _cache;
    std::unique_ptr<raycaster> _raycaster;
    std::unique_ptr<level>     _level;
    player                     _player;

    asset_owner_ptr<gfx::material> _material;
    asset_owner_ptr<gfx::texture>  _texture;
    gfx::renderer                  _renderer {gfx::buffer_usage_hint::StaticDraw};

    size_i _screen {};
    bool   _update {true};
};
