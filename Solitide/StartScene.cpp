// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "StartScene.hpp"

#include "ui/Styles.hpp"

namespace solitaire {

static char const* SAVE_NAME {"save.ini"};
static char const* SETTINGS_NAME {"settings"};

auto static get_size(std::string_view str) -> size_i
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
    , _sources {std::make_shared<menu_sources>()}
{
    _saveGame.load(SAVE_NAME);

    auto& config {locate_service<data::config_file>()};
    config.try_get(_sources->Settings, SETTINGS_NAME); // TODO: check version

    auto& resMgr {locate_service<assets::library>()};
    auto& resGrp {resMgr.create_or_get_group("solitaire")};
    resGrp.mount("./assets.zip");
    resMgr.load_all_groups();

    auto& window {get_window()};
    auto  windowSize {window.Size()};

    auto defaultCursor {resGrp.get<gfx::cursor>("default")};
    window.Cursor             = defaultCursor;
    defaultCursor->ActiveMode = "cursor32";

    load_scripts();

    // themes
    load_themes(_sources->Themes);

    // cardsets
    load_card_sets(_sources->CardSets, resGrp);

    // games
    _db.insert_games(_sources->Games);

    // ui
    _formControls = std::make_shared<form_controls>(&window, resGrp, _sources);

    _formMenu = std::make_shared<form_menu>(&window, resGrp, _sources);
    _formMenu->hide();

    // card table
    _cardTable = std::make_shared<card_table>(&window, _formControls->Canvas.get(), resGrp, &_sources->Settings);

    _cardTable->HoverChange.connect([&](pile* const pile) {
        if (!pile) {
            _formControls->set_pile_labels({});
            return;
        }

        pile_description str;
        str.Pile      = get_pile_type_name(pile->Type);
        str.CardCount = std::to_string(pile->size());

        data::config::array rules;
        auto const&         gameObj {_currentRules};
        if (!gameObj.try_get(rules, str.Pile, "rules")) { return; }

        for (auto const& rule : rules) {
            auto const ruleObj {rule.as<data::config::object>()};
            if (std::unordered_set<i32> piles; ruleObj.try_get(piles, "piles")) {
                if (!piles.contains(pile->Index)) { continue; }
            }

            switch (pile->Type) {
            case pile_type::Stock: {
                auto const& state {_cardTable->game()->state()};
                str.Description      = state.Redeals < 0 ? "∞" : std::to_string(state.Redeals);
                str.DescriptionLabel = "Redeals";
            } break;
            case pile_type::Waste:
            case pile_type::Reserve:
            case pile_type::FreeCell:
            case pile_type::Foundation:
            case pile_type::Tableau: {
                str.Description      = ruleObj["build"].as<std::string>();
                str.DescriptionLabel = "Build";
                str.Move             = ruleObj["move"].as<std::string>();
                str.MoveLabel        = "Move";
                str.Base             = ruleObj["base"].as<std::string>();
                str.BaseLabel        = "Base";
                break;
            }
            }

            _formControls->set_pile_labels(str);
            return;
        }
    });

    connect_events();
    set_children_bounds(windowSize);
    set_theme();
    set_cardset();
}

start_scene::~start_scene() = default;

void start_scene::register_game(game_info const& info, reg_game_func&& game)
{
    if (info.DeckCount > 24) { return; }                                                   // TODO: error
    if (info.DeckCount * info.DeckSuits.size() * info.DeckSuits.size() > 1500) { return; } // TODO: error
    if (_sources->Games.size() > 2500) { return; }                                         // TODO: error

    _sources->Games[info.Name] = {info, std::move(game)};
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
    // render queues
    get_root_node()->create_child()->attach_entity(_cardTable);
    get_root_node()->create_child()->attach_entity(_formControls);
    get_root_node()->create_child()->attach_entity(_formMenu);

    locate_service<stats>().reset();

    // config
    set_cardset();

    _formMenu->fixed_update(0s);     // updates style
    _formControls->fixed_update(0s); // updates style

    if (!_sources->Settings.LastGame.empty()) {
        start_game(_sources->Settings.LastGame, start_reason::Resume, std::nullopt);
    }
}

void start_scene::connect_events()
{
    _formControls->BtnNewGame->Click.connect([&](auto const&) { start_game(_sources->Settings.LastGame, start_reason::Restart, std::nullopt); });
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
        save();
        get_game().pop_current_scene();
    });
    get_game().get_window().Close.connect([&](auto const&) {
#if !defined(TCOB_DEBUG)
        save();
#endif
    });

    _sources->SelectedGame.Changed.connect([&](auto const& game) { update_game_ui(game); });
    _sources->Settings.Theme.Changed.connect([&](auto const&) { set_theme(); });
    _sources->Settings.Cardset.Changed.connect([&]() { set_cardset(); });

    _formMenu->StartGame.connect([&](auto const& seed) { start_game(_sources->SelectedGame, start_reason::Resume, seed); });

    _formMenu->VideoSettingsChanged.connect([&]() {
        data::config::object obj;
        _formMenu->submit_settings(obj);

        assert(obj.has("ddlResolution", "selected"));
        assert(obj.has("chkFullScreen", "checked"));
        assert(obj.has("chkVSync", "checked"));

        auto& window {get_window()};

        window.FullScreen = obj["chkFullScreen"]["checked"].as<bool>();
        window.VSync      = obj["chkVSync"]["checked"].as<bool>();

        auto const res {get_size(obj["ddlResolution"]["selected"].as<std::string>())};
        window.Size = res;
        set_children_bounds(res);

        start_game(_sources->Settings.LastGame, start_reason::Resume, std::nullopt);
    });
}

void start_scene::save()
{
    if (auto* game {_cardTable->game()}) {
        game->save(_saveGame);
        _saveGame.save(SAVE_NAME);
        auto& config {locate_service<data::config_file>()};
        config[SETTINGS_NAME] = _sources->Settings;
    }
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
        start_game(_sources->Settings.LastGame, start_reason::Restart, std::nullopt);
        ev.Handled = true;
    }
    if (ev.KeyCode == input::key_code::t && (ev.KeyMods & input::key_mod::LeftAlt) == input::key_mod::LeftAlt) {
        load_themes(_sources->Themes);
        set_theme();
        ev.Handled = true;
    }
    if (ev.KeyCode == input::key_code::g) {
        _sources->Translator.set_language("de-DE");
        _currentRules = generate_rule(_sources->Settings.LastGame);
        ev.Handled    = true;
    }
    if (ev.KeyCode == input::key_code::e) {
        _sources->Translator.set_language("en-US");
        _currentRules = generate_rule(_sources->Settings.LastGame);
        ev.Handled    = true;
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

void start_scene::set_theme()
{
    auto themeName {_sources->Settings.Theme()};
    if (!_sources->Themes.contains(themeName)) { themeName = "default"; }
    auto const& newTheme {_sources->Themes[themeName]};

    styles     styles {*locate_service<assets::library>().get_group("solitaire")};
    auto const styleCollection {styles.load(newTheme)};
    _formMenu->Styles = styleCollection;
    _formMenu->fixed_update(milliseconds {0});

    _formControls->Styles = styleCollection;
    _formControls->fixed_update(milliseconds {0});

    if (_wizard) {
        _wizard->update_theme(newTheme);
    }

    _cardTable->set_theme(newTheme);
    _sources->Settings.Theme = themeName;
}

void start_scene::set_cardset()
{
    auto newCardset {_sources->Settings.Cardset()};
    if (!_sources->CardSets.contains(newCardset)) { newCardset = "default"; }

    _sources->Settings.Cardset = newCardset;

    _cardTable->set_card_set(_sources->CardSets[newCardset]);
    start_game(_sources->Settings.LastGame, start_reason::Resume, std::nullopt);
}

void start_scene::start_game(std::string const& name, start_reason reason, std::optional<u64> seed)
{
    if (seed) { reason = start_reason::Restart; } // force restart if seed is set

    if (!_sources->Games.contains(name)) { return; }
    _sources->SelectedGame = name;
    update_recent(name);
    _currentRules = generate_rule(name);

    auto& camera {*get_window().Camera};
    camera.set_position(point_f::Zero);
    camera.set_zoom(size_f::One);

    auto* currentGame {_cardTable->game()};
    if (reason == start_reason::Resume) {         // save current game
        if (currentGame) { currentGame->save(_saveGame); }
    } else if (reason == start_reason::Restart) { // fail current game
        if (currentGame && currentGame->Status != game_status::Success) { currentGame->Status = game_status::Failure; }
    }

    auto newGame {_sources->Games[name].second()};
    newGame->Status.Changed.connect([&, current = newGame.get()](auto val) {
        switch (val) {
        case game_status::Success:
        case game_status::Failure: {
            auto const& state {current->state()};
            if (state.Turns == 0) { return; } // don't store unplayed games in history

            auto const& gameName {current->info().Name};
            _db.insert_history_entry(gameName, state, current->rng(), current->Status);
            update_game_ui(gameName);
        } break;
        default: break;
        }
    });

    switch (reason) {
    case start_reason::Restart:
        _cardTable->start(newGame, seed);
        break;
    case start_reason::Resume: {
        _cardTable->resume(newGame, _saveGame);
    } break;
    }

    locate_service<stats>().reset();
}

void start_scene::start_wizard()
{
    if (!_wizard) {
        _wizard = std::make_shared<wizard_scene>(get_game());
        _wizard->update_theme(_sources->Themes[_sources->Settings.Theme]);
    }

    _wizard->GameGenerated.connect([&](auto const& val) {
        if (_luaScript.run_file(val.Path)) {
            _sources->GameAdded();
            _db.insert_games(_sources->Games);
            start_game(val.Name, start_reason::Resume, std::nullopt);
        }
    });
    get_game().push_scene(_wizard);
}

void start_scene::update_game_ui(std::string const& name) const
{
    _sources->SelectedHistory = _db.get_history(name);
    _sources->SelectedRules   = generate_rule(name);
}

void start_scene::update_recent(std::string const& name)
{
    _sources->Settings.LastGame = name;

    usize constexpr static maxEntries {10};

    std::deque<std::string> recent {_sources->Settings.Recent()};

    auto it {std::find(recent.begin(), recent.end(), name)};
    if (it != recent.end()) { recent.erase(it); }
    if (recent.size() >= maxEntries) { recent.pop_back(); }
    recent.push_front(name);

    _sources->Settings.Recent = recent;
}

auto start_scene::generate_rule(std::string const& name) const -> data::config::object
{
    if (!_sources->Games.contains(name)) { return {}; }

    auto                 game {_sources->Games[name].second()};
    data::config::object gameObj;

    auto writePileType {
        [&](pile_type pt, std::vector<pile*> const& piles) {
            if (piles.empty()) { return; }

            data::config::array  pileRulesArr;
            data::config::object pileObj;
            gameObj[get_pile_type_name(pt)] = pileObj;
            pileObj["count"]                = piles.size();
            pileObj["rules"]                = pileRulesArr;

            flat_map<game_rule, std::vector<i32>> pileRules;
            for (auto const* pile : piles) {
                game_rule desc;
                desc.Base  = _sources->Translator.translate("rule", "base", pile->Rule.BaseHint);
                desc.Build = _sources->Translator.translate("rule", "build", pile->Rule.BuildHint);
                desc.Move  = _sources->Translator.translate("rule", "move", pile->Rule.MoveHint);
                pileRules[desc].push_back(pile->Index);
            }
            for (auto const& [k, v] : pileRules) {
                data::config::object rule;
                if (pileRules.size() > 1) { rule["piles"] = v; }
                rule["base"]  = k.Base;
                rule["build"] = k.Build;
                rule["move"]  = k.Move;
                // TODO: create long description
                pileRulesArr.add(rule);
            }
        }};

    auto const& piles {game->piles()};
    for (auto pt : {pile_type::Stock, pile_type::Waste, pile_type::Foundation, pile_type::Tableau, pile_type::Reserve, pile_type::FreeCell}) {
        if (!piles.contains(pt)) { continue; }
        writePileType(pt, piles.at(pt));
    }

    return gameObj;
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
