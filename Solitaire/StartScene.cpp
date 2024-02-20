// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "StartScene.hpp"

namespace solitaire {

start_scene::start_scene(game& game)
    : scene {game}
{
}

start_scene::~start_scene() = default;

void start_scene::register_game(games::game_info const& info, func&& game)
{
    _games[info.Name] = {info, std::move(game)};
}

void start_scene::on_start()
{
    auto& resMgr {locate_service<assets::library>()};
    auto& resGrp {resMgr.create_or_get_group("solitaire")};
    resGrp.mount("./solitaire.zip");
    resMgr.load_all_groups();

    auto& window {get_game().get_window()};
    auto  windowSize {window.Size()};

    // field
    f32 const width {windowSize.Width / 6.f * 5.f};
    _playField = std::make_shared<field>(&get_window(), size_i {static_cast<i32>(width), windowSize.Height}, resGrp);

    _playField->HoverChange.connect([&](std::string const& str) {
        _mainForm->LblInfo->Label = str;
    });

    load_scripts();

    // ui
    rect_i const menuBounds {static_cast<i32>(width), 0, windowSize.Width - static_cast<i32>(width), windowSize.Height};
    _mainForm = std::make_shared<main_menu>(&window, rect_f {menuBounds});

#if defined(TCOB_DEBUG)
    io::ofstream stream {"games.txt"};
    for (auto const& g : _games) {
        stream << g.first << "\n";
    }
#endif

    std::vector<games::game_info> games;
    games.reserve(_games.size());
    for (auto const& gi : _games) {
        games.push_back(gi.second.first);
    }

    _gameForm = std::make_shared<game_list>(&window, rect_f {point_f::Zero, size_f {windowSize}}, games);
    _gameForm->hide();
    connect_ui_events();

    // render queues
    get_root_node()->create_child()->attach_entity(_playField);
    get_root_node()->create_child()->attach_entity(_mainForm);
    get_root_node()->create_child()->attach_entity(_gameForm);

    locate_service<stats>().reset();
}

void start_scene::connect_ui_events()
{
    _mainForm->BtnStart->Click.connect([&](auto const&) {
        auto const game {_gameForm->SelectedGame()};
        if (_games.contains(game)) {
            _playField->start(_games[game].second(*_playField), false);
            locate_service<stats>().reset();
        }
    });

    _mainForm->LbxGames->SelectedItemIndex.Changed.connect([&](auto val) {
        if (val == -1) { return; }

        _gameForm->SelectedGame = _mainForm->LbxGames->get_selected_item();
    });

    for (auto const& kvp : _games) {
        _mainForm->LbxGames->add_item(kvp.first);
    }

    _mainForm->BtnGames->Click.connect([&](auto const&) {
        _gameForm->show();
    });

    _mainForm->BtnUndo->Click.connect([&](auto const&) {
        _playField->undo();
    });

    _mainForm->BtnQuit->Click.connect([&](auto const&) {
        _playField->quit();
        get_game().pop_current_scene();
    });

    _gameForm->SelectedGame.Changed.connect([&](auto const& game) {
        _mainForm->LbxGames->select_item(game);

        auto& camera {*get_window().Camera};
        camera.set_position(point_f::Zero);
        camera.set_zoom(size_f::One);

        _playField->start(_games[game].second(*_playField), true);
        locate_service<stats>().reset();
    });

    _gameForm->VisibilityChanged.connect([&](bool val) {
        if (val) {
            _mainForm->hide();
        } else {
            _mainForm->show();
        }
    });
}

void start_scene::on_draw_to(gfx::render_target&)
{
}

void start_scene::on_update(milliseconds)
{
}

void start_scene::on_fixed_update(milliseconds /* deltaTime */)
{
    auto stat {locate_service<stats>()};

    get_window().Title = std::format("Solitaire {} | avg FPS: {:.2f} best FPS: {:.2f} worst FPS: {:.2f}",
                                     _gameForm->SelectedGame(), stat.get_average_FPS(), stat.get_best_FPS(), stat.get_worst_FPS());
}

void start_scene::on_key_down(input::keyboard::event& ev)
{
    if (_mainForm->get_focus_widget() != nullptr || _gameForm->get_focus_widget() != nullptr) {
        return;
    }

    switch (ev.ScanCode) {
    case input::scan_code::BACKSPACE:
        get_game().pop_current_scene();
        break;
    case input::scan_code::R: {
        auto files {io::enumerate("/", "*.games.lua", true)};
        for (auto const& file : files) {
            [[maybe_unused]] auto _ {_script.run_file(file)};
        }
        ev.Handled = true;
    } break;
    default:
        break;
    }
}

void start_scene::on_mouse_motion(input::mouse::motion_event& ev)
{
    if (input::system::IsMouseButtonDown(input::mouse::button::Right)) {
        auto&         camera {*get_window().Camera};
        size_f const  zoom {camera.get_zoom()};
        point_f const off {-ev.RelativeMotion.X / zoom.Width, -ev.RelativeMotion.Y / zoom.Height};
        camera.move_by(off);
        ev.Handled = true;
    }
}

void start_scene::on_mouse_wheel(input::mouse::wheel_event& ev)
{
    if (!_mainForm->Bounds->contains(ev.Position)) {
        auto& camera {*get_window().Camera};
        if (ev.Scroll.Y > 0) {
            camera.zoom_by({1.1f, 1.1f});
        } else {
            camera.zoom_by({1 / 1.1f, 1 / 1.1f});
        }
        ev.Handled = true;
    }
}

void start_scene::load_scripts()
{
    using namespace scripting::lua;

    games::script_game::CreateAPI(this, _script, _functions);

    auto files {io::enumerate("/", "*.games.lua", true)};
    for (auto const& file : files) {
        [[maybe_unused]] auto _ {_script.run_file(file)};
    }
}

}
