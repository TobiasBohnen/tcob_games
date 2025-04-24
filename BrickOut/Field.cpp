// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Field.hpp"

namespace BrickOut {

constexpr float physicsWorldSize {50.0};

field::field(assets::group& resGrp, i32 padding, size_i windowSize)
    : gfx::entity {update_mode::Both}
    , _resGrp {resGrp}
    , _debugDraw {resGrp.get<gfx::font_family>("Poppins")->get_font({}, 16).ptr()}
    , _paddle {*this, resGrp}
    , _ball {*this, resGrp}
{
    // Create edges around the entire screen

    auto groundBody {_physicsWorld.create_body({}, {})};

    f32 const left {padding / 2.f};
    f32 const top {0.0f};
    f32 const right {((windowSize.Height + padding / 2.f))};
    f32 const bottom {static_cast<f32>(windowSize.Height)};
    _bounds                = rect_f::FromLTRB(left, top, right, bottom);
    _lightingSystem.Bounds = _bounds.as_padded_by(-size_f {100, 100});

    _physicsBounds = convert_to_physics(_bounds);

    physics::segment_shape::settings groundBoxSettings;
    groundBoxSettings.Point1 = _physicsBounds.top_left();
    groundBoxSettings.Point2 = _physicsBounds.top_right();
    groundBody->create_shape<physics::segment_shape>(groundBoxSettings);

    groundBoxSettings.Point1 = _physicsBounds.top_left();
    groundBoxSettings.Point2 = _physicsBounds.bottom_left();
    groundBody->create_shape<physics::segment_shape>(groundBoxSettings);

    groundBoxSettings.Point1 = _physicsBounds.bottom_left();
    groundBoxSettings.Point2 = _physicsBounds.bottom_right();
    groundBody->create_shape<physics::segment_shape>(groundBoxSettings);

    groundBoxSettings.Point1 = _physicsBounds.bottom_right();
    groundBoxSettings.Point2 = _physicsBounds.top_right();
    groundBody->create_shape<physics::segment_shape>(groundBoxSettings);

    auto leftEdgeSprite(create_sprite());
    leftEdgeSprite->Bounds   = rect_f {0, 0, padding / 2.f, static_cast<f32>(windowSize.Height)};
    leftEdgeSprite->Color    = colors::Gray;
    leftEdgeSprite->Material = _edgeMat;
    auto rightEdgeSprite(create_sprite());
    rightEdgeSprite->Bounds   = rect_f {static_cast<f32>(windowSize.Height) + padding / 2.f, 0,
                                      static_cast<f32>(windowSize.Width - windowSize.Height), static_cast<f32>(windowSize.Height)};
    rightEdgeSprite->Color    = colors::Gray;
    rightEdgeSprite->Material = _edgeMat;

    groundBody->UserData = "Ground";
}

void field::start(std::span<brick_def const> bricks)
{
    Score = 0;

    _physicsWorld.Gravity = {0, 50};

    _paddle.reset();
    _ball.reset();

    _brickDefs.assign(bricks.begin(), bricks.end());
    _bricks.clear();
    for (auto const& brickDef : bricks) {
        _bricks.emplace_back(std::make_unique<brick>(*this, _resGrp, brickDef))->reset();
    }

    _hitCount = 0;
    _state    = game_state::Running;
}

void field::fail()
{
    _spriteBatch.update(0ms);
    _state = game_state::Failure;
}

auto field::state() const -> game_state
{
    return _state;
}

auto field::create_sprite() -> std::shared_ptr<gfx::rect_shape>
{
    return _spriteBatch.create_shape<gfx::rect_shape>();
}

void field::remove_sprite(std::shared_ptr<gfx::rect_shape> const& rect_shape)
{
    _spriteBatch.remove_shape(*rect_shape);
}

auto field::create_body(physics::body::settings bodySettings) -> std::shared_ptr<physics::body>
{
    return _physicsWorld.create_body({}, bodySettings);
}

void field::remove_body(std::shared_ptr<physics::body> const& body)
{
    _physicsWorld.remove_body(*body);
}

auto field::create_light() -> std::shared_ptr<gfx::light_source>
{
    return _lightingSystem.create_light_source();
}

auto field::create_shadow() -> std::shared_ptr<gfx::shadow_caster>
{
    return _lightingSystem.create_shadow_caster();
}

void field::remove_shadow(std::shared_ptr<gfx::shadow_caster> const& sc)
{
    _lightingSystem.remove_shadow_caster(*sc);
}

auto field::get_field_bounds() const -> rect_f
{
    return _bounds;
}

auto field::get_physics_bounds() const -> rect_f
{
    return _physicsBounds;
}

void field::on_update(milliseconds deltaTime)
{
    if (_state != game_state::Running) {
        return;
    }

    _lightingSystem.update(deltaTime);
    _spriteBatch.update(deltaTime);

    if (_hitCount == _bricks.size()) {
        _state = game_state::Success;
    }
}

void field::on_fixed_update(milliseconds deltaTime)
{
    if (_state != game_state::Running) {
        return;
    }

    _physicsWorld.update(deltaTime);

    _paddle.update(deltaTime);
    _ball.update(deltaTime);
    for (auto& brick : _bricks) {
        brick->update(deltaTime);
    }
}

void field::on_draw_to(gfx::render_target& target)
{
    _spriteBatch.draw_to(target);
    _lightingSystem.draw_to(target);

    if (_debug != debug_mode::Off) {
        _debugDraw.draw(_physicsWorld, _debug == debug_mode::Transparent ? 0.5f : 1.0f, target);
    }
}

auto field::can_draw() const -> bool
{
    return true;
}

void field::on_key_down(input::keyboard::event const& ev)
{
    if (!ev.Repeat) {
        switch (ev.KeyCode) {
        case input::key_code::RIGHT: _paddle.move(1); break;
        case input::key_code::LEFT: _paddle.move(-1); break;
        case input::key_code::d:
            switch (_debug) {
            case debug_mode::Off: _debug = debug_mode::Transparent; break;
            case debug_mode::Transparent: _debug = debug_mode::On; break;
            case debug_mode::On: _debug = debug_mode::Off; break;
            }

            break;
        default: break;
        }
    }
}

void field::on_key_up(input::keyboard::event const& ev)
{
    if (!ev.Repeat) {
        switch (ev.KeyCode) {
        case input::key_code::RIGHT: _paddle.stop(); break;
        case input::key_code::LEFT: _paddle.stop(); break;
        default: break;
        }
    }
}

void field::on_controller_button_down(input::controller::button_event const& ev)
{
    switch (ev.Button) {
    case input::controller::button::Start: start(_brickDefs); break;
    case input::controller::button::DPadRight: _paddle.move(1); break;
    case input::controller::button::DPadLeft: _paddle.move(-1); break;
    default: break;
    }
}

void field::on_controller_button_up(input::controller::button_event const& ev)
{
    switch (ev.Button) {
    case input::controller::button::DPadRight: _paddle.stop(); break;
    case input::controller::button::DPadLeft: _paddle.stop(); break;
    default: break;
    }
}

void field::on_controller_axis_motion(input::controller::axis_event const& ev)
{
    switch (ev.Axis) {
    case input::controller::axis::LeftX:
        if (ev.RelativeValue > 0.25f) {
            _paddle.move(ev.RelativeValue);
        } else if (ev.RelativeValue < -0.25f) {
            _paddle.move(ev.RelativeValue);
        } else {
            _paddle.stop();
        }

        logger::Debug("{}", ev.RelativeValue);
        break;
    default: break;
    }
}

auto field::convert_to_physics(rect_f const& screenObject) const -> rect_f
{
    rect_f retValue;

    // Convert center coordinates to physics world coordinates
    retValue.Position.X = (screenObject.Position.X / _bounds.Size.Width) * physicsWorldSize;
    retValue.Position.Y = (screenObject.Position.Y / _bounds.Size.Height) * physicsWorldSize;

    // Convert size to physics world size
    retValue.Size.Width  = (screenObject.Size.Width / _bounds.Size.Width) * physicsWorldSize;
    retValue.Size.Height = (screenObject.Size.Height / _bounds.Size.Height) * physicsWorldSize;

    return retValue;
}

auto field::convert_to_screen(rect_f const& physicsObject) const -> rect_f
{
    rect_f retValue;

    // Convert center coordinates to screen coordinates
    retValue.Position.X = (physicsObject.Position.X / physicsWorldSize) * _bounds.Size.Width;
    retValue.Position.Y = (physicsObject.Position.Y / physicsWorldSize) * _bounds.Size.Height;

    // Convert size to screen size
    retValue.Size.Width  = (physicsObject.Size.Width / physicsWorldSize) * _bounds.Size.Width;
    retValue.Size.Height = (physicsObject.Size.Height / physicsWorldSize) * _bounds.Size.Height;

    return retValue;
}

void field::give_score(i32 score)
{
    Score += score;

    _hitCount++;
}
}
