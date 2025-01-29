#include "Mission.hpp"

////////////////////////////////////////////////////////////

environment::environment()
{
    _physicsWorld.Gravity = point_f::Zero;
}

void environment::remove(object& obj)
{
    obj.cleanup(_physicsWorld, _gfxShapes);
}

void environment::draw_to(gfx::render_target& target)
{
    _gfxShapes.draw_to(target);
}

void environment::update(milliseconds deltaTime)
{
    _gfxShapes.update(deltaTime);
}

void environment::fixed_update(milliseconds deltaTime)
{
    _physicsWorld.update(deltaTime);
}

////////////////////////////////////////////////////////////

mission::mission()
    : gfx::entity {update_mode::Both}
{
}

void mission::on_draw_to(gfx::render_target& target)
{
    _environment.draw_to(target);
}

void mission::on_update(milliseconds deltaTime)
{
    _time += deltaTime;

    _environment.update(deltaTime);

    for (auto& craft : _spacecrafts) {
        craft->update(deltaTime);
    }

    _station->update(deltaTime);
}

void mission::on_fixed_update(milliseconds deltaTime)
{
    _environment.fixed_update(deltaTime);
}

void mission::on_key_down(input::keyboard::event const& ev)
{
}

void mission::on_key_up(input::keyboard::event const& ev)
{
}

void mission::on_mouse_motion(input::mouse::motion_event const& ev)
{
}

void mission::on_mouse_button_down(input::mouse::button_event const& ev)
{
}

void mission::on_mouse_button_up(input::mouse::button_event const& ev)
{
}

void mission::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
}

void mission::set_station(station::def const& def)
{
    _station = _environment.spawn<station>(def);
}

auto mission::add_ship(spacecraft::def const& def) -> std::shared_ptr<spacecraft>
{
    auto ship {_environment.spawn<spacecraft>(def)};
    _spacecrafts.push_back(ship);
    return ship;
}

void mission::remove_ship(std::shared_ptr<spacecraft> const& ship)
{
    for (isize i {0}; i < std::ssize(_spacecrafts); ++i) {
        if (_spacecrafts[i] == ship) {
            _environment.remove(*ship);
            _spacecrafts.erase(_spacecrafts.begin() + i);
            break;
        }
    }
}
