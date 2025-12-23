// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "StartScene.hpp"

#include <iomanip>

start_scene::start_scene(game& game)
    : scene {game}
    , _gameNode(&root_node().create_child())
{
    auto& resMgr {library()};
    auto& resGrp {resMgr.create_or_get_group("dice")};
    resGrp.mount("./dice.zip");
    resMgr.load_all_groups();

    auto modules {io::enumerate("/", {.String = "*.die", .MatchWholePath = true}, true)};
    for (auto const& module : modules) { resGrp.mount(module); }
    scan_games();

    auto& win {window()};
    win.ClearColor = colors::Black;

    _selectForm = std::make_shared<game_select_form>(rect_f {win.bounds()}, resGrp, _games);
    _selectForm->StartGame.connect([this](i32 id) { _startGameID = id; });
    root_node().create_child().Entity = _selectForm;
}

start_scene::~start_scene() = default;

void start_scene::on_start()
{
    locate_service<gfx::render_system>().statistics().reset();
}

void start_scene::on_draw_to(gfx::render_target&)
{
}

void start_scene::on_update(milliseconds)
{
    if (_games.contains(_startGameID)) {
        _currentGameID = _startGameID;
        _startGameID   = 0;

        auto* resGrp {library().get_group("dice")};
        _currentGame = std::make_shared<base_game>(base_game::init {
            .Group          = *resGrp,
            .RealWindowSize = size_f {window().bounds().Size}});

        _currentGame->Restart.connect([this]() {
            _startGameID = _currentGameID;
        });
        _currentGame->Quit.connect([this]() {
            _gameNode->Entity = nullptr;
            _gameNode->send_to_back();
            _selectForm->show();
        });

        _gameNode->Entity = _currentGame;
        _gameNode->bring_to_front();
        _selectForm->hide();
        _currentGame->run(_games[_currentGameID].LuaPath);
    }
}

void start_scene::on_fixed_update(milliseconds deltaTime)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2);
    auto const& stats {locate_service<gfx::render_system>().statistics()};
    stream << "avg FPS:" << stats.average_FPS();
    stream << " best FPS:" << stats.best_FPS();
    stream << " worst FPS:" << stats.worst_FPS();

    window().Title = "Dice >| " + stream.str();
}

void start_scene::on_key_down(input::keyboard::event const& ev)
{
    switch (ev.ScanCode) {
    case input::scan_code::BACKSPACE: parent().pop_current_scene(); break;
    case input::scan_code::S:         {
        auto const fileName {[]() {
            for (i32 i {0};; ++i) {
                auto const name {std::format("screen{:02}.webp", i)};
                if (!io::exists(name)) { return name; }
            }
        }()};
        std::ignore = window().copy_to_image().save(fileName);
    } break;
    default:
        break;
    }
}

void start_scene::scan_games()
{
    auto inis {io::enumerate("/", {.String = "*game.ini", .MatchWholePath = true}, true)};
    for (auto const& ini : inis) {
        data::object obj;
        // TODO: errors
        if (!obj.load(ini)) { continue; }

        game_def game {};
        if (!obj.try_get(game.Number, "Number")) { continue; };
        if (game.Number == 0) { continue; }
        if (!obj.try_get(game.Name, "Name")) { continue; };
        if (!obj.try_get(game.Genre, "Genre")) { continue; };

        string cover;
        if (!obj.try_get(cover, "Cover")) { continue; };
        auto       imgCover {gfx::image::CreateEmpty(COVER_SIZE, gfx::image::format::RGBA)};
        auto const dots {decode_texture_pixels(cover, COVER_SIZE)};
        for (i32 y {0}; y < COVER_SIZE.Height; ++y) {
            for (i32 x {0}; x < COVER_SIZE.Width; ++x) {
                imgCover.set_pixel({x, y}, PALETTE[dots[x + (y * COVER_SIZE.Width)]]);
            }
        }
        game.Cover->resize(COVER_SIZE, 1, gfx::texture::format::RGBA8);
        game.Cover->update_data(imgCover, 0);
        game.Cover->regions()["default"] = {.UVRect = {0, 0, 1, 1}, .Level = 0};

        auto luas {io::enumerate(io::get_parent_folder(ini), {.String = "*game.lua", .MatchWholePath = true}, true)};
        if (luas.size() != 1) { continue; }
        game.LuaPath = *luas.begin();

        _games[game.Number] = game;
    }
}
