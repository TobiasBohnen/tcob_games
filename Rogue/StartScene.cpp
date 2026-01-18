// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "StartScene.hpp"
#include "ui/Renderer.hpp"

namespace Rogue {

start_scene::start_scene(game& game)
    : scene {game}
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
    auto  windowSize {*win.Size};

    _mainForm                         = std::make_shared<main_menu>(resGrp, rect_i {point_i::Zero, windowSize});
    root_node().create_child().Entity = _mainForm;

    _renderer = std::make_shared<renderer>(*_mainForm->Terminal);

    locate_service<gfx::render_system>().statistics().reset();
}

void start_scene::on_draw_to(gfx::render_target&)
{
    _master.draw(*_renderer);
}

void start_scene::on_update(milliseconds deltaTime)
{
    _master.update(deltaTime, _actionQueue);
}

void start_scene::on_fixed_update(milliseconds deltaTime)
{
    auto const& stats {locate_service<gfx::render_system>().statistics()};
    auto const& mouse {locate_service<input::system>().mouse().get_position()};
    window().Title = std::format("ValVerde | FPS avg:{:.2f} best:{:.2f} worst:{:.2f} | x:{} y:{} ",
                                 stats.average_FPS(), stats.best_FPS(), stats.worst_FPS(),
                                 mouse.X, mouse.Y);
}

void start_scene::on_key_down(input::keyboard::event const& ev)
{

    switch (ev.ScanCode) {
    case input::scan_code::BACKSPACE:
        parent().pop_current_scene();
        return;
    default: break;
    }

    bool const shiftDown {ev.KeyMods.shift() || ev.KeyMods.caps_lock()};

    switch (ev.KeyCode) {
    case input::key_code::KP_8:
    case input::key_code::UP:       _actionQueue.push(action::Up); break;
    case input::key_code::KP_2:
    case input::key_code::DOWN:     _actionQueue.push(action::Down); break;
    case input::key_code::KP_4:
    case input::key_code::LEFT:     _actionQueue.push(action::Left); break;
    case input::key_code::KP_6:
    case input::key_code::RIGHT:    _actionQueue.push(action::Right); break;
    case input::key_code::KP_7:     _actionQueue.push(action::LeftUp); break;
    case input::key_code::KP_9:     _actionQueue.push(action::RightUp); break;
    case input::key_code::KP_1:     _actionQueue.push(action::LeftDown); break;
    case input::key_code::KP_3:     _actionQueue.push(action::RightDown); break;
    case input::key_code::KP_ENTER:
    case input::key_code::RETURN:   _actionQueue.push(action::Execute); break;
    case input::key_code::KP_5:     _actionQueue.push(action::Center); break;

    case input::key_code::l:        _actionQueue.push(action::LookMode); break;
    case input::key_code::u:        _actionQueue.push(action::UseMode); break;
    case input::key_code::g:        _actionQueue.push(action::Get); break;

    case input::key_code::c:        _actionQueue.push(action::MFDModeCharacter); break;
    case input::key_code::i:        _actionQueue.push(action::MFDModeInventory); break;
    case input::key_code::m:        _actionQueue.push(action::MFDModeMonsters); break;

    case input::key_code::TAB:      _actionQueue.push(action::MFDModeChange); break;

    case input::key_code::s:        _actionQueue.push(action::Search); break;

    default:                        break;
    }
}

}
