// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "StartScene.hpp"

#include <iomanip>

namespace GAME_NAME {

start_scene::start_scene(game& game)
    : scene(game)
{
}

start_scene::~start_scene() = default;

void start_scene::on_start()
{
    auto& resMgr {get_game().get_library()};
    auto& resGrp {resMgr.create_or_get_group("GAME_NAME_LOWER")};
    resGrp.mount("./GAME_NAME_LOWER.zip");
    resMgr.load_all_groups();

    auto& window {get_game().get_window()};
    auto  windowSize {window.Size()};

    _playField = std::make_shared<field>();

    rect_i const menuBounds {windowSize.Height, 0, windowSize.Width - windowSize.Height, windowSize.Height};
    _mainForm = std::make_shared<main_menu>(&window, rect_f {menuBounds});
    _mainForm->BtnStart->Click.connect([&, windowSize](auto const&) {
        _playField->start();
    });
    _mainForm->BtnQuit->Click.connect([&](auto const&) {
        get_game().pop_current_scene();
    });

    auto renderQ0 {std::make_shared<gfx::render_queue>()};
    add_render_queue(100, renderQ0);
    add_entity(_playField, renderQ0);
    auto renderQ1 {std::make_shared<gfx::render_queue>()};
    add_render_queue(999, renderQ1);
    add_entity(_mainForm, renderQ1);

    locate_service<stats>().reset();
}

void start_scene::on_draw_to(gfx::render_target&)
{
}

void start_scene::on_update(milliseconds)
{
}

void start_scene::on_fixed_update(milliseconds deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    stream << " avg FPS:" << locate_service<stats>().get_average_FPS();
    stream << " best FPS:" << locate_service<stats>().get_best_FPS();
    stream << " worst FPS:" << locate_service<stats>().get_worst_FPS();
    if (_playField->state() == game_state::Failure) {
        stream << "| GAME OVER ";
    }
    if (_playField->state() == game_state::Success) {
        stream << "| A WINNER IS YOU ";
    }

    get_window().Title = "GAME_NAME |" + stream.str();
}

void start_scene::on_key_down(input::keyboard::event& ev)
{
    switch (ev.ScanCode) {
    case input::scan_code::BACKSPACE:
        get_game().pop_current_scene();
        break;
    default:
        break;
    }
}

}
