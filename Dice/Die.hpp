// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep
#include "Hand.hpp"

////////////////////////////////////////////////////////////

class die {
public:
    explicit die(rng& rng, std::span<die_face const> faces);

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
    dice(gfx::shape_batch& batch, gfx::window& window, slots& slots);

    void add_die(rng& rng, std::span<die_face const> faces, asset_ptr<gfx::material> const& material);

    void roll();

    void update(milliseconds deltaTime);

    void reset();

    void drag(point_i mousePos, rect_i const& bounds);

    auto drop_die() -> hand;

    void hover_die(point_i mousePos);
    void hover_slot(point_i mousePos);

private:
    bool             _isDragging {false};
    die*             _hoverDie {nullptr};
    std::vector<die> _dice;

    gfx::window&      _window;
    gfx::shape_batch& _batch;
    slots&            _slots;
};
