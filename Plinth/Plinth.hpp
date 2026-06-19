// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

// based on: https://lodev.org/cgtutor/raycasting.html Copyright (c) 2004-2019, Lode Vandevenne
// textures from: https://opengameart.org/content/zortch-unused-textures

#pragma once
#include "../_common/Common.hpp"
#include "Cache.hpp"

using namespace std::chrono_literals;
using namespace tcob::literals;

////////////////////////////////////////////////////////////

class Plinth : public scene {
public:
    Plinth(game& game);
    ~Plinth() override;

protected:
    void on_start() override;

    void on_draw_to(gfx::render_target& target, transform const& xform) override;

    void on_fixed_update(milliseconds deltaTime) override;
    void on_update(milliseconds deltaTime) override;

    void on_key_down(input::keyboard::event const& ev) override;

private:
    void draw();
    void cast(i32 x);
    void draw_sprites();

    auto move(milliseconds deltaTime) -> bool;

    point_d          _pos {0, 0};      // x and y start position
    point_d          _dir {-1, 0};     // initial direction vector
    point_d          _plane {0, 0.66}; // the 2d raycaster version of camera plane
    std::vector<f64> _rowDist;
    std::vector<f64> _zBuffer;         // one entry per screen column, set each frame

    i32 _screenWidth {0};
    i32 _screenHeight {0};
    i32 _texWidth {0};
    i32 _texHeight {0};
    i32 _texBpp {0};

    f64 _projPlaneDist {0};

    asset_owner_ptr<gfx::material> _material;
    asset_owner_ptr<gfx::texture>  _texture;
    gfx::renderer                  _renderer {gfx::buffer_usage_hint::StaticDraw};

    std::unique_ptr<cache_base> _cache;

    bool _update {true};

    struct sprite {
        point_d Pos {point_d::Zero};
        size_f  Size {size_f::One};
        i32     Texture {};
    };

    std::vector<sprite> _sprites;
};
