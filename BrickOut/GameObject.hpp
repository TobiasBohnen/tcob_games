// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "BrickOut.hpp" // IWYU pragma: keep"

namespace BrickOut {

class field;

class game_object : public updatable {
public:
    game_object(field& parent);
    ~game_object() override;

    virtual void reset() = 0;
    void         update(milliseconds deltaTime) final;

protected:
    void give_score(i32 score);
    void fail();

    void set_material(assets::asset_ptr<gfx::material> const& mat);

    auto get_sprite() -> gfx::rect_shape&;
    auto get_body() -> physics::body&;

    void create_fixture(physics::shape const& shape, physics::fixture_settings const& settings);

    auto get_field_bounds() -> rect_f;
    auto convert_to_physics(rect_f const& screenObject) const -> rect_f;
    auto convert_to_screen(rect_f const& physicsObject) const -> rect_f;

    void destroy();

private:
    field&                            _parent;
    std::shared_ptr<gfx::rect_shape>  _sprite;
    std::shared_ptr<physics::body>    _physicsBody;
    std::shared_ptr<physics::fixture> _physicsFixture;
};

class paddle : public game_object {
public:
    paddle(field& parent, assets::group& resGrp);

    void reset() override;

    void move(direction dir);
    void stop(direction dir);

protected:
    void on_update(milliseconds deltaTime) override;

    direction _dir {direction::None};
    f32       _moveSpeed {0.0f};
    f32       _physicsMoveSpeed {0.0f};
    size_f    _size;
};

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

}
