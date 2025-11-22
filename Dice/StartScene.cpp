// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "StartScene.hpp"

#include <iomanip>

start_scene::start_scene(game& game)
    : scene {game}
{
}

start_scene::~start_scene() = default;

void start_scene::on_start()
{
    auto& resMgr {library()};
    auto& resGrp {resMgr.create_or_get_group("dice")};
    resGrp.mount("./dice.zip");
    resMgr.load_all_groups();

    auto& win {window()};
    win.ClearColor = colors::White;
    _currentGame   = std::make_shared<base_game>(resGrp, win.bounds().Size);
    _currentGame->run("dice/games/SpaceRocks/game.lua");

    root_node().create_child().Entity = _currentGame;

    locate_service<gfx::render_system>().statistics().reset();
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
    auto const& stats {locate_service<gfx::render_system>().statistics()};
    stream << "avg FPS:" << stats.average_FPS();
    stream << " best FPS:" << stats.best_FPS();
    stream << " worst FPS:" << stats.worst_FPS();

    window().Title = "Dice |" + stream.str();
}

void start_scene::on_key_down(input::keyboard::event const& ev)
{
    switch (ev.ScanCode) {
    case input::scan_code::BACKSPACE:
        parent().pop_current_scene();
        break;
    case input::scan_code::R: {
        auto _ = window().copy_to_image().save("screen01.webp");
    } break;
    default:
        break;
    }
}
