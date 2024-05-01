// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "StartScene.hpp"

namespace solitaire {

static char const* SAVE_NAME {"save.ini"};

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
    _saveGame["version"] = "1.0.0"; // TODO: check version
}

start_scene::~start_scene() = default;

void start_scene::register_game(game_info const& info, reg_game_func&& game)
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

auto start_scene::get_games() const -> std::vector<game_info>
{
    std::vector<game_info> retValue;
    retValue.reserve(_games.size());
    for (auto const& gi : _games) { retValue.emplace_back(gi.second.first); }
    return retValue;
}

auto start_scene::get_themes() const -> std::vector<std::string>
{
    std::vector<std::string> retValue;
    retValue.reserve(_themes.size());
    for (auto const& t : _themes) { retValue.push_back(t.first); }
    return retValue;
}

auto start_scene::get_cardsets() const -> std::vector<std::string>
{
    std::vector<std::string> retValue;
    retValue.reserve(_cardSets.size());
    for (auto const& cs : _cardSets) { retValue.push_back(cs.first); }
    return retValue;
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

    // games
    _db.insert_games(_games);

    // themes
    _themes = load_themes();

    // cardsets
    _cardSets = load_cardsets();

    // ui
    _formControls = std::make_shared<form_controls>(&window, resGrp);
    _formMenu     = std::make_shared<form_menu>(&window, resGrp, *this);
    _formMenu->hide();
    connect_ui_events();

    // card table
    _cardTable = std::make_shared<card_table>(&window, _formControls->Canvas.get(), resGrp);

    _cardTable->HoverChange.connect([&](pile_description const& str) {
        _formControls->LblPile->Label      = str.Pile;
        _formControls->LblCardCount->Label = str.CardCount;

        _formControls->LblDescription->Label      = str.Description;
        _formControls->LblDescriptionLabel->Label = str.DescriptionLabel;
        _formControls->LblMove->Label             = str.Move;
        _formControls->LblMoveLabel->Label        = str.MoveLabel;
        _formControls->LblBase->Label             = str.Base;
        _formControls->LblBaseLabel->Label        = str.BaseLabel;
    });

    set_children_bounds(windowSize);

    // render queues
    get_root_node()->create_child()->attach_entity(_cardTable);
    get_root_node()->create_child()->attach_entity(_formControls);
    get_root_node()->create_child()->attach_entity(_formMenu);

    locate_service<stats>().reset();

    // config
    _formMenu->SelectedTheme   = _saveGame.get<std::string>("theme").value_or("default");
    _formMenu->SelectedCardset = _saveGame.get<std::string>("cardset").value_or("default");

    _formMenu->fixed_update(0s);     // updates style
    _formControls->fixed_update(0s); // updates style

    // load config
    if (_saveGame.has("game")) {
        _formMenu->SelectedGame = _saveGame["game"].as<std::string>();
    }
}

void start_scene::connect_ui_events()
{
    _formControls->BtnNewGame->Click.connect([&](auto const&) {
        auto game {_cardTable->game()};
        if (game && game->Status != game_status::Success) { game->Status = game_status::Failure; }

        start_game(_formMenu->SelectedGame(), start_reason::Restart);
    });

    _formControls->BtnMenu->Click.connect([&](auto const&) { _formMenu->show(); });
    _formControls->BtnHint->Click.connect([&](auto const&) { _cardTable->show_next_hint(); });
    _formControls->BtnUndo->Click.connect([&](auto const&) { _cardTable->undo(); });

    _formControls->BtnQuit->Click.connect([&](auto const&) {
        if (auto game {_cardTable->game()}) {
            game->save(_saveGame);
            _saveGame.save(SAVE_NAME);
        }
        get_game().pop_current_scene();
    });

    _formMenu->SelectedGame.Changed.connect([&](auto const& game) {
        auto& camera {*get_window().Camera};
        camera.set_position(point_f::Zero);
        camera.set_zoom(size_f::One);

        start_game(game, start_reason::Resume);
    });

    _formMenu->SelectedTheme.Changed.connect([&](auto const& theme) {
        auto& resMgr {locate_service<assets::library>()};
        auto& resGrp {resMgr.create_or_get_group("solitaire")};

        auto themeName {theme};
        if (!_themes.contains(theme)) { themeName = "default"; }
        auto const& newTheme {_themes[themeName]};

        create_styles(newTheme, resGrp, *_formMenu->Styles);
        create_styles(newTheme, resGrp, *_formControls->Styles);

        _formMenu->force_redraw("");
        _formControls->force_redraw("");
        _cardTable->set_theme(newTheme);
        _saveGame["theme"] = themeName;
    });

    _formMenu->SelectedCardset.Changed.connect([&](auto const& cardset) {
        auto newCardset {cardset};
        if (!_cardSets.contains(cardset)) { newCardset = "default"; }

        _saveGame["cardset"] = newCardset;

        _cardTable->set_cardset(_cardSets[newCardset]);
        start_game(_formMenu->SelectedGame(), start_reason::Resume);
    });

    _formMenu->VisibilityChanged.connect([&](bool val) {
        if (val) {
            _formControls->hide();
        } else {
            _formControls->show();
        }
    });

    _formMenu->BtnApplySettings->Click.connect([&]() {
        data::config::object obj;
        _formMenu->submit_settings(obj);

        assert(obj.has("ddlResolution", "selected"));
        assert(obj.has("chkFullScreen", "checked"));
        assert(obj.has("chkVSync", "checked"));

        auto& window {get_window()};

        auto const res {get_size(obj["ddlResolution"]["selected"].as<string>())};
        window.Size = res;
        set_children_bounds(res);

        window.FullScreen = obj["chkFullScreen"]["checked"].as<bool>();
        window.VSync      = obj["chkVSync"]["checked"].as<bool>();

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
    if (auto game {_cardTable->game()}) {
        game->update(deltaTime);

        auto const& info {game->info()};
        _formControls->LblGameName->Label = info.Name;
        auto const& state {game->state()};
        _formControls->LblTurns->Label = std::to_string(state.Turns);
        _formControls->LblScore->Label = std::to_string(state.Score);
        _formControls->LblTime->Label  = std::format("{:%M:%S}", seconds {state.Time.count() / 1000});
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

    switch (ev.ScanCode) {
    case input::scan_code::BACKSPACE:
        get_game().pop_current_scene();
        break;
    default:
        break;
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
    if (reason == start_reason::Resume) {
        if (auto game {_cardTable->game()}) {
            game->save(_saveGame);
        }
    }

    if (!_games.contains(name)) { return; }

    auto newGame {_games[name].second()};
    newGame->Status.Changed.connect([&](auto val) {
        switch (val) {
        case game_status::Success:
        case game_status::Failure: {
            auto const  current {_cardTable->game()};
            auto const& state {current->state()};
            if (state.Turns == 0) { return; } // don't save unplayed games

            _db.insert_history_entry(current->info().Name, state, current->Status);

            update_stats(name);
        } break;
        default: break;
        }
    });

    switch (reason) {
    case start_reason::Restart:
        generate_rule(*newGame);
        _cardTable->start(newGame);
        break;
    case start_reason::Resume:
        _cardTable->resume(newGame, _saveGame);
        break;
    }

    locate_service<stats>().reset();
    _saveGame["game"] = name;
    update_stats(name);
}

void start_scene::update_stats(string const& name) const
{
    _formMenu->set_game_stats(_db.get_history(name));
}

void start_scene::generate_rule(base_game const& game) const
{
    // generate rules     // TODO: translate
    io::create_folder("/rules/");
    auto         file {std::format("/rules/rule-{}.txt", game.info().Name)};
    //   if (io::exists(file)) { return; }
    io::ofstream fs {file};

    auto writePileType {
        [&](pile_type pt, std::vector<pile*> const& piles) {
            if (piles.empty()) { return; }

            fs.write(std::format("{} ({})\n", get_pile_type_name(pt), piles.size()));
            pile_description last;
            for (auto const* pile : piles) {
                auto const desc {pile->get_description(game)};
                if (!desc.equal(last)) {
                    fs.write(std::format("{}: {}\n", desc.DescriptionLabel, desc.Description));
                    if (!desc.MoveLabel.empty()) { fs.write(std::format("{}: {}\n", desc.MoveLabel, desc.Move)); }
                    if (!desc.MoveLabel.empty()) { fs.write(std::format("{}: {}\n", desc.BaseLabel, desc.Base)); }
                    last = desc;
                    fs.write("\n");
                }
            }
        }};

    auto const&                    piles {game.piles()};
    std::array<pile_type, 6> const pileOrder {pile_type::Foundation, pile_type::Tableau, pile_type::Stock,
                                              pile_type::Waste, pile_type::Reserve, pile_type::FreeCell};
    for (auto pt : pileOrder) {
        if (!piles.contains(pt)) { continue; }
        writePileType(pt, piles.at(pt));
    }
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
