#include "Object.hpp"

#include <utility>

constexpr f32 physicsFactor {20.0};

object::object(std::shared_ptr<physics::body> body, std::shared_ptr<gfx::rect_shape> shape)
    : _physicsBody {std::move(body)}
    , _gfxShape {std::move(shape)}
{
}

void object::cleanup(physics::world& physicsWorld, gfx::shape_batch& shapeBatch)
{
    if (_physicsBody) {
        physicsWorld.remove_body(*_physicsBody);
        _physicsBody = nullptr;
    }
    if (_gfxShape) {
        shapeBatch.remove_shape(*_gfxShape);
        _gfxShape = nullptr;
    }
}

auto object::body() -> physics::body&
{
    return *_physicsBody;
}

auto object::shape() -> gfx::rect_shape&
{
    return *_gfxShape;
}

void object::update(milliseconds deltaTime)
{
    if (_physicsBody && _gfxShape) {
        rect_f const pb {(*_physicsBody->Transform).Center, size_f::Zero};

        _gfxShape->Bounds   = _gfxShape->Bounds->as_centered_at(convert_to_screen(pb).center());
        _gfxShape->Rotation = (*_physicsBody->Transform).Angle;
    }

    on_update(deltaTime);
}

auto object::convert_to_physics(rect_f const& screenObject) -> rect_f
{
    rect_f retValue;
    retValue.Position.X  = (screenObject.Position.X / physicsFactor);
    retValue.Position.Y  = (screenObject.Position.Y / physicsFactor);
    retValue.Size.Width  = (screenObject.Size.Width / physicsFactor);
    retValue.Size.Height = (screenObject.Size.Height / physicsFactor);
    return retValue;
}

auto object::convert_to_screen(rect_f const& physicsObject) -> rect_f
{
    rect_f retValue;
    retValue.Position.X  = (physicsObject.Position.X * physicsFactor);
    retValue.Position.Y  = (physicsObject.Position.Y * physicsFactor);
    retValue.Size.Width  = (physicsObject.Size.Width * physicsFactor);
    retValue.Size.Height = (physicsObject.Size.Height * physicsFactor);
    return retValue;
}
