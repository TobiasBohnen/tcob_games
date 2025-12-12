// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep
#include "Painter.hpp"

////////////////////////////////////////////////////////////

enum class die_state : u8 {
    Normal,
    Hovered,
    Dragged
};

class die {
    friend class dice;

public:
    die(gfx::rect_shape* shape, rng& rng, std::span<die_face const> faces, die_face initFace);

    auto current_face() const -> die_face;

    void freeze();
    void unfreeze();

    void roll();
    auto is_rolling() const -> bool;

    void update(milliseconds deltaTime);

    auto bounds() const -> rect_f const&;
    void move_to(point_f pos);
    void move_by(point_f offset);

    void on_slotted(rect_f const& bounds);

private:
    gfx::rect_shape* _shape {nullptr};

    rng& _rng;
    bool _rolling {false};
    bool _frozen {false};

    std::unique_ptr<linear_tween<f32>> _tween;

    std::shared_ptr<audio::sound> _sound;

    std::vector<die_face> _faces;
    die_face              _currentFace;

    die_state _colorState {die_state::Normal};
};

////////////////////////////////////////////////////////////

class dice {
public:
    dice(gfx::shape_batch& batch, size_f scale);

    auto add_die(point_f pos, rng& rng, die_face currentFace, std::span<die_face const> faces) -> die*;

    void roll();

    auto on_hover(point_f mousePos) -> die*;

    void on_drag(point_f mousePos, rect_f const& winBounds);

    void update(milliseconds deltaTime);

private:
    std::vector<std::unique_ptr<die>> _dice;

    gfx::shape_batch& _batch;
    dice_painter      _painter;
    size_f            _scale;

    die* _hoverDie {nullptr};
};
