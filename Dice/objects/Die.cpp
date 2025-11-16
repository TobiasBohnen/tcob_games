// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Die.hpp"

#include "Slot.hpp"

////////////////////////////////////////////////////////////

die::die(gfx::rect_shape* shape, rng& rng, std::span<die_face const> faces, die_face initFace)
    : _shape(shape)
    , _rng {rng}
    , _faces {faces.begin(), faces.end()}
    , _currentFace {initFace}
{
    data::object ob {};
    ob.load("dice/dice.ini");

    audio::sound_generator gen {};
    audio::sound_wave      wv {ob["wave"].as<audio::sound_wave>()};
    wv = gen.mutate_wave(wv);
    auto buffer {gen.create_buffer(wv)};
    _sound = std::make_shared<audio::sound>(buffer);

    _shape->TextureRegion = _currentFace.texture_region();
}

auto die::current_face() const -> die_face { return _currentFace; }

void die::freeze() { _frozen = true; }

void die::unfreeze() { _frozen = false; }

auto die::is_rolling() const -> bool
{
    return _rolling;
}

void die::roll()
{
    if (_frozen || _rolling) { return; }

    _rolling = true;

    constexpr i32 MinRollTime {250};
    constexpr i32 MaxRollTime {250};
    constexpr i32 MinRollInterval {20};
    constexpr i32 MaxRollInterval {40};

    _tween           = make_unique_tween<linear_tween<f32>>(milliseconds {_rng(MinRollTime, MaxRollTime)}, 0.0f, 1.0f);
    _tween->Interval = milliseconds {_rng(MinRollInterval, MaxRollInterval)};
    _tween->Value.Changed.connect([&](auto val) {
        _tween->Interval = *_tween->Interval + milliseconds {_rng(MinRollInterval / 2, MaxRollInterval / 2)};

        u8 const    idx {static_cast<u8>(_rng(usize {0}, _faces.size() - 1))};
        auto const& face {_faces.at(idx)};

        _currentFace.Color = face.Color;
        _currentFace.Value = face.Value;

        if (auto texRegion {_currentFace.texture_region()}; _shape->TextureRegion != texRegion) {
            _shape->TextureRegion = texRegion;

            if (_sound && _sound->state() != playback_state::Running) {
                _sound->play();
            }
        }
    });
    _tween->Finished.connect([&] { _rolling = false; });
    _tween->start();
}

void die::update(milliseconds deltaTime)
{
    switch (_colorState) {
    case die_state::Normal:
        _shape->Color = colors::White;
        break;
    case die_state::Hovered:
    case die_state::Dragged:
        _shape->Color = colors::DarkGray;
        break;
    }

    if (!_rolling) { return; }
    if (_tween) { _tween->update(deltaTime); }
}

auto die::bounds() const -> rect_f const&
{
    return *_shape->Bounds;
}

void die::move_to(point_f pos)
{
    _shape->Bounds.mutate([&](rect_f& bounds) {
        bounds.Position = pos;
    });
}

void die::move_by(point_f offset)
{
    _shape->Bounds.mutate([&](rect_f& bounds) {
        bounds.Position += offset;
    });
}

////////////////////////////////////////////////////////////

dice::dice(gfx::shape_batch& batch, gfx::window& window)
    : _window {window}
    , _batch {batch}
    , _painter {{10, 50}}
{
}

void dice::add_die(point_f pos, rng& rng, die_face currentFace, std::span<die_face const> faces)
{
    auto* shape {&_batch.create_shape<gfx::rect_shape>()};
    shape->Bounds   = {pos, DICE_SIZE};
    shape->Material = _painter.material();

    _dice.emplace_back(shape, rng, faces, currentFace);
    _painter.make_die(faces);
}

auto dice::get_die(usize idx) -> die*
{
    return &_dice[idx];
}

void dice::roll()
{
    _hoverDie = nullptr;
    for (auto& die : _dice) { die.roll(); }
}

void dice::drag(point_i mousePos)
{
    if (!_hoverDie || _hoverDie->_frozen) { return; }

    point_f const newPos {_window.camera().convert_screen_to_world(mousePos)};
    rect_i const& bounds {_window.bounds()};

    point_f const halfSize {DICE_SIZE.Width / 2, DICE_SIZE.Height / 2};

    point_f clampedPos {newPos};

    if (newPos.X - halfSize.X < bounds.left()) {
        clampedPos.X = bounds.left() + halfSize.X;
    } else if (newPos.X + halfSize.X > bounds.right()) {
        clampedPos.X = bounds.right() - halfSize.X;
    }

    if (newPos.Y - halfSize.Y < bounds.top()) {
        clampedPos.Y = bounds.top() + halfSize.Y;
    } else if (newPos.Y + halfSize.Y > bounds.bottom()) {
        clampedPos.Y = bounds.bottom() - halfSize.Y;
    }

    rect_f const newBounds {clampedPos - halfSize, DICE_SIZE};

    _batch.bring_to_front(*_hoverDie->_shape);
    _hoverDie->_shape->Bounds = newBounds;
    _hoverDie->_colorState    = die_state::Dragged;
}

void dice::accept(slot* slot)
{
    if (!_hoverDie || !slot || !slot->can_accept(_hoverDie->current_face())) { return; }

    if (slot->is_empty()) {
        slot->accept(_hoverDie);
    }

    _hoverDie->_colorState    = die_state::Hovered;
    _hoverDie->_shape->Bounds = slot->bounds();
    _batch.send_to_back(*_hoverDie->_shape);
}

auto dice::hover_die(point_i mousePos) -> die*
{
    point_f const mp {_window.camera().convert_screen_to_world(mousePos)};

    auto const findDie {[&](point_f mp) -> die* {
        auto const vec {_batch.intersect({mp, size_f::One})};
        if (vec.empty()) { return nullptr; }
        for (auto* v : vec | std::views::reverse) {
            auto* shape {dynamic_cast<gfx::rect_shape*>(v)};
            for (auto& die : _dice) {
                if (die.is_rolling()) { continue; }
                if (die._shape == shape) { return &die; }
            }
        }
        return nullptr;
    }};

    auto* die {findDie(mp)};
    if (die) { die->_colorState = die_state::Hovered; }

    if (_hoverDie) {
        if (die != _hoverDie) {
            _hoverDie->_colorState = die_state::Normal;
        }
    }

    _hoverDie = die;
    return _hoverDie;
}

void dice::clear()
{
    _dice.clear();
    _batch.clear();
}

void dice::update(milliseconds deltaTime)
{
    for (auto& die : _dice) {
        die.update(deltaTime);
    }
}

auto dice::count() const -> usize
{
    return _dice.size();
}
