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

auto start_scene::call_lua(std::vector<std::string> const& funcs, lua_params const& args) -> lua_return
{
    using namespace scripting::lua;
    table tab {_luaScript.get_global_table()};
    for (isize i {0}; i < std::ssize(funcs) - 1; ++i) {
        tab = tab[funcs[i]].as<table>();
    }

    return tab[funcs.back()].as<function<lua_return>>()(args);
}

void start_scene::on_start()
{
    auto& resMgr {locate_service<assets::library>()};
    auto& resGrp {resMgr.create_or_get_group("solitaire")};
    resGrp.mount("./solitaire.zip");
    resMgr.load_all_groups();

    auto& window {get_game().get_window()};
    auto  windowSize {window.Size()};
    auto& configFile {locate_service<data::config_file>()};

    // field
    f32 const height {windowSize.Height / 10.f * 9.f};
    _playField = std::make_shared<field>(&get_window(), size_i {windowSize.Width, static_cast<i32>(height)}, resGrp);

    _playField->HoverChange.connect([&](hover_info const& str) {
        _formControls->LblPile->Label      = str.Pile;
        _formControls->LblRule->Label      = str.Rule;
        _formControls->LblCardCount->Label = str.CardCount;
    });

    load_scripts();

    // ui
    rect_i const menuBounds {0, static_cast<i32>(height), windowSize.Width, windowSize.Height - static_cast<i32>(height)};
    _formControls = std::make_shared<form_controls>(&window, rect_f {menuBounds});

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

    _themes = load_themes();
    std::vector<std::string> themes;
    themes.reserve(_themes.size());
    for (auto const& gi : _themes) {
        themes.push_back(gi.first);
    }

    _formMenu = std::make_shared<form_menu>(&window, rect_f {point_f::Zero, size_f {windowSize}}, games, themes);
    _formMenu->hide();
    connect_ui_events();

    // render queues
    get_root_node()->create_child()->attach_entity(_playField);
    get_root_node()->create_child()->attach_entity(_formControls);
    get_root_node()->create_child()->attach_entity(_formMenu);

    locate_service<stats>().reset();

    // load config
    if (configFile.has("sol", "game")) {
        _formMenu->SelectedGame = configFile["sol"]["game"].as<std::string>();
    }

    _formMenu->SelectedTheme = configFile.get<std::string>("sol", "theme").value_or("default");
}

void start_scene::connect_ui_events()
{
    _formControls->BtnStart->Click.connect([&](auto const&) {
        auto const game {_formMenu->SelectedGame()};
        if (_games.contains(game)) {
            _playField->start(_games[game].second(*_playField), false);
            locate_service<stats>().reset();
        }
    });

    _formControls->BtnMenu->Click.connect([&](auto const&) {
        _formMenu->show();
    });

    _formControls->BtnUndo->Click.connect([&](auto const&) {
        _playField->undo();
    });

    _formControls->BtnQuit->Click.connect([&](auto const&) {
        _playField->quit();
        get_game().pop_current_scene();
    });

    _formMenu->SelectedGame.Changed.connect([&](auto const& game) {
        auto& camera {*get_window().Camera};
        camera.set_position(point_f::Zero);
        camera.set_zoom(size_f::One);

        if (_games.contains(game)) {
            _playField->start(_games[game].second(*_playField), true);
            locate_service<stats>().reset();
            locate_service<data::config_file>()["sol"]["game"] = game;
        }
    });

    _formMenu->SelectedTheme.Changed.connect([&](auto const& theme) {
        auto& resMgr {locate_service<assets::library>()};
        auto& resGrp {resMgr.create_or_get_group("solitaire")};

        auto newTheme {theme};
        if (!_themes.contains(theme)) { newTheme = "default"; }

        create_styles(_themes[newTheme], resGrp, *_formMenu->Styles);
        create_styles(_themes[newTheme], resGrp, *_formControls->Styles);

        _formMenu->force_redraw("");
        _formControls->force_redraw("");

        locate_service<data::config_file>()["sol"]["theme"] = newTheme;
    });

    _formMenu->VisibilityChanged.connect([&](bool val) {
        if (val) {
            _formControls->hide();
        } else {
            _formControls->show();
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
                                     _formMenu->SelectedGame(), stat.get_average_FPS(), stat.get_best_FPS(), stat.get_worst_FPS());
}

void start_scene::on_key_down(input::keyboard::event& ev)
{
    if (_formControls->get_focus_widget() != nullptr || _formMenu->get_focus_widget() != nullptr) {
        return;
    }

    switch (ev.ScanCode) {
    case input::scan_code::BACKSPACE:
        get_game().pop_current_scene();
        break;
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
    if (!_formControls->Bounds->contains(ev.Position)) {
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
    {
        games::lua_script_game::CreateAPI(this, _luaScript, _luaFunctions);
        auto const files {io::enumerate("/", {"games.*.lua", false}, true)};
        for (auto const& file : files) {
            (void)_luaScript.run_file(file);
        }
    }

    {
        games::squirrel_script_game::CreateAPI(this, _sqScript, _sqFunctions);
        auto const files {io::enumerate("/", {"games.*.nut", false}, true)};
        for (auto const& file : files) {
            (void)_sqScript.run_file(file);
        }
    }
}

}
