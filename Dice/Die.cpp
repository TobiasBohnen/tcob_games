// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Die.hpp"

die::die(rng& rng, std::span<die_face const> faces)
    : _rng {rng}
    , _faces {faces.begin(), faces.end()}
{
    data::object ob {};
    ob.load("dice/dice.ini");

    audio::sound_generator gen {};
    audio::sound_wave      wv {ob["wave"].as<audio::sound_wave>()};
    wv = gen.mutate_wave(wv);
    auto buffer {gen.create_buffer(wv)};
    _sound = std::make_shared<audio::sound>(buffer);
}

void die::lock() { _locked = true; }

void die::unlock() { _locked = false; }

auto die::is_rolling() const -> bool
{
    return _rolling;
}

void die::roll()
{
    if (_locked) { return; }

    _rolling = true;
    _elapsed = milliseconds {0.0f};

    _targetTime     = milliseconds {_rng(500, 1200)};
    _baseInterval   = milliseconds {_rng(20, 35)};
    _slowdownFactor = _rng(0.7f, 1.1f);
}

void die::update(milliseconds deltaTime)
{
    if (!_rolling) { return; }

    _elapsed += deltaTime;
    _updateTimer += deltaTime;

    f32 const          progress {std::clamp(static_cast<f32>(_elapsed.count()) / static_cast<f32>(_targetTime.count()),
                                            0.0f, 1.0f)};
    f32 const          eased {std::pow(progress, _slowdownFactor)};
    milliseconds const interval {std::lerp(static_cast<f32>(_baseInterval.count()), 200.0f, eased)};

    bool ticked {false};
    if (_elapsed >= _targetTime) {
        _rolling = false;
        ticked   = true;
    } else if (_updateTimer >= interval) {
        _updateTimer -= interval;
        ticked = true;
    }

    if (!ticked) { return; }

    u8 const    idx {static_cast<u8>(_rng(usize {0}, _faces.size() - 1))};
    auto const& face {_faces.at(idx)};

    Face.Color = face.Color;
    Face.Value = face.Value;

    if (auto texRegion {texture_region()}; Shape->TextureRegion != texRegion) {
        Shape->TextureRegion = texRegion;

        if (_sound && _sound->state() != playback_state::Running) {
            _sound->play();
        }
    }
}

auto die::texture_region() const -> string
{
    string color;
    switch (Face.Color) {
    case color_type::Red:    color = "red"; break;
    case color_type::Green:  color = "green"; break;
    case color_type::Yellow: color = "yellow"; break;
    case color_type::Cyan:   color = "cyan"; break;
    case color_type::Blue:   color = "blue"; break;
    }
    string value;
    switch (Face.Value) {
    case 1:   value = "1"; break;
    case 2:   value = "2"; break;
    case 3:   value = "3"; break;
    case 4:   value = "4"; break;
    case 5:   value = "5"; break;
    case 6:   value = "6"; break;
    case 254: value = "E"; break;
    case 255: value = "Q"; break;
    }
    return color + value;
}

////////////////////////////////////////////////////////////

dice::dice(gfx::shape_batch& batch, gfx::window& window)
    : _window {window}
    , _batch {batch}
{
}

void dice::add_die(rng& rng, std::span<die_face const> faces, asset_ptr<gfx::material> const& material)
{
    auto& die {_dice.emplace_back(rng, faces)};
    die.Face.Color = color_type::Red;
    die.Face.Value = rng(1, 6);

    die.Shape                = &_batch.create_shape<gfx::rect_shape>();
    die.Shape->Bounds        = {{_dice.size() * DICE_OFFSET, 2 * DICE_OFFSET}, DICE_SIZE};
    die.Shape->Material      = material;
    die.Shape->TextureRegion = die.texture_region();
}

void dice::roll()
{
    _hoverDie = nullptr;
    for (auto& die : _dice) { die.roll(); }
}

void dice::update(milliseconds deltaTime)
{
    for (auto& die : _dice) {
        die.update(deltaTime);
    }
}

void dice::reset()
{
    for (auto& die : _dice) {
        die.Shape->Color  = colors::White;
        die.Shape->Bounds = {die.Shape->Bounds->center() - point_f {DICE_SIZE.Width / 2, DICE_SIZE.Height / 2}, DICE_SIZE};
    }
}

void dice::drag(point_i mousePos)
{
    if (!_hoverDie) { return; }

    point_f const newPos {_window.camera().convert_screen_to_world(mousePos)};
    rect_i const& bounds {_window.bounds()};

    constexpr static f32 DragScaleFactor {1.2f};
    size_f const         dieSize {DICE_SIZE * DragScaleFactor};
    point_f              halfSize {dieSize.Width / 2, dieSize.Height / 2};

    point_f clampedPos {newPos};

    // slide along edges instead of hard stop
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

    rect_f const newBounds {clampedPos - halfSize, dieSize};

    _batch.bring_to_front(*_hoverDie->Shape);
    _hoverDie->Shape->Bounds = newBounds;
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
                if (die.Shape == shape) { return &die; }
            }
        }
        return nullptr;
    }};

    auto* die {findDie(mp)};
    if (die) { die->Shape->Color = colors::Silver; }
    _hoverDie = die;
    return _hoverDie;
}
