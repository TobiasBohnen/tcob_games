// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "B2DebugDraw.hpp"
#include "BrickOut.hpp" // IWYU pragma: keep"
#include "GameObject.hpp"

////////////////////////////////////////////////////////////

namespace BrickOut {

class field : public gfx::entity {
public:
    field(assets::group& resGrp, i32 padding, size_i windowSize);

    prop<i32> Score;

    void start(std::span<brick_def const> bricks);
    void fail();

    auto state() const -> game_state;

    auto create_sprite() -> std::shared_ptr<gfx::rect_shape>;
    void remove_sprite(std::shared_ptr<gfx::rect_shape> const& rect_shape);

    auto create_body(physics::body::settings bodySettings) -> std::shared_ptr<physics::body>;
    void remove_body(std::shared_ptr<physics::body> const& body);

    auto create_light() -> std::shared_ptr<gfx::light_source>;

    auto create_shadow() -> std::shared_ptr<gfx::shadow_caster>;
    void remove_shadow(std::shared_ptr<gfx::shadow_caster> const& sc);

    auto get_field_bounds() const -> rect_f;
    auto get_physics_bounds() const -> rect_f;
    auto convert_to_physics(rect_f const& screenObject) const -> rect_f;
    auto convert_to_screen(rect_f const& physicsObject) const -> rect_f;
    void give_score(i32 score);

protected:
    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_draw_to(gfx::render_target& target) override;
    auto can_draw() const -> bool override;

    void on_key_down(input::keyboard::event const& ev) override;
    void on_key_up(input::keyboard::event const& ev) override;
    void on_controller_button_down(input::controller::button_event const& ev) override;
    void on_controller_button_up(input::controller::button_event const& ev) override;
    void on_controller_axis_motion(input::controller::axis_event const& ev) override;

private:
    assets::group& _resGrp;

    rng        _rand {};
    game_state _state {game_state::Initial};

    rect_f _bounds;

    gfx::shape_batch _spriteBatch;

    debug_mode   _debug {debug_mode::Off};
    B2DDebugDraw _debugDraw;

    gfx::lighting_system _lightingSystem;

    physics::world _physicsWorld;
    rect_f         _physicsBounds;

    paddle                                  _paddle;
    ball                                    _ball;
    std::vector<brick_def>                  _brickDefs;
    std::vector<std::unique_ptr<brick>>     _bricks {};
    assets::owning_asset_ptr<gfx::material> _edgeMat;

    i32 _hitCount {0};
};

}
