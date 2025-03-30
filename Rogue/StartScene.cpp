// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "StartScene.hpp"

#include <iomanip>

namespace Rogue {

start_scene::start_scene(game& game)
    : scene(game)
{
    window().ClearColor = colors::Black;
}

start_scene::~start_scene() = default;

void start_scene::on_start()
{
    auto& resMgr {library()};
    auto& resGrp {resMgr.create_or_get_group("rogue")};
    resGrp.mount("./rogue.zip");
    resMgr.load_all_groups();

    auto& win {window()};
    auto  windowSize {win.Size()};

    _mainForm = std::make_shared<main_menu>(resGrp, rect_i {point_i::Zero, windowSize});

    root_node()->create_child()->Entity = _mainForm;

    locate_service<gfx::render_system>().stats().reset();
}

void start_scene::on_draw_to(gfx::render_target&)
{
    _level.draw(*_mainForm->Terminal, _mapOffset);
}

void start_scene::on_update(milliseconds)
{
}

void start_scene::on_fixed_update(milliseconds deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    auto const& stats {locate_service<gfx::render_system>().stats()};
    stream << "avg FPS:" << stats.average_FPS();
    stream << " best FPS:" << stats.best_FPS();
    stream << " worst FPS:" << stats.worst_FPS();

    window().Title = "Roguefort |" + stream.str();
}

void start_scene::on_key_down(input::keyboard::event const& ev)
{
    switch (ev.ScanCode) {
    case input::scan_code::BACKSPACE:
        parent().pop_current_scene();
        break;
    case input::scan_code::UP: _level.move_player(direction::Up); break;
    case input::scan_code::DOWN: _level.move_player(direction::Down); break;
    case input::scan_code::LEFT: _level.move_player(direction::Left); break;
    case input::scan_code::RIGHT: _level.move_player(direction::Right); break;
    default:
        break;
    }
}

}
