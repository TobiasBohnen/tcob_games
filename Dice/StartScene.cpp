// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "StartScene.hpp"

start_scene::start_scene(game& game)
    : scene {game}
    , _gameNode(&root_node().create_child())
{
    auto& resMgr {library()};
    auto& resGrp {resMgr.create_or_get_group("dice")};
    resGrp.mount("./dice.7z");
    resMgr.load_all_groups();

    auto modules {io::enumerate("/", {.String = "*.die", .MatchWholePath = true}, true)};
    for (auto const& module : modules) { resGrp.mount(module); }
    scan_games();

    auto& win {window()};
    win.ClearColor = colors::Black;

    _selectForm = std::make_shared<game_select_form>(rect_f {win.bounds()}, resGrp, _games);
    _selectForm->StartGame.connect([this](u32 id) { start_game(id); });
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

void start_scene::on_update(milliseconds deltaTime)
{
    if (_quitQueued) {
        _quitQueued       = false;
        _gameNode->Entity = nullptr;
        _gameNode->send_to_back();
        _selectForm->show();
        _currentGameID = 0;
        _currentGame   = nullptr;
        _events        = nullptr;
    }

    if (_queuedGameID != 0) {
        start_game(_queuedGameID);
        _queuedGameID = 0;
    }

    if (_startRecord) {
        if (_clipFtr.valid()) {
            if (_clipFtr.wait_for(0ms) == std::future_status::ready) {
                logger::Info("capture saved");
                _startRecord = false;
                _frames.clear();
                _clipFtr = {};
            }
            return;
        }

        _frameTimer += deltaTime;
        if (_frameTimer >= 50ms) {
            auto img {window().copy_to_image()};
            _frames.push_back({.Image = img, .Duration = _frameTimer});
            _frameTimer = 0ms;
        }
        if (_frames.size() == 500) {
            logger::Info("capture done. saving...");
            _clipFtr = gfx::save_animation_async("clip.gif", _frames);
        }
    }
}

void start_scene::on_fixed_update(milliseconds /* deltaTime */)
{
#if defined(TCOB_DEBUG)
    auto const& stats {locate_service<gfx::render_system>().statistics()};
    auto const& mouse {locate_service<input::system>().mouse().get_position()};
    window().Title = std::format("Dice | FPS avg:{:.2f} best:{:.2f} worst:{:.2f} | x:{} y:{} ",
                                 stats.average_FPS(), stats.best_FPS(), stats.worst_FPS(),
                                 mouse.X, mouse.Y);
#endif
}

void start_scene::on_key_down(input::keyboard::event const& ev)
{
    switch (ev.ScanCode) {
    case input::scan_code::BACKSPACE: parent().pop_current_scene(); break;
    case input::scan_code::S:         {
        auto const fileName {[]() {
            for (i32 i {0};; ++i) {
                auto const name {std::format("screen{:02}.png", i)};
                if (!io::exists(name)) { return name; }
            }
        }()};
        std::ignore = window().copy_to_image().save(fileName);
    } break;
    case input::scan_code::V: {
        _startRecord = true;
    } break;
    default:
        break;
    }
}

void start_scene::start_game(u32 id)
{
    if (!_games.contains(id)) { return; }

    _currentGameID = id;
    _events        = std::make_unique<event_bus>();

    auto* resGrp {library().get_group("dice")};
    _currentGame = std::make_shared<dice_game>(
        dice_game::init {.Dice           = _games[_currentGameID].Dice,
                         .Group          = *resGrp,
                         .RealWindowSize = size_f {window().bounds().Size},
                         .Events         = *_events});

    _events->Restart.connect([this]() { _queuedGameID = _currentGameID; });
    _events->Quit.connect([this]() { _quitQueued = true; });

    _gameNode->Entity = _currentGame;
    _gameNode->bring_to_front();
    _selectForm->hide();
    _currentGame->run(_games[_currentGameID].LuaPath);
}

void start_scene::scan_games()
{
    auto inis {io::enumerate("/", {.String = "*game.ini", .MatchWholePath = true}, true)};
    for (auto const& ini : inis) {
        if (!scan_game(ini)) {
            continue; // TODO: errors
        }
    }
}

auto start_scene::scan_game(string const& ini) -> bool
{
    data::object obj;
    // TODO: errors
    if (!obj.load(ini)) { return false; }

    auto gameDef {obj.get<game_def>()};
    if (!gameDef) { return false; };
    auto& game {*gameDef};

    // Cover
    auto       imgCover {gfx::image::CreateEmpty(COVER_SIZE, gfx::image::format::RGBA)};
    auto const dots {decode_texture_pixels(game.Cover, COVER_SIZE)};
    if (dots.size() == COVER_SIZE.area()) {
        for (i32 y {0}; y < COVER_SIZE.Height; ++y) {
            for (i32 x {0}; x < COVER_SIZE.Width; ++x) {
                imgCover.set_pixel({x, y}, PALETTE[dots[x + (y * COVER_SIZE.Width)]]);
            }
        }
    }
    game.CoverTex->resize(COVER_SIZE, 1, gfx::texture::format::RGBA8);
    game.CoverTex->update_data(imgCover, 0);
    game.CoverTex->regions()["default"] = {.UVRect = {0, 0, 1, 1}, .Level = 0};

    // lua script path
    auto luas {io::enumerate(io::get_parent_folder(ini), {.String = "*game.lua", .MatchWholePath = true}, true)};
    if (luas.size() != 1) { return false; }
    game.LuaPath = *luas.begin();

    _games[game.Number] = game;
    return true;
}
