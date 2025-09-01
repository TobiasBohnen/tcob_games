// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "BrickOut.hpp" // IWYU pragma: keep"

namespace BrickOut {

class field;
enum class lighting_type {
    None,
    Shadow,
    Light
};
////////////////////////////////////////////////////////////

class game_object : public updatable {
public:
    game_object(field& parent, lighting_type lt);
    ~game_object() override;

    virtual void reset() = 0;
    void         update(milliseconds deltaTime) final;

protected:
    void give_score(i32 score);
    void fail();

    void set_material(asset_ptr<gfx::material> const& mat);

    auto get_sprite() -> gfx::rect_shape&;
    auto get_body() -> physics::body&;
    auto get_light_source() -> gfx::light_source&;
    auto get_shadow_caster() -> gfx::shadow_caster&;

    auto get_field_bounds() -> rect_f;
    auto convert_to_physics(rect_f const& screenObject) const -> rect_f;
    auto convert_to_screen(rect_f const& physicsObject) const -> rect_f;

    void destroy();

    template <typename T>
    inline void create_shape(auto&& shapeSettings);

private:
    field&                              _parent;
    std::shared_ptr<gfx::rect_shape>    _sprite;
    std::shared_ptr<physics::body>      _physicsBody;
    std::shared_ptr<physics::shape>     _physicsShape;
    std::shared_ptr<gfx::light_source>  _lightSource;
    std::shared_ptr<gfx::shadow_caster> _shadowCaster;
};

////////////////////////////////////////////////////////////

class paddle : public game_object {
public:
    paddle(field& parent, assets::group& resGrp);

    void reset() override;

    void move(f32 val);
    void stop();

protected:
    void on_update(milliseconds deltaTime) override;

    f32    _dir {0.0f};
    f32    _moveSpeed {0.0f};
    f32    _physicsMoveSpeed {0.0f};
    size_f _size;
};

////////////////////////////////////////////////////////////

class ball : public game_object {
public:
    ball(field& parent, assets::group& resGrp);

    void reset() override;

protected:
    void on_update(milliseconds deltaTime) override;

private:
    size_f _size;
    f32    _failHeight {0.0f};
};

////////////////////////////////////////////////////////////

class brick : public game_object {
public:
    brick(field& parent, assets::group& resGrp, brick_def def);

    void reset() override;

protected:
    void on_update(milliseconds deltaTime) override;

private:
    brick_def    _def;
    size_f       _size;
    bool         _dead {false};
    milliseconds _timeOut {};
};

////////////////////////////////////////////////////////////

template <typename T>
inline void game_object::create_shape(auto&& shapeSettings)
{
    if (_physicsBody) {
        if (_physicsShape) {
            _physicsBody->remove_shape(*_physicsShape);
            _physicsShape = nullptr;
        }

        _physicsShape = _physicsBody->create_shape<T>(shapeSettings);
    }
}

}
