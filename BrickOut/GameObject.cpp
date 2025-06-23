#include "GameObject.hpp"

#include "Field.hpp"

namespace BrickOut {

////////////////////////////////////////////////////////////

game_object::game_object(field& parent, lighting_type lt)
    : _parent {parent}
    , _sprite {parent.create_sprite()}
    , _physicsBody {parent.create_body({})}
    , _lightSource {lt == lighting_type::Light ? _parent.create_light() : nullptr}
    , _shadowCaster {lt == lighting_type::Shadow ? _parent.create_shadow() : nullptr}
{
}

game_object::~game_object()
{
    destroy();
}

void game_object::destroy()
{
    if (_physicsBody) {
        if (_physicsShape) {
            _physicsBody->remove_shape(*_physicsShape);
            _physicsShape = nullptr;
        }
        _parent.remove_body(_physicsBody);
        _physicsBody = nullptr;
    }

    if (_sprite) {
        _parent.remove_sprite(_sprite);
        _sprite = nullptr;
    }

    if (_shadowCaster) {
        _parent.remove_shadow(_shadowCaster);
        _shadowCaster = nullptr;
    }
}

void game_object::update(milliseconds deltaTime)
{
    if (_physicsBody && _sprite) {
        rect_f const pb {(*_physicsBody->Transform).Center, size_f::Zero};

        _sprite->Bounds   = _sprite->Bounds->as_centered_at(convert_to_screen(pb).center());
        _sprite->Rotation = (*_physicsBody->Transform).Angle;

        if (_lightSource) {
            _lightSource->Position = _sprite->Bounds->center();
        }

        if (_shadowCaster) {
            auto const& spriteBounds {*_sprite->Bounds};
            auto const& xform {_sprite->transform()};
            _shadowCaster->Polygon = {xform * spriteBounds.top_left(), xform * spriteBounds.bottom_left(),
                                      xform * spriteBounds.bottom_right(), xform * spriteBounds.top_right()};
        }
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

auto game_object::get_sprite() -> gfx::rect_shape&
{
    return *_sprite;
}

auto game_object::get_body() -> physics::body&
{
    return *_physicsBody;
}

auto game_object::get_light_source() -> gfx::light_source&
{
    return *_lightSource;
}

auto game_object::get_shadow_caster() -> gfx::shadow_caster&
{
    return *_shadowCaster;
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

////////////////////////////////////////////////////////////

paddle::paddle(field& parent, assets::group& resGrp)
    : game_object {parent, lighting_type::None}
{
    set_material(resGrp.get<gfx::material>("paddleBlu"));
    auto& body {get_body()};
    body.UserData = "Paddle";
}

void paddle::reset()
{
    auto& body {get_body()};
    body.LinearVelocity  = {0, 0};
    body.AngularVelocity = degree_f {0};

    auto const rect {get_field_bounds()};
    auto&      sprite {get_sprite()};
    _size         = size_f {rect.width() / 8.0f, rect.height() / 40.0f};
    sprite.Bounds = {point_f::Zero, _size};
    rect_f const physRect {convert_to_physics({(rect.width() - _size.Width) / 2,
                                               (rect.height() - (_size.Height * 2)),
                                               _size.Width,
                                               _size.Height})};

    body.Transform    = physics::body_transform {physRect.center(), degree_f {0}};
    body.Type         = physics::body_type::Static;
    body.GravityScale = 0;

    physics::capsule_shape::settings settings;
    f32 const                        rad {physRect.Size.Height / 2.0f};
    settings.Center0                    = {(-physRect.Size.Width / 2) + rad, 0};
    settings.Center1                    = {(physRect.Size.Width / 2) - rad, 0};
    settings.Radius                     = physRect.Size.Height / 2.0f;
    settings.Shape.Material.Restitution = 0.1f;

    settings.Shape.Density           = 10.0f;
    settings.Shape.Material.Friction = 0.4f;

    create_shape<physics::capsule_shape>(settings);

    _moveSpeed        = rect.width() / 75.f;
    _physicsMoveSpeed = convert_to_physics({{_moveSpeed, 0}, size_f::Zero}).Position.X;
}

void paddle::move(f32 val)
{
    _dir = val;
}

void paddle::stop()
{
    _dir = 0;
}

void paddle::on_update(milliseconds deltaTime)
{
    if (_dir == 0.f) { return; }

    auto& body {get_body()};
    auto  xform {*body.Transform};

    auto const rect {get_field_bounds()};
    auto       bodyRect {convert_to_screen({xform.Center, size_f::Zero})};
    bodyRect.Size.Width = _size.Width + _moveSpeed * 2;
    bodyRect.Position.X -= bodyRect.Size.Width / 2;

    if ((_dir < 0.0f && bodyRect.left() <= rect.left())
        || (_dir > 0.0f && bodyRect.right() >= rect.right())) {
        _dir = 0.0f;
    }

    xform.Center.X += _physicsMoveSpeed * _dir;
    body.Transform = xform;
}

////////////////////////////////////////////////////////////

ball::ball(field& parent, assets::group& resGrp)
    : game_object {parent, lighting_type::Light}
{
    set_material(resGrp.get<gfx::material>("ballBlue"));
    auto& body {get_body()};
    body.UserData = "Ball";
}

void ball::reset()
{
    auto& body {get_body()};
    body.LinearVelocity  = {0, 0};
    body.AngularVelocity = degree_f {0};

    auto const rect {get_field_bounds()};
    auto&      rect_shape {get_sprite()};
    _size             = size_f {rect.width() / 40.0f, rect.height() / 40.0f};
    rect_shape.Bounds = {point_f::Zero, _size};
    rect_f const physRect {convert_to_physics({(rect.width() - _size.Width) / 2,
                                               (rect.height() / 2.0f),
                                               _size.Width,
                                               _size.Height})};

    body.Transform    = physics::body_transform {.Center = physRect.center(), .Angle = degree_f {0}};
    body.Type         = physics::body_type::Dynamic;
    body.GravityScale = 0;

    physics::circle_shape::settings settings;
    settings.Radius = {physRect.Size.Width / 2};

    settings.Shape.Material.Friction    = 0.01f;
    settings.Shape.Material.Restitution = 1.f;
    settings.Shape.Density              = 0.35f;

    create_shape<physics::circle_shape>(settings);

    rng           rand;
    point_f const force {rand(-10.f, 10.f), -10};
    body.apply_linear_impulse(force, physRect.center());

    _failHeight = rect.height() - _size.Height / 2;

    auto& lightSource {get_light_source()};
    lightSource.Color = {255, 255, 255, 128};
    lightSource.Range = 500.0f;
}

void ball::on_update(milliseconds deltaTime)
{
    auto& body {get_body()};
    auto  xform {*body.Transform};

    auto bodyRect {convert_to_screen({xform.Center, size_f::Zero})};
    bodyRect.Size.Height = _size.Height;

    if (bodyRect.bottom() >= _failHeight) { fail(); }

    constexpr f32 minSpeed {20};
    constexpr f32 minSpeedY {5};
    constexpr f32 maxSpeed {50};

    point_f   velocity {*body.LinearVelocity};
    f32 const speedY {std::abs(velocity.Y)};
    if (speedY < minSpeedY) {
        velocity.Y = (velocity.Y > 0) ? minSpeedY : -minSpeedY;
    }
    f64 const speed {velocity.length()};
    if (speed < minSpeed) {
        velocity *= (minSpeed / speed);
    }
    if (speed > maxSpeed) {
        velocity *= (maxSpeed / speed);
    }
    body.LinearVelocity = velocity;
}

////////////////////////////////////////////////////////////

brick::brick(field& parent, assets::group& resGrp, brick_def def)
    : game_object {parent, lighting_type::Shadow}
    , _def {def}
{
    std::string matName {"element_"};
    switch (def.Color) {
    case brick_def::color::Blue:   matName += "blue_"; break;
    case brick_def::color::Green:  matName += "green_"; break;
    case brick_def::color::Grey:   matName += "grey_"; break;
    case brick_def::color::Purple: matName += "purple_"; break;
    case brick_def::color::Red:    matName += "red_"; break;
    case brick_def::color::Yellow: matName += "yellow_"; break;
    }
    switch (def.Shape) {
    case brick_def::shape::Rectangle: matName += "rectangle"; break;
    case brick_def::shape::Square:    matName += "square"; break;
    }

    set_material(resGrp.get<gfx::material>(matName));
    auto& body {get_body()};
    body.UserData = "Brick";
}

void brick::reset()
{
    auto& body {get_body()};

    auto const rect {get_field_bounds()};
    auto&      rect_shape {get_sprite()};
    f32 const  size {rect.width() / 20.f * _def.Size};
    _size             = size_f {_def.Shape == brick_def::shape::Square ? size : size * 2, size};
    rect_shape.Bounds = {point_f::Zero, _size};
    rect_f const physRect {convert_to_physics({rect.Position.X + (rect.width() * _def.Position.X),
                                               rect.Position.Y + (rect.height() * _def.Position.Y),
                                               _size.Width,
                                               _size.Height})};

    body.Transform = physics::body_transform {physRect.center(), degree_f {0}};
    body.Type      = physics::body_type::Dynamic;
    body.IsAwake   = false;

    physics::rect_shape::settings settings;
    settings.Extents = {point_f::Zero, physRect.Size};

    switch (_def.Color) {
    case brick_def::color::Blue:
        settings.Shape.Material.Restitution = 0.1f;
        settings.Shape.Density              = 10.0f;
        settings.Shape.Material.Friction    = 0.0f;
        settings.Shape.Material.CustomColor = colors::Blue;
        _timeOut                            = 4s;
        break;
    case brick_def::color::Green:
        settings.Shape.Material.Restitution = 0.2f;
        settings.Shape.Density              = 10.0f;
        settings.Shape.Material.Friction    = 1.0f;
        settings.Shape.Material.CustomColor = colors::Green;
        _timeOut                            = 3s;
        break;
    case brick_def::color::Grey:
        settings.Shape.Material.Restitution = 0.3f;
        settings.Shape.Density              = 10.0f;
        settings.Shape.Material.Friction    = 0.0f;
        settings.Shape.Material.CustomColor = colors::Gray;
        _timeOut                            = 2s;
        break;
    case brick_def::color::Purple:
        settings.Shape.Material.Restitution = 0.4f;
        settings.Shape.Density              = 10.0f;
        settings.Shape.Material.Friction    = 0.4f;
        settings.Shape.Material.CustomColor = colors::Purple;
        _timeOut                            = 10s;
        break;
    case brick_def::color::Red:
        settings.Shape.Material.Restitution = 0.5f;
        settings.Shape.Density              = 10.0f;
        settings.Shape.Material.Friction    = 0.3f;
        settings.Shape.Material.CustomColor = colors::Red;
        _timeOut                            = 1s;
        break;
    case brick_def::color::Yellow:
        settings.Shape.Material.Restitution = 0.6f;
        settings.Shape.Density              = 10.0f;
        settings.Shape.Material.Friction    = 0.6f;
        settings.Shape.Material.CustomColor = colors::Yellow;
        _timeOut                            = 5s;
        break;
    }

    create_shape<physics::rect_shape>(settings);
}

void brick::on_update(milliseconds deltaTime)
{
    if (!_dead) {
        auto& body {get_body()};
        auto  xform {*body.Transform};

        auto bodyRect {convert_to_screen({xform.Center, size_f::Zero})};
        bodyRect.Size.Height = _size.Height;

        if (bodyRect.top() >= get_field_bounds().Size.Height * 0.8f) {
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
