// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "StartScene.hpp"

#include "ui/Styles.hpp"
#include "wizard/WizardScene.hpp"

namespace solitaire {

static char const* SAVE_NAME {"save.ini"};
static char const* SETTINGS_NAME {"settings"};

auto static get_size(string_view str) -> size_i
{
    i32                    width {}, height {};
    size_t const           xPos {str.find('x')};
    std::string_view const widthStrView(str.data(), xPos);
    std::string_view const heightStrView(str.data() + xPos + 1);
    std::from_chars(widthStrView.data(), widthStrView.data() + widthStrView.size(), width);
    std::from_chars(heightStrView.data(), heightStrView.data() + heightStrView.size(), height);
    return {width, height};
}

start_scene::start_scene(game& game)
    : scene {game}
{
    _saveGame.load(SAVE_NAME);

    auto& config {locate_service<data::config_file>()};
    config.try_get(_settings, SETTINGS_NAME); // TODO: check version
}

start_scene::~start_scene() = default;

void start_scene::register_game(game_info const& info, reg_game_func&& game)
{
    if (info.DeckCount > 24) { return; }                                                   // TODO: error
    if (info.DeckCount * info.DeckSuits.size() * info.DeckSuits.size() > 1500) { return; } // TODO: error
    if (_games.size() > 2500) { return; }                                                  // TODO: error

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
    resGrp.mount("./solitide.zip");
    resMgr.load_all_groups();

    auto& window {get_window()};
    auto  windowSize {window.Size()};

    auto defaultCursor {resGrp.get<gfx::cursor>("default")};
    window.Cursor             = defaultCursor;
    defaultCursor->ActiveMode = "cursor32";

    load_scripts();

    // themes
    _themes = load_themes();

    // cardsets
    _cardSets = load_cardsets();

    menu_sources source;
    source.Games.reserve(_games.size());
    for (auto const& x : _games) { source.Games.emplace_back(x.second.first); }
    source.Themes.reserve(_themes.size());
    for (auto const& x : _themes) { source.Themes.push_back(x.first); }
    source.Cardsets.reserve(_cardSets.size());
    for (auto const& x : _cardSets) { source.Cardsets.push_back(x.first); }

    // games
    _db.insert_games(source.Games);

    // ui
    _formControls = std::make_shared<form_controls>(&window, resGrp);

    _formMenu = std::make_shared<form_menu>(&window, resGrp, source);
    _formMenu->hide();
    connect_ui_events();

    // card table
    _cardTable = std::make_shared<card_table>(&window, _formControls->Canvas.get(), resGrp);

    _cardTable->HoverChange.connect([&](pile_description const& str) { _formControls->set_pile_labels(str); });

    set_children_bounds(windowSize);

    // render queues
    get_root_node()->create_child()->attach_entity(_cardTable);
    get_root_node()->create_child()->attach_entity(_formControls);
    get_root_node()->create_child()->attach_entity(_formMenu);

    locate_service<stats>().reset();

    // config
    _formMenu->SelectedTheme   = _settings.Theme;
    _formMenu->SelectedCardset = _settings.Cardset;

    _formMenu->fixed_update(0s);     // updates style
    _formControls->fixed_update(0s); // updates style

    if (!_settings.Game.empty()) {
        _formMenu->SelectedGame = _settings.Game;
        start_game(_formMenu->SelectedGame(), start_reason::Resume);
    }
}

void start_scene::connect_ui_events()
{
    _formControls->BtnNewGame->Click.connect([&](auto const&) { start_game(_formMenu->SelectedGame(), start_reason::Restart); });
    _formControls->BtnWizard->Click.connect([&](auto const&) { start_wizard(); });
    _formControls->BtnMenu->Click.connect([&](auto const&) { _formMenu->show(); });

    _formControls->BtnHint->Click.connect([&](auto const&) { _cardTable->show_next_hint(); });
    _formControls->BtnCollect->Click.connect([&](auto const&) {
        if (auto* game {_cardTable->game()}) { game->collect_all(); }
    });
    _formControls->BtnUndo->Click.connect([&](auto const&) {
        if (auto* game {_cardTable->game()}) { game->undo(); }
    });

    _formControls->BtnQuit->Click.connect([&](auto const&) {
        if (auto* game {_cardTable->game()}) {
            game->save(_saveGame);
            _saveGame.save(SAVE_NAME);
            auto& config {locate_service<data::config_file>()};
            config[SETTINGS_NAME] = _settings;
        }
        get_game().pop_current_scene();
    });

    _formMenu->SelectedGame.Changed.connect([&](auto const& game) {
        update_stats(game);
    });

    _formMenu->BtnStartGame->Click.connect([&]() {
        start_game(_formMenu->SelectedGame, start_reason::Resume);
    });

    _formMenu->SelectedTheme.Changed.connect([&](auto const& theme) {
        auto& resMgr {locate_service<assets::library>()};
        auto& resGrp {resMgr.create_or_get_group("solitaire")};

        auto themeName {theme};
        if (!_themes.contains(theme)) { themeName = "default"; }
        auto const& newTheme {_themes[themeName]};

        styles     styles {resGrp};
        auto const styleCollection {styles.load(newTheme)};
        _formMenu->Styles = styleCollection;
        _formMenu->fixed_update(milliseconds {0});

        _formControls->Styles = styleCollection;
        _formControls->fixed_update(milliseconds {0});

        _cardTable->set_theme(newTheme);
        _settings.Theme = themeName;
    });

    _formMenu->SelectedCardset.Changed.connect([&](auto const& cardset) {
        auto newCardset {cardset};
        if (!_cardSets.contains(cardset)) { newCardset = "default"; }

        _settings.Cardset = newCardset;

        _cardTable->set_cardset(_cardSets[newCardset]);
        start_game(_formMenu->SelectedGame(), start_reason::Resume);
    });

    _formMenu->BtnApplySettings->Click.connect([&]() {
        data::config::object obj;
        _formMenu->submit_settings(obj);

        assert(obj.has("ddlResolution", "selected"));
        assert(obj.has("chkFullScreen", "checked"));
        assert(obj.has("chkVSync", "checked"));

        auto& window {get_window()};

        window.FullScreen = obj["chkFullScreen"]["checked"].as<bool>();
        window.VSync      = obj["chkVSync"]["checked"].as<bool>();

        auto const res {get_size(obj["ddlResolution"]["selected"].as<string>())};
        window.Size = res;
        set_children_bounds(res);

        start_game(_formMenu->SelectedGame(), start_reason::Resume);
    });
}

void start_scene::on_draw_to(gfx::render_target&)
{
}

void start_scene::on_update(milliseconds)
{
}

void start_scene::on_fixed_update(milliseconds deltaTime)
{
    if (auto* game {_cardTable->game()}) {
        game->update(deltaTime);
        _formControls->set_game_labels(game);
    }
#if defined(TCOB_DEBUG)
    auto stat {locate_service<stats>()};
    get_window().Title = std::format("Solitide | avg FPS: {:.2f} best FPS: {:.2f} worst FPS: {:.2f}",
                                     stat.get_average_FPS(), stat.get_best_FPS(), stat.get_worst_FPS());
#endif
}

void start_scene::on_key_down(input::keyboard::event& ev)
{
    if (_formMenu->get_focus_widget() != nullptr) { return; }

    using namespace tcob::enum_ops;

    if (ev.KeyCode == input::key_code::n && (ev.KeyMods & input::key_mod::LeftControl) == input::key_mod::LeftControl) {
        start_game(_formMenu->SelectedGame(), start_reason::Restart);
        ev.Handled = true;
    }
}

void start_scene::set_children_bounds(size_i size)
{
    rect_f const menuBounds {0, 0, static_cast<f32>(size.Width), static_cast<f32>(size.Height)};
    _formMenu->Bounds     = menuBounds;
    _formControls->Bounds = menuBounds;

    f32 const tableX {0};
    f32 const tableY {menuBounds.Height * 0.05f};
    f32 const tableWidth {menuBounds.Width};
    f32 const tableHeight {menuBounds.Height * 0.85f};
    _cardTable->Bounds = rect_f {{tableX, tableY}, {tableWidth, tableHeight}};
}

void start_scene::start_game(string const& name, start_reason reason)
{
    if (!_games.contains(name)) { return; }

    auto& camera {*get_window().Camera};
    camera.set_position(point_f::Zero);
    camera.set_zoom(size_f::One);

    auto* game {_cardTable->game()};
    if (reason == start_reason::Resume) {         // save current game
        update_recent(name);
        if (game) { game->save(_saveGame); }
    } else if (reason == start_reason::Restart) { // fail current game
        if (game && game->Status != game_status::Success) { game->Status = game_status::Failure; }
    }

    auto newGame {_games[name].second()};
    newGame->Status.Changed.connect([&, current = newGame.get()](auto val) {
        switch (val) {
        case game_status::Success:
        case game_status::Failure: {
            auto const& state {current->state()};
            if (state.Turns == 0) { return; } // don't store unplayed games in history

            auto const& gameName {current->info().Name};
            _db.insert_history_entry(gameName, state, current->rng(), current->Status);
            update_stats(gameName);
        } break;
        default: break;
        }
    });

    switch (reason) {
    case start_reason::Restart:
        _cardTable->start(newGame);
        break;
    case start_reason::Resume: {
        _cardTable->resume(newGame, _saveGame);
    } break;
    }

#if defined(TCOB_DEBUG)
    generate_rule(*newGame);
#endif
    locate_service<stats>().reset();
    _settings.Game = name;
}

void start_scene::start_wizard()
{
    auto wizard {std::make_shared<wizard_scene>(get_game(), _themes[_formMenu->SelectedTheme])};
    wizard->GameGenerated.connect([&](auto const& val) {
        if (_luaScript.run_file(val.Path)) {
            std::vector<game_info> games;
            games.reserve(_games.size());
            for (auto const& x : _games) { games.emplace_back(x.second.first); }

            _formMenu->update_games(games);
            _db.insert_games(games);

            start_game(val.Name, start_reason::Resume);
        }
    });
    get_game().push_scene(wizard);
}

void start_scene::update_stats(string const& name) const
{
    _formMenu->set_game_stats(_db.get_history(name));
}

void start_scene::update_recent(string const& name)
{
    std::deque<string>& recent {_settings.Recent};

    auto it {std::find(recent.begin(), recent.end(), name)};
    if (it != recent.end()) { recent.erase(it); }
    if (recent.size() >= 5) { recent.pop_back(); }
    recent.push_front(name);

    _settings.Recent       = recent;
    _formMenu->RecentGames = recent;
}

void start_scene::generate_rule(base_game const& game) const
{
    auto const& info {game.info()};

    io::create_folder("/rules/");
    auto         file {std::format("/rules/{}.html", info.Name)};
    io::ofstream fs {file};
    fs.write("<!DOCTYPE html>\n<html>\n<body>\n");

    fs.write(std::format("<h1>{}</h1>", info.Name));
    fs.write(std::format("<p>Number of decks: {}</p>", info.DeckCount));
    // fs.write(std::format("<p>Family: {}</p><br>\n", info.Family));

    fs.write("<h2>Piles</h2>");
    auto writePileType {
        [&](pile_type pt, std::vector<pile*> const& piles) {
            if (piles.empty()) { return; }

            fs.write(std::format("<h3>{} ({})</h3>", get_pile_type_name(pt), piles.size()));

            fs.write("<p>");
            pile_description last;
            for (auto const* pile : piles) {
                auto const desc {pile->get_description(game)};
                if (!desc.equal(last)) {
                    fs.write(std::format("{}: {}<br>", desc.DescriptionLabel, desc.Description));
                    if (!desc.MoveLabel.empty()) { fs.write(std::format("{}: {}<br>", desc.MoveLabel, desc.Move)); }
                    if (!desc.MoveLabel.empty()) { fs.write(std::format("{}: {}<br>", desc.BaseLabel, desc.Base)); }
                    last = desc;
                }
            }
            fs.write("</p>");
        }};

    auto const&                    piles {game.piles()};
    std::array<pile_type, 6> const pileOrder {pile_type::Stock, pile_type::Waste,
                                              pile_type::Foundation, pile_type::Tableau,
                                              pile_type::Reserve, pile_type::FreeCell};
    for (auto pt : pileOrder) {
        if (!piles.contains(pt)) { continue; }
        writePileType(pt, piles.at(pt));
    }

    fs.write("\n</body>\n</html>\n");
}

void start_scene::load_scripts()
{
    {
        lua_script_game::CreateAPI(this, _luaScript, _luaFunctions);
        auto const files {io::enumerate("/", {"games.*.lua", false}, true)};
        for (auto const& file : files) {
            std::ignore = _luaScript.run_file(file);
        }
    }

    {
        squirrel_script_game::CreateAPI(this, _sqScript, _sqFunctions);
        auto const files {io::enumerate("/", {"games.*.nut", false}, true)};
        for (auto const& file : files) {
            std::ignore = _sqScript.run_file(file);
        }
    }
}

}
