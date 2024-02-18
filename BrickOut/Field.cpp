// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Field.hpp"

namespace BrickOut {

constexpr float physicsWorldSize {50.0};

field::field(assets::group& resGrp, i32 padding, size_i windowSize)
    : _resGrp {resGrp}
    , _paddle {*this, resGrp}
    , _ball {*this, resGrp}
{
    // Create edges around the entire screen

    auto groundBody {_physicsWorld.create_body({}, {})};

    physics::edge_shape       groundBox;
    physics::fixture_settings groundBoxSettings;

    f32 const left {padding / 2.f};
    f32 const top {0.0f};
    f32 const right {((windowSize.Height + padding / 2.f))};
    f32 const bottom {static_cast<f32>(windowSize.Height)};
    _bounds = rect_f::FromLTRB(left, top, right, bottom);

    _physicsBounds = convert_to_physics(_bounds);

    groundBox.set_two_sided(_physicsBounds.top_left(), _physicsBounds.top_right());
    groundBody->create_fixture(groundBox, groundBoxSettings);

    groundBox.set_two_sided(_physicsBounds.top_left(), _physicsBounds.bottom_left());
    groundBody->create_fixture(groundBox, groundBoxSettings);

    groundBox.set_two_sided(_physicsBounds.bottom_left(), _physicsBounds.bottom_right());
    groundBody->create_fixture(groundBox, groundBoxSettings);

    groundBox.set_two_sided(_physicsBounds.bottom_right(), _physicsBounds.top_right());
    groundBody->create_fixture(groundBox, groundBoxSettings);

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

auto field::create_sprite() -> std::shared_ptr<gfx::sprite>
{
    return _spriteBatch.create_sprite();
}

void field::remove_sprite(std::shared_ptr<gfx::sprite> const& sprite)
{
    _spriteBatch.remove_sprite(sprite);
}

auto field::create_body(physics::body_settings bodySettings) -> std::shared_ptr<physics::body>
{
    return _physicsWorld.create_body({}, bodySettings);
}

void field::remove_body(std::shared_ptr<physics::body> const& body)
{
    _physicsWorld.destroy_body(body);
}

auto field::get_update_mode() const -> update_mode
{
    return update_mode::Both;
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

    _paddle.update(deltaTime);
    _ball.update(deltaTime);
    for (auto& brick : _bricks) {
        brick->update(deltaTime);
    }

    _physicsWorld.update(deltaTime);
}

void field::on_draw_to(gfx::render_target& target)
{
    _spriteBatch.draw_to(target);
}

auto field::can_draw() const -> bool
{
    return true;
}

void field::on_key_down(input::keyboard::event& ev)
{
    if (!ev.Repeat) {
        switch (ev.KeyCode) {
        case input::key_code::RIGHT: _paddle.move(direction::Right); break;
        case input::key_code::LEFT: _paddle.move(direction::Left); break;
        default: break;
        }
    }
}

void field::on_key_up(input::keyboard::event& ev)
{
    if (!ev.Repeat) {
        switch (ev.KeyCode) {
        case input::key_code::RIGHT: _paddle.stop(direction::Right); break;
        case input::key_code::LEFT: _paddle.stop(direction::Left); break;
        default: break;
        }
    }
}

void field::on_controller_button_down(input::controller::button_event& ev)
{
    switch (ev.Button) {
    case input::controller::button::DPadRight: _paddle.move(direction::Right); break;
    case input::controller::button::DPadLeft: _paddle.move(direction::Left); break;
    default: break;
    }
}

void field::on_controller_button_up(input::controller::button_event& ev)
{
    switch (ev.Button) {
    case input::controller::button::DPadRight: _paddle.stop(direction::Right); break;
    case input::controller::button::DPadLeft: _paddle.stop(direction::Left); break;
    default: break;
    }
}

auto field::convert_to_physics(rect_f const& screenObject) const -> rect_f
{
    rect_f retValue;

    // Convert center coordinates to physics world coordinates
    retValue.X = (screenObject.X / _bounds.Width) * physicsWorldSize;
    retValue.Y = (screenObject.Y / _bounds.Height) * physicsWorldSize;

    // Convert size to physics world size
    retValue.Width  = (screenObject.Width / _bounds.Width) * physicsWorldSize;
    retValue.Height = (screenObject.Height / _bounds.Height) * physicsWorldSize;

    return retValue;
}

auto field::convert_to_screen(rect_f const& physicsObject) const -> rect_f
{
    rect_f retValue;

    // Convert center coordinates to screen coordinates
    retValue.X = (physicsObject.X / physicsWorldSize) * _bounds.Width;
    retValue.Y = (physicsObject.Y / physicsWorldSize) * _bounds.Height;

    // Convert size to screen size
    retValue.Width  = (physicsObject.Width / physicsWorldSize) * _bounds.Width;
    retValue.Height = (physicsObject.Height / physicsWorldSize) * _bounds.Height;

    return retValue;
}

void field::give_score(i32 score)
{
    Score += score;

    _hitCount++;
}
}
