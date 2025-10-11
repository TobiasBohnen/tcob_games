// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

////////////////////////////////////////////////////////////

class die {
public:
    die(rng& rng, std::span<die_face const> faces);

    gfx::rect_shape* Shape {nullptr};
    die_face         Face;

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

    milliseconds _elapsed {0};
    milliseconds _updateTimer {0};
    milliseconds _targetTime {0};
    milliseconds _baseInterval {0};
    f32          _slowdownFactor {1.0f};

    std::shared_ptr<audio::sound> _sound;

    std::vector<die_face> _faces;
};

////////////////////////////////////////////////////////////

class dice {
public:
    dice(gfx::shape_batch& batch, gfx::window& window);

    void add_die(rng& rng, std::span<die_face const> faces, asset_ptr<gfx::material> const& material);

    void roll();

    void update(milliseconds deltaTime);

    void reset();

    void drag(point_i mousePos);

    auto hover_die(point_i mousePos) -> die*;

private:
    die*             _hoverDie {nullptr};
    std::vector<die> _dice;

    gfx::window&      _window;
    gfx::shape_batch& _batch;
};
