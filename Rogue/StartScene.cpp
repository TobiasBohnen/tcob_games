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
    _master.draw(*_mainForm->Terminal);
}

void start_scene::on_update(milliseconds deltaTime)
{
    _master.update(deltaTime, _actionQueue);
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
        return;
    default: break;
    }
    switch (ev.KeyCode) {
    case input::key_code::KP_8:
    case input::key_code::UP: _actionQueue.push(action::MoveUp); break;
    case input::key_code::KP_2:
    case input::key_code::DOWN: _actionQueue.push(action::MoveDown); break;
    case input::key_code::KP_4:
    case input::key_code::LEFT: _actionQueue.push(action::MoveLeft); break;
    case input::key_code::KP_6:
    case input::key_code::RIGHT: _actionQueue.push(action::MoveRight); break;
    case input::key_code::KP_7: _actionQueue.push(action::MoveLeftUp); break;
    case input::key_code::KP_9: _actionQueue.push(action::MoveRightUp); break;
    case input::key_code::KP_1: _actionQueue.push(action::MoveLeftDown); break;
    case input::key_code::KP_3: _actionQueue.push(action::MoveRightDown); break;
    case input::key_code::KP_ENTER:
    case input::key_code::RETURN: _actionQueue.push(action::Execute); break;
    case input::key_code::KP_5:
    case input::key_code::l: _actionQueue.push(action::LookMode); break;
    default: break;
    }
}

}
