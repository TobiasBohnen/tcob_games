// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#pragma once

#include "Common.hpp"

#include "Spacecraft.hpp"
#include "Station.hpp"

////////////////////////////////////////////////////////////

class environment final {
public:
    environment();

    void draw_to(gfx::render_target& target);

    void update(milliseconds deltaTime);
    void fixed_update(milliseconds deltaTime);

    template <typename T>
    auto spawn(T::def const& def) -> std::shared_ptr<T>;

    void remove(object& obj);

private:
    physics::world   _physicsWorld;
    gfx::shape_batch _gfxShapes;
};

////////////////////////////////////////////////////////////

class mission : public gfx::entity {
public:
    mission();

protected:
    void set_station(station::def const& def);
    auto add_ship(spacecraft::def const& def) -> std::shared_ptr<spacecraft>;
    void remove_ship(std::shared_ptr<spacecraft> const& ship);

    void on_draw_to(gfx::render_target& target) override;

    void on_update(milliseconds deltaTime) override;
    void on_fixed_update(milliseconds deltaTime) override;

    void on_key_down(input::keyboard::event const& ev) override;
    void on_key_up(input::keyboard::event const& ev) override;
    void on_mouse_motion(input::mouse::motion_event const& ev) override;
    void on_mouse_button_down(input::mouse::button_event const& ev) override;
    void on_mouse_button_up(input::mouse::button_event const& ev) override;
    void on_mouse_wheel(input::mouse::wheel_event const& ev) override;

private:
    environment _environment;

    std::vector<std::shared_ptr<spacecraft>> _spacecrafts;
    std::shared_ptr<station>                 _station;

    milliseconds _time {0};
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

template <typename T>
inline auto environment::spawn(T::def const& def) -> std::shared_ptr<T>
{
    return std::make_shared<T>(
        def,
        _physicsWorld.create_body({}, {}),
        _gfxShapes.create_shape<gfx::rect_shape>());
}