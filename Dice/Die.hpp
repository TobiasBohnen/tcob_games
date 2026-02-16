// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

////////////////////////////////////////////////////////////

enum class die_state : u8 {
    Normal,
    Hovered,
    Dragged
};

struct die_face {
    u8 Value {0};
    u8 Color {};

    auto texture_region(die_state state) const -> string
    {
        string suffix;
        switch (state) {
        case die_state::Normal: break;
        case die_state::Hovered:
        case die_state::Dragged:
            suffix = "-hover";
            break;
        }
        return std::format("d{}-{:X}{}", Value, Color, suffix);
    }

    auto operator==(die_face const& other) const -> bool = default;
};

class die {
    friend class dice;

public:
    die(gfx::rect_shape* shape, audio::buffer const& buffer, rng& rng, std::span<die_face const> faces, die_face initFace);

    auto face() const -> die_face;

    void mark_for_reroll();
    void lock();
    void unlock();

    void roll();
    auto is_rolling() const -> bool;

    void update(milliseconds deltaTime);

    auto get_bounds() const -> rect_f const&;
    void set_bounds(rect_f const& bounds);
    void move_to(point_f pos);

    auto shape() const -> gfx::rect_shape*;

private:
    gfx::rect_shape* _shape {nullptr};

    rng& _rng;
    bool _rolling {false};
    bool _locked {false};
    bool _reroll {true};

    std::unique_ptr<linear_tween<f32>> _tween;

    std::shared_ptr<audio::sound> _sound;

    std::vector<die_face> _faces;
    die_face              _currentFace;

    die_state _colorState {die_state::Normal};
};

////////////////////////////////////////////////////////////

class dice {
public:
    dice(assets::group& group, gfx::shape_batch& batch, size_f scale);

    auto add(point_f pos, rng& rng, die_face currentFace, std::span<die_face const> faces) -> die*;
    void move(usize idx, point_f target);

    void roll();

    auto on_hover(point_f mousePos) -> die*;

    void on_drag(point_f mousePos, rect_f const& winBounds);

    void update(milliseconds deltaTime);

    auto count() const -> usize;

private:
    std::vector<std::unique_ptr<die>> _dice;

    asset_ptr<gfx::material> _material;
    gfx::shape_batch&        _batch;
    size_f                   _scale;

    die* _hoverDie {nullptr};

    audio::buffer _buffer;
};
