// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

////////////////////////////////////////////////////////////

class object : public updatable {
public:
    object(std::shared_ptr<physics::body> body, std::shared_ptr<gfx::rect_shape> shape);

    void cleanup(physics::world& physicsWorld, gfx::shape_batch& shapeBatch);

    void update(milliseconds deltaTime) override;

protected:
    auto body() -> physics::body&;
    auto shape() -> gfx::rect_shape&;

    template <typename T>
    void create_shape(auto&& shapeSettings);

    static auto convert_to_physics(rect_f const& screenObject) -> rect_f;
    static auto convert_to_screen(rect_f const& physicsObject) -> rect_f;

private:
    std::shared_ptr<physics::body>   _physicsBody;
    std::shared_ptr<physics::shape>  _physicsShape;
    std::shared_ptr<gfx::rect_shape> _gfxShape;
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

template <typename T>
inline void object::create_shape(auto&& shapeSettings)
{
    if (_physicsBody) {
        if (_physicsShape) {
            _physicsBody->remove_shape(*_physicsShape);
            _physicsShape = nullptr;
        }

        _physicsShape = _physicsBody->create_shape<T>(shapeSettings);
    }
}