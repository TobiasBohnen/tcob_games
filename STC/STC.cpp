// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "STC.hpp"

#include "_test.hpp"

main_scene::main_scene(game& game)
    : scene {game}
{
    //    locate_service<gfx::render_system>().window().grab_input(true);
}

main_scene::~main_scene()
{
}

void main_scene::on_start()
{
    _currentMission    = std::make_shared<test_mission>();
    root_node().Entity = _currentMission;
}

void main_scene::on_finish()
{
}

void main_scene::on_draw_to(gfx::render_target& target)
{
}

void main_scene::on_update(milliseconds deltaTime)
{
}

void main_scene::on_fixed_update(milliseconds deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    auto const& stats {locate_service<gfx::render_system>().statistics()};
    stream << "avg FPS:" << stats.average_FPS();
    stream << " best FPS:" << stats.best_FPS();
    stream << " worst FPS:" << stats.worst_FPS();

    window().Title = "STC |" + stream.str();
}

void main_scene::on_key_down(input::keyboard::event const& ev)
{
}

void main_scene::on_mouse_motion(input::mouse::motion_event const& ev)
{
}

void main_scene::on_mouse_button_up(input::mouse::button_event const& ev)
{
}

void main_scene::on_mouse_button_down(input::mouse::button_event const& ev)
{
}

void main_scene::on_mouse_wheel(input::mouse::wheel_event const& ev)
{
}
