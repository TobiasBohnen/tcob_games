// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

////////////////////////////////////////////////////////////

class die {
public:
    die(rng& rng, std::span<die_face const> faces, die_face initFace);

    gfx::rect_shape* Shape {nullptr};

    auto current_face() const -> die_face;

    void lock();
    void unlock();

    auto is_rolling() const -> bool;
    void roll();

    void update(milliseconds deltaTime);

    auto texture_region() const -> string;

private:
    rng& _rng;
    bool _rolling {false};
    bool _locked {false};

    std::unique_ptr<linear_tween<f32>> _tween;

    std::shared_ptr<audio::sound> _sound;

    std::vector<die_face> _faces;
    die_face              _currentFace;
};

////////////////////////////////////////////////////////////

class dice {
public:
    dice(gfx::shape_batch& batch, asset_ptr<gfx::material> const& material, gfx::window& window);

    void add_die(point_f pos, rng& rng, std::span<die_face const> faces);
    auto get_die(usize idx) const -> die*;

    void roll();

    void drag(point_i mousePos);

    auto hover_die(point_i mousePos) -> die*;

    void clear();
    void reset_shapes();

    void update(milliseconds deltaTime);

private:
    die*             _hoverDie {nullptr};
    std::vector<die> _dice;

    gfx::window&             _window;
    gfx::shape_batch&        _batch;
    asset_ptr<gfx::material> _material;
};
