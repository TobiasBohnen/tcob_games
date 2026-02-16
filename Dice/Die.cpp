// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Die.hpp"

////////////////////////////////////////////////////////////

die::die(gfx::rect_shape* shape, audio::buffer const& buffer, rng& rng, std::span<die_face const> faces, die_face initFace)
    : _shape(shape)
    , _rng {rng}
    , _faces {faces.begin(), faces.end()}
    , _currentFace {initFace}
{
    _sound = std::make_shared<audio::sound>(buffer);

    _shape->TextureRegion = _currentFace.texture_region(_colorState);
}

auto die::face() const -> die_face { return _currentFace; }

void die::mark_for_reroll() { _reroll = true; }

void die::lock() { _locked = true; }

void die::unlock() { _locked = false; }

auto die::is_rolling() const -> bool { return _rolling; }

void die::roll()
{
    if (!_reroll || _locked || _rolling) { return; }
    _reroll  = false;
    _rolling = true;

    constexpr i32 MinRollTime {250};
    constexpr i32 MaxRollTime {750};
    constexpr i32 MinRollInterval {50};
    constexpr i32 MaxRollInterval {100};

    _tween           = make_unique_tween<linear_tween<f32>>(milliseconds {_rng(MinRollTime, MaxRollTime)}, 0.0f, 1.0f);
    _tween->Interval = milliseconds {_rng(MinRollInterval, MaxRollInterval)};
    _tween->Value.Changed.connect([&](auto) {
        _tween->Interval = *_tween->Interval + milliseconds {_rng(MinRollInterval / 2, MaxRollInterval / 2)};

        usize const idx {_rng(usize {0}, _faces.size() - 1)};
        auto const& face {_faces.at(idx)};

        _currentFace.Color = face.Color;
        _currentFace.Value = face.Value;

        if (auto texRegion {_currentFace.texture_region(_colorState)}; _shape->TextureRegion != texRegion) {
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
    if (_rolling) {
        if (_tween) { _tween->update(deltaTime); }
    }
    _shape->TextureRegion = _currentFace.texture_region(_colorState);
}

auto die::get_bounds() const -> rect_f const& { return *_shape->Bounds; }
void die::set_bounds(rect_f const& bounds)
{
    _colorState    = die_state::Normal;
    _shape->Bounds = _shape->Bounds->as_centered_at(bounds.center());
}

void die::move_to(point_f pos)
{
    _shape->Bounds.mutate([&](rect_f& bounds) {
        bounds.Position = pos;
    });
}

auto die::shape() const -> gfx::rect_shape* { return _shape; }

////////////////////////////////////////////////////////////

dice::dice(assets::group& group, gfx::shape_batch& batch, size_f scale)
    : _material {group.get<gfx::material>("dice")}
    , _batch {batch}
    , _scale {scale}
{
    data::object ob {};
    ob.load("dice/dice.ini");

    audio::sound_generator gen {};
    audio::sound_wave      wv {ob["wave"].as<audio::sound_wave>()};
    _buffer = gen.create_buffer(wv);
}

auto dice::add(point_f pos, rng& rng, die_face currentFace, std::span<die_face const> faces) -> die*
{
    auto* shape {&_batch.create_shape<gfx::rect_shape>()};
    shape->Bounds   = {pos, DICE_SIZE * _scale};
    shape->Material = _material;

    auto& retValue {_dice.emplace_back(std::make_unique<die>(shape, _buffer, rng, faces, currentFace))};
    return retValue.get();
}

void dice::move(usize idx, point_f target)
{
    _dice[idx]->move_to(target);
}

void dice::roll()
{
    for (auto& die : _dice) { die->roll(); }
}

auto dice::on_hover(point_f mousePos) -> die*
{
    auto const findDie {[&](point_f mp) -> die* {
        auto const vec {_batch.intersect({mp, size_f::One})};
        if (vec.empty()) { return nullptr; }
        for (auto* v : vec | std::views::reverse) {
            auto* shape {dynamic_cast<gfx::rect_shape*>(v)};
            for (auto& die : _dice) {
                if (die->is_rolling()) { continue; }
                if (die->shape() == shape) { return die.get(); }
            }
        }
        return nullptr;
    }};

    auto* die {findDie(mousePos)};
    if (die) { die->_colorState = die_state::Hovered; }

    if (_hoverDie) {
        if (die != _hoverDie) {
            _hoverDie->_colorState = die_state::Normal;
        }
    }

    _hoverDie = die;
    return _hoverDie;
}

void dice::on_drag(point_f mousePos, rect_f const& winBounds)
{
    if (!_hoverDie || _hoverDie->_locked) { return; }

    point_f const halfSize {DICE_SIZE.Width * _scale.Width / 2, DICE_SIZE.Height * _scale.Height / 2};
    point_f       newPos {mousePos};

    if (mousePos.X - halfSize.X < winBounds.left()) {
        newPos.X = winBounds.left() + halfSize.X;
    } else if (mousePos.X + halfSize.X > winBounds.right()) {
        newPos.X = winBounds.right() - halfSize.X;
    }

    if (mousePos.Y - halfSize.Y < winBounds.top()) {
        newPos.Y = winBounds.top() + halfSize.Y;
    } else if (mousePos.Y + halfSize.Y > winBounds.bottom()) {
        newPos.Y = winBounds.bottom() - halfSize.Y;
    }

    rect_f const newBounds {newPos - halfSize, DICE_SIZE * _scale};

    _batch.bring_to_front(*_hoverDie->shape());
    _hoverDie->shape()->Bounds = newBounds;
    _hoverDie->_colorState     = die_state::Dragged;
}

void dice::update(milliseconds deltaTime)
{
    for (auto& die : _dice) {
        die->update(deltaTime);
    }
}

auto dice::count() const -> usize { return _dice.size(); }
