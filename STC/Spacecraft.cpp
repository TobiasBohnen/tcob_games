#include "Spacecraft.hpp"

#include <utility>

////////////////////////////////////////////////////////////

spacecraft::spacecraft(def const& def, std::shared_ptr<physics::body> body, std::shared_ptr<gfx::rect_shape> shape)
    : object {std::move(body), std::move(shape)}
    , _def {def}
{
    setup();
}

void spacecraft::on_update(milliseconds deltaTime)
{
    degree_f const targetAngle {0};

    f32 const      seconds {static_cast<f32>(deltaTime.count() / 1000)};
    auto&          body {this->body()};
    degree_f const bodyAngle {body.Transform().Angle.as_normalized(angle_normalize::PositiveFullTurn)};

    auto       velocity {body.LinearVelocity()};
    f32 const  acceleration {_def.MaxAcceleration * seconds};
    auto const dir {point_f::FromDirection(bodyAngle)};
    body.apply_force_to_center(dir * (acceleration));

    if (velocity.length() > _def.MaxSpeed) {
        velocity *= _def.MaxSpeed / velocity.length();
        body.LinearVelocity = velocity;
    }

    f32 const turnRate {_def.MaxTurnrate.Value * seconds};
    if (bodyAngle > targetAngle + degree_f {1} && bodyAngle < targetAngle - degree_f {1}) {
        if (bodyAngle > 180) {
            body.apply_torque(turnRate);
        } else {
            body.apply_torque(-turnRate);
        }
    } else {
        body.AngularVelocity = radian_f {0};
    }
}

void spacecraft::setup()
{
    auto& body {this->body()};
    body.LinearVelocity  = {0, 0};
    body.AngularVelocity = degree_f {0};

    auto& shape {this->shape()};
    shape.Bounds   = _def.Bounds;
    shape.Rotation = _def.Rotation;
    shape.Color    = colors::Blue;
    shape.Material = gfx::material::Empty();

    rect_f const physRect {convert_to_physics(shape.Bounds())};
    body.Transform    = physics::body_transform {physRect.center(), shape.Rotation()};
    body.Type         = physics::body_type::Dynamic;
    body.GravityScale = 0;
    logger::Info("{}", _def.MaxTurnrate.Value);

    physics::rect_shape::settings settings;
    settings.Extents                 = {point_f::Zero, physRect.Size};
    settings.Shape.Density           = 10.0f;
    settings.Shape.Material.Friction = 0;

    create_shape<physics::rect_shape>(settings);
}
