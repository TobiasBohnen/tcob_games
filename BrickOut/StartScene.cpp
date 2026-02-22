// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "StartScene.hpp"

namespace BrickOut {

start_scene::start_scene(game& game)
    : scene {game}
{
    window().ClearColor = colors::Black;
}

start_scene::~start_scene() = default;

std::vector<brick_def> level0 {};

std::vector<brick_def> level1 {
    blue_rect {{0.10f, 0.1f}},
    green_rect {{0.25f, 0.1f}},
    blue_rect {{0.40f, 0.1f}},
    grey_rect {{0.55f, 0.1f}},
    blue_rect {{0.70f, 0.1f}},
    green_rect {{0.85f, 0.1f}},

    green_rect {{0.10f, 0.175f}},
    blue_rect {{0.25f, 0.175f}},
    grey_rect {{0.40f, 0.175f}},
    red_rect {{0.55f, 0.175f}},
    green_rect {{0.70f, 0.175f}},
    blue_rect {{0.85f, 0.175f}},

    grey_square {{0.125f, 0.25f}},
    blue_square {{0.275f, 0.25f}},
    green_square {{0.425f, 0.25f}},
    blue_square {{0.575f, 0.25f}},
    yellow_square {{0.725f, 0.25f}},
    blue_square {{0.875f, 0.25f}},

    grey_rect {{0.10f, 0.325f}},
    blue_rect {{0.25f, 0.325f}},
    green_rect {{0.40f, 0.325f}},
    blue_rect {{0.55f, 0.325f}},
    purple_rect {{0.70f, 0.325f}},
    blue_rect {{0.85f, 0.325f}},

};

void start_scene::on_start()
{
    auto& resMgr {library()};
    auto& resGrp {resMgr.create_or_get_group("brickout")};
    resGrp.mount("./brickout.zip");
    resMgr.load_all_groups();

    auto& win {window()};
    auto  windowSize {*win.Size};

    i32 const padding {(windowSize.Width / 4 * 3) - windowSize.Height};

    _playField = std::make_shared<field>(resGrp, padding, windowSize);
    _playField->Score.Changed.connect([&](auto val) {
        _mainForm->LblScore->Label = std::to_string(val);
    });

    rect_i const menuBounds {windowSize.Height + padding, 0, windowSize.Width - windowSize.Height - padding, windowSize.Height};
    _mainForm = std::make_shared<main_menu>(menuBounds);
    _mainForm->BtnStart->Click.connect([&, windowSize](auto const&) {
        _playField->start(level1);
    });
    _mainForm->BtnQuit->Click.connect([&](auto const&) {
        parent().pop_current_scene();
    });

    root_node().create_child().Entity = _playField;
    root_node().create_child().Entity = _mainForm;

    locate_service<gfx::render_system>().statistics().reset();
}

void start_scene::on_draw_to(gfx::render_target&, transform const&)
{
}

void start_scene::on_update(milliseconds)
{
}

void start_scene::on_fixed_update(milliseconds deltaTime)
{
    string state;

    if (_playField->state() == game_state::Failure) {
        state = "| GAME OVER ";
    }
    if (_playField->state() == game_state::Success) {
        state = "| A WINNER IS YOU ";
    }

    auto const& stats {locate_service<gfx::render_system>().statistics()};
    auto const& mouse {locate_service<input::system>().mouse().get_position()};
    window().Title = std::format("BrickOut | FPS avg:{:.2f} best:{:.2f} worst:{:.2f} | x:{} y:{} {} ",
                                 stats.average_FPS(), stats.best_FPS(), stats.worst_FPS(),
                                 mouse.X, mouse.Y,
                                 state);
}

void start_scene::on_key_down(input::keyboard::event const& ev)
{
    switch (ev.ScanCode) {
    case input::scan_code::BACKSPACE:
        parent().pop_current_scene();
        break;
    default:
        break;
    }
}

void start_scene::on_controller_button_down(input::controller::button_event const& ev)
{
    if (ev.Button == input::controller::button::Start) {
        _playField->start(level1);
        ev.Handled = true;
    } else if (ev.Button == input::controller::button::Back) {
        parent().pop_current_scene();
    }
}
}
