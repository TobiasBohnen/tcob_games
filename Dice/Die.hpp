// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

////////////////////////////////////////////////////////////

struct die_face {
    u8    Value {0};
    color Color {};

    auto texture_region() const -> string
    {
        return std::format("d{}-{}", Value, Color.value());
    }

    auto operator==(die_face const& other) const -> bool = default;
};

enum class die_state : u8 {
    Normal,
    Hovered,
    Dragged
};

class die {
    friend class dice;

public:
    die(gfx::rect_shape* shape, audio::buffer const& buffer, rng& rng, std::span<die_face const> faces, die_face initFace);

    auto current_face() const -> die_face;

    void freeze();
    void unfreeze();

    void roll();
    auto is_rolling() const -> bool;

    void update(milliseconds deltaTime);

    auto bounds() const -> rect_f const&;
    void move_to(point_f pos);
    void move_by(point_f offset);

    void on_socketed(rect_f const& bounds);

    auto shape() const -> gfx::rect_shape*;

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

class dice_painter {
public:
    explicit dice_painter(size_i texGrid);

    auto material() -> asset_owner_ptr<gfx::material>;

    void make_die(std::span<die_face const> faces);

private:
    gfx::canvas _canvas;

    size_i                         _texGrid;
    point_f                        _pen {2, 2};
    asset_ptr<gfx::texture>        _tex;
    asset_owner_ptr<gfx::material> _material;
};

////////////////////////////////////////////////////////////

class dice {
public:
    dice(gfx::shape_batch& batch, size_f scale);

    auto add_die(point_f pos, rng& rng, die_face currentFace, std::span<die_face const> faces) -> die*;
    void move_die(usize idx, point_f target);

    void roll();

    auto on_hover(point_f mousePos) -> die*;

    void on_drag(point_f mousePos, rect_f const& winBounds);

    void update(milliseconds deltaTime);

    auto count() const -> usize;

private:
    std::vector<std::unique_ptr<die>> _dice;

    gfx::shape_batch& _batch;
    dice_painter      _painter;
    size_f            _scale;

    die* _hoverDie {nullptr};

    audio::buffer _buffer;
};
