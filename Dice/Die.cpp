// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Die.hpp"

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
    wv = gen.mutate_wave(0, wv);
    auto buffer {gen.create_buffer(wv)};
    _sound = std::make_shared<audio::sound>(buffer);

    _shape->TextureRegion = _currentFace.texture_region();
}

auto die::current_face() const -> die_face { return _currentFace; }

void die::freeze() { _frozen = true; }

void die::unfreeze() { _frozen = false; }

auto die::is_rolling() const -> bool { return _rolling; }

void die::roll()
{
    if (_frozen || _rolling) { return; }

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

auto die::bounds() const -> rect_f const& { return *_shape->Bounds; }

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

void die::on_slotted(rect_f const& bounds)
{
    _colorState    = die_state::Normal;
    _shape->Bounds = bounds;
}

////////////////////////////////////////////////////////////

dice::dice(gfx::shape_batch& batch, size_f scale)
    : _batch {batch}
    , _painter {{10, 50}}
    , _scale {scale}
{
}

auto dice::add_die(point_f pos, rng& rng, die_face currentFace, std::span<die_face const> faces) -> die*
{
    auto* shape {&_batch.create_shape<gfx::rect_shape>()};
    shape->Bounds   = {pos, DICE_SIZE * _scale};
    shape->Material = _painter.material();

    auto& retValue {_dice.emplace_back(std::make_unique<die>(shape, rng, faces, currentFace))};
    _painter.make_die(faces);

    return retValue.get();
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
                if (die->_shape == shape) { return die.get(); }
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
    if (!_hoverDie || _hoverDie->_frozen) { return; }

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

    _batch.bring_to_front(*_hoverDie->_shape);
    _hoverDie->_shape->Bounds = newBounds;
    _hoverDie->_colorState    = die_state::Dragged;
}

void dice::update(milliseconds deltaTime)
{
    for (auto& die : _dice) {
        die->update(deltaTime);
    }
}

////////////////////////////////////////////////////////////

dice_painter::dice_painter(size_i texGrid)
    : _texGrid {texGrid}
{
    _tex = _canvas.get_texture(0);

    _material->first_pass().Texture = _tex;
}

auto dice_painter::material() -> asset_owner_ptr<gfx::material>
{
    return _material;
}

void dice_painter::make_die(std::span<die_face const> faces)
{
    static auto const dicePath {*gfx::path2d::Parse("m 15,0 h 34 c 8,0 15,7 15,15 v 34 c 0,8 -7,15 -15,15 H 15 C 7,64 0,57 0,49 V 15 C 0,7 7,0 15,0 Z")};
    auto const        paintDie {[&](color fg, color bg) {
        _canvas.path_2d(dicePath);
        _canvas.set_fill_style(fg);
        _canvas.fill(false);
        _canvas.set_stroke_width(2);
        _canvas.set_stroke_style(bg);
        _canvas.stroke();
    }};
    auto const        paintDots {[&](color fg, auto... dots) {
        _canvas.set_fill_style(fg);
        _canvas.begin_path();
        (_canvas.circle(dots, 6), ...);
        _canvas.fill();
    }};

    auto const texSize {_texGrid * 68};
    _canvas.begin_frame(texSize, 1, 0, false);
    for (auto const& face : faces) {
        if (_tex->regions().contains(face.texture_region())) { continue; }

        _canvas.save();

        _canvas.translate(_pen);

        _tex->regions()[face.texture_region()] = {
            .UVRect = {{(_pen.X - 2) / static_cast<f32>(texSize.Width),
                        -(_pen.Y - 2) / static_cast<f32>(texSize.Height)},
                       {68.f / static_cast<f32>(texSize.Width),
                        -68.f / static_cast<f32>(texSize.Height)}},
            .Level  = 0};

        _pen += point_f {68, 0};
        if (_pen.X >= static_cast<f32>(texSize.Width)) {
            _pen.X = 2;
            _pen.Y += 68;
        }

        paintDie(face.Color, colors::Black);

        switch (face.Value) {
        case 1: paintDots(colors::Black, point_f {32, 32}); break;
        case 2: paintDots(colors::Black, point_f {16, 16}, point_f {48, 48}); break;
        case 3: paintDots(colors::Black, point_f {16, 16}, point_f {32, 32}, point_f {48, 48}); break;
        case 4: paintDots(colors::Black, point_f {16, 16}, point_f {48, 16}, point_f {16, 48}, point_f {48, 48}); break;
        case 5: paintDots(colors::Black, point_f {16, 16}, point_f {48, 16}, point_f {32, 32}, point_f {16, 48}, point_f {48, 48}); break;
        case 6: paintDots(colors::Black, point_f {16, 16}, point_f {48, 16}, point_f {16, 32}, point_f {48, 32}, point_f {16, 48}, point_f {48, 48}); break;
        }

        _canvas.restore();
    }
    _canvas.end_frame();
    _tex->Filtering = gfx::texture::filtering::Linear;
}
