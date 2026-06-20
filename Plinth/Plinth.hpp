// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <memory>

#include "../_common/Common.hpp"
#include "Cache.hpp"
#include "Raycaster.hpp"

using world_map_t = static_grid<u8, /*mapWidth*/ 24, /*mapHeight*/ 24>;
using gfx_cache   = cache<{800, 600}, {256, 256}>;

class Plinth final : public scene { // uncertain: exact scene base class name/signature
public:
    explicit Plinth(game& game);
    ~Plinth() override;

    void on_start() override;

    void on_draw_to(gfx::render_target& target, transform const& xform) override;
    void on_fixed_update(milliseconds deltaTime) override;
    void on_update(milliseconds deltaTime) override;

    void on_key_down(input::keyboard::event const& ev) override;

private:
    void draw();
    auto move(milliseconds deltaTime) -> bool;

    std::unique_ptr<gfx_cache>                         _cache; // gfx_cache aliased in Plinth.cpp as cache<{1600,900},{256,256}>
    std::unique_ptr<raycaster<gfx_cache, world_map_t>> _raycaster;

    asset_owner_ptr<gfx::material> _material;
    asset_owner_ptr<gfx::texture>  _texture;
    gfx::renderer                  _renderer {gfx::buffer_usage_hint::StaticDraw};

    size_i _screen {};
    bool   _update {true};
};
