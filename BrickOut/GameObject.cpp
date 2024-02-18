#include "GameObject.hpp"

#include "Field.hpp"

namespace BrickOut {

////////////////////////////////////////////////////////////

game_object::game_object(field& parent)
    : _parent {parent}
    , _sprite {parent.create_sprite()}
    , _physicsBody {parent.create_body({})}
{
}

game_object::~game_object()
{
    destroy();
}

void game_object::destroy()
{
    if (_physicsFixture) {
        _physicsBody->destroy_fixture(_physicsFixture);
        _physicsFixture = nullptr;
    }
    if (_physicsBody) {
        _parent.remove_body(_physicsBody);
        _physicsBody = nullptr;
    }
    if (_sprite) {
        _parent.remove_sprite(_sprite);
        _sprite = nullptr;
    }
}

void game_object::update(milliseconds deltaTime)
{
    if (_physicsBody && _sprite) {
        rect_f const pb {_physicsBody->Transform().Position, size_f::Zero};

        _sprite->Center   = convert_to_screen(pb).get_center();
        _sprite->Rotation = _physicsBody->Transform().Angle;
    }

    on_update(deltaTime);
}

void game_object::give_score(i32 score)
{
    _parent.give_score(score);
}

void game_object::fail()
{
    _parent.fail();
}

void game_object::set_material(assets::asset_ptr<gfx::material> const& mat)
{
    _sprite->Material = mat;
}

auto game_object::get_sprite() -> gfx::sprite&
{
    return *_sprite;
}

auto game_object::get_body() -> physics::body&
{
    return *_physicsBody;
}

auto game_object::get_field_bounds() -> rect_f
{
    return _parent.get_field_bounds();
}

auto game_object::convert_to_physics(rect_f const& screenObject) const -> rect_f
{
    return _parent.convert_to_physics(screenObject);
}

auto game_object::convert_to_screen(rect_f const& physicsObject) const -> rect_f
{
    return _parent.convert_to_screen(physicsObject);
}

void game_object::create_fixture(physics::shape const& shape, physics::fixture_settings const& settings)
{
    if (_physicsFixture) {
        _physicsBody->destroy_fixture(_physicsFixture);
    }
    if (_physicsBody) {
        _physicsFixture = _physicsBody->create_fixture(shape, settings);
    }
}

////////////////////////////////////////////////////////////

paddle::paddle(field& parent, assets::group& resGrp)
    : game_object {parent}
{
    set_material(resGrp.get<gfx::material>("paddleBlu"));
    auto& body {get_body()};
    body.UserData = "Paddle";
}

void paddle::reset()
{
    auto& body {get_body()};
    body.LinearVelocity  = {0, 0};
    body.AngularVelocity = 0;

    auto const rect {get_field_bounds()};
    auto&      sprite {get_sprite()};
    _size         = size_f {rect.Width / 8.0f, rect.Height / 40.0f};
    sprite.Bounds = {point_f::Zero, _size};
    rect_f const physRect {convert_to_physics({(rect.Width - _size.Width) / 2,
                                               (rect.Height - _size.Height * 2),
                                               _size.Width,
                                               _size.Height})};

    body.Transform    = physics::body_transform {physRect.get_center(), 0};
    body.Type         = physics::body_type::Static;
    body.GravityScale = 0;

    physics::polygon_shape shape;
    // shape.set_as_box(physRect.get_size());
    std::array<point_f, 4> vecs {{
        {-physRect.Width / 5, -physRect.Height / 2},
        {physRect.Width / 5, -physRect.Height / 2},
        {physRect.Width / 2, physRect.Height / 2},
        {-physRect.Width / 2, physRect.Height / 2},
    }};
    shape.set(vecs);

    physics::fixture_settings settings;
    settings.Restitution = 0.1f;
    settings.Density     = 10.0f;
    settings.Friction    = 0.4f;
    create_fixture(shape, settings);

    _moveSpeed        = rect.Width / 75.f;
    _physicsMoveSpeed = convert_to_physics({{_moveSpeed, 0}, size_f::Zero}).X;
}

void paddle::move(direction dir)
{
    _dir = dir;
}

void paddle::stop(direction dir)
{
    if (_dir == dir) {
        _dir = direction::None;
    }
}

void paddle::on_update(milliseconds deltaTime)
{
    if (_dir == direction::None) {
        return;
    }

    auto& body {get_body()};
    auto  xform {body.Transform()};

    auto const rect {get_field_bounds()};
    auto       bodyRect {convert_to_screen({xform.Position, size_f::Zero})};
    bodyRect.Width = _size.Width + _moveSpeed * 2;
    bodyRect.X -= bodyRect.Width / 2;

    switch (_dir) {
    case direction::Left:
        if (bodyRect.left() > rect.left()) {
            xform.Position.X -= _physicsMoveSpeed;
        } else {
            _dir = direction::None;
        }
        break;
    case direction::Right:
        if (bodyRect.right() < rect.right()) {
            xform.Position.X += _physicsMoveSpeed;
        } else {
            _dir = direction::None;
        }
        break;
    default: break;
    }

    body.Transform = xform;
}

////////////////////////////////////////////////////////////

ball::ball(field& parent, assets::group& resGrp)
    : game_object {parent}
{
    set_material(resGrp.get<gfx::material>("ballBlue"));
    auto& body {get_body()};
    body.UserData = "Ball";
}

void ball::reset()
{
    auto& body {get_body()};
    body.LinearVelocity  = {0, 0};
    body.AngularVelocity = 0;

    auto const rect {get_field_bounds()};
    auto&      sprite {get_sprite()};
    _size         = size_f {rect.Width / 40.0f, rect.Height / 40.0f};
    sprite.Bounds = {point_f::Zero, _size};
    rect_f const physRect {convert_to_physics({(rect.Width - _size.Width) / 2,
                                               (rect.Height / 2.0f),
                                               _size.Width,
                                               _size.Height})};

    body.Transform    = physics::body_transform {physRect.get_center(), 0};
    body.Type         = physics::body_type::Dynamic;
    body.GravityScale = 0;

    physics::circle_shape shape {physRect.get_size().Width / 2};

    physics::fixture_settings settings {
        .Friction    = 0.0f,
        .Restitution = 1.f,
        .Density     = 0.5f,
    };

    create_fixture(shape, settings);

    rng           rand;
    point_f const force {rand(-20.f, 20.f), -20};
    body.apply_linear_impulse(force, physRect.get_center());

    _failHeight = rect.Height - _size.Height / 2;
}

void ball::on_update(milliseconds deltaTime)
{
    auto& body {get_body()};
    auto  xform {body.Transform()};

    auto bodyRect {convert_to_screen({xform.Position, size_f::Zero})};
    bodyRect.Height = _size.Height;

    auto const lv {body.LinearVelocity()};

    f32 const maxVel {45};
    if (std::abs(lv.Y) > maxVel) {
        f32 const ratio {std::abs(lv.X / lv.Y)};
        f32 const signY {lv.Y < 0 ? -1.f : 1.f};
        body.LinearVelocity = {maxVel * ratio * signY, maxVel * signY};
    }
    if (std::abs(lv.X) > maxVel) {
        f32 const ratio {std::abs(lv.Y / lv.X)};
        f32 const signX {lv.X < 0 ? -1.f : 1.f};
        body.LinearVelocity = {maxVel * signX, maxVel * ratio * signX};
    }

    if (bodyRect.bottom() >= _failHeight) {
        fail();
    }
}

////////////////////////////////////////////////////////////

brick::brick(field& parent, assets::group& resGrp, brick_def def)
    : game_object {parent}
    , _def {def}
{
    std::string matName {"element_"};
    switch (def.Color) {
    case brick_def::color::Blue: matName += "blue_"; break;
    case brick_def::color::Green: matName += "green_"; break;
    case brick_def::color::Grey: matName += "grey_"; break;
    case brick_def::color::Purple: matName += "purple_"; break;
    case brick_def::color::Red: matName += "red_"; break;
    case brick_def::color::Yellow: matName += "yellow_"; break;
    }
    switch (def.Shape) {
    case brick_def::shape::Rectangle: matName += "rectangle"; break;
    case brick_def::shape::Square: matName += "square"; break;
    }

    set_material(resGrp.get<gfx::material>(matName));
    auto& body {get_body()};
    body.UserData = "Brick";
}

void brick::reset()
{
    auto& body {get_body()};

    auto const rect {get_field_bounds()};
    auto&      sprite {get_sprite()};
    f32 const  size {rect.Width / 20.f * _def.Size};
    _size         = size_f {_def.Shape == brick_def::shape::Square ? size : size * 2, size};
    sprite.Bounds = {point_f::Zero, _size};
    rect_f const physRect {convert_to_physics({rect.X + rect.Width * _def.Position.X,
                                               rect.Y + rect.Height * _def.Position.Y,
                                               _size.Width,
                                               _size.Height})};

    body.Transform = physics::body_transform {physRect.get_center(), 0};
    body.Type      = physics::body_type::Dynamic;
    body.Awake     = false;

    physics::polygon_shape shape;
    shape.set_as_box(physRect.get_size());

    physics::fixture_settings settings;
    switch (_def.Color) {
    case brick_def::color::Blue:
        settings.Restitution = 0.1f;
        settings.Density     = 10.0f;
        settings.Friction    = 0.0f;
        _timeOut             = 4s;
        break;
    case brick_def::color::Green:
        settings.Restitution = 0.2f;
        settings.Density     = 10.0f;
        settings.Friction    = 1.0f;
        _timeOut             = 3s;
        break;
    case brick_def::color::Grey:
        settings.Restitution = 0.3f;
        settings.Density     = 10.0f;
        settings.Friction    = 0.0f;
        _timeOut             = 2s;
        break;
    case brick_def::color::Purple:
        settings.Restitution = 0.4f;
        settings.Density     = 10.0f;
        settings.Friction    = 0.4f;
        _timeOut             = 10s;
        break;
    case brick_def::color::Red:
        settings.Restitution = 0.5f;
        settings.Density     = 10.0f;
        settings.Friction    = 0.3f;
        _timeOut             = 1s;
        break;
    case brick_def::color::Yellow:
        settings.Restitution = 0.6f;
        settings.Density     = 10.0f;
        settings.Friction    = 0.6f;
        _timeOut             = 5s;
        break;
    }

    create_fixture(shape, settings);
}

void brick::on_update(milliseconds deltaTime)
{
    if (!_dead) {
        auto& body {get_body()};
        auto  xform {body.Transform()};

        auto bodyRect {convert_to_screen({xform.Position, size_f::Zero})};
        bodyRect.Height = _size.Height;

        if (bodyRect.top() >= get_field_bounds().Height * 0.8f) {
            _dead = true;
            give_score(100);
        }
    } else {
        _timeOut -= deltaTime;
        if (_timeOut <= 0s) {
            destroy();
        }
    }
}

////////////////////////////////////////////////////////////
}
