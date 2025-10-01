// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "MainScene.hpp"

#include <algorithm>
#include <utility>

#include "ui/Styles.hpp"

namespace solitaire {

////////////////////////////////////////////////////////////

static char const* SAVE_NAME {"save.ini"};
static char const* SETTINGS_NAME {"settings"};

main_scene::main_scene(game& game)
    : scene {game}
    , _sources {std::make_shared<sources>()}
{
    _saveGame.load(SAVE_NAME);

    auto const& config {locate_service<platform>().config()};
    config.try_get(_sources->Settings, SETTINGS_NAME); // TODO: check version

    auto& resMgr {library()};
    auto& resGrp {*resMgr.get_group("solitaire")};

    auto& win {window()};
    auto  windowSize {*win.Size};

    auto defaultCursor {resGrp.get<gfx::cursor>("default")};
    win.Cursor                = defaultCursor;
    defaultCursor->ActiveMode = "cursor32";

    _scriptHost = std::make_shared<script_host>(_sources);

    // themes
    load_themes(_sources->Themes);

    // cardsets
    load_card_sets(_sources->CardSets, resGrp);

    // games
    _db.insert_games(_sources->Games);

    // ui
    _formControls = std::make_shared<form_controls>(win, resGrp, _sources);

    _formMenu = std::make_shared<form_menu>(win, resGrp, _sources);
    _formMenu->hide();

    // card table
    _cardTable = std::make_shared<card_table>(win.camera(), resGrp, &_sources->Settings);

    _cardTable->HoverChange.connect([&](pile* const pile) {
        _formControls->set_pile_labels(pile, _currentRules, _cardTable->game()->state());
    });

    connect_events();
    set_children_bounds(windowSize);
    apply_theme();
    apply_cardset();
}

main_scene::~main_scene() = default;

void main_scene::on_start()
{
    // render queues
    root_node().create_child().Entity = _cardTable;
    root_node().create_child().Entity = _formControls;
    root_node().create_child().Entity = _formMenu;

    locate_service<gfx::render_system>().statistics().reset();

    // config
    apply_cardset();

    _formMenu->update(0s);     // updates style
    _formControls->update(0s); // updates style

    if (!_sources->Settings.LastGame.empty()) {
        start_game(_sources->Settings.LastGame, start_reason::Resume, std::nullopt);
    }
}

void main_scene::connect_events()
{
    _sources->Events.RestartGame.connect([&]() { start_game(_sources->Settings.LastGame, start_reason::Restart, std::nullopt); });
    _sources->Events.ShowWizard.connect([&]() { start_wizard(); });
    _sources->Events.ShowMenu.connect([&]() { _formMenu->show(); });

    _sources->Events.ShowHint.connect([&]() { _cardTable->show_next_hint(); });
    _sources->Events.Collect.connect([&]() {
        if (auto* game {_cardTable->game()}) { game->collect_all(); }
    });
    _sources->Events.Undo.connect([&]() {
        if (auto* game {_cardTable->game()}) { game->undo(); }
    });

    _sources->Events.Quit.connect([&]() {
        save();
        parent().pop_current_scene();
    });
    window().Close.connect([&](auto const&) {
#if !defined(TCOB_DEBUG)
        save();
#endif
    });

    _sources->SelectedGame.Changed.connect([&](auto const& game) { update_selected(game); });
    _sources->Settings.Theme.Changed.connect([&](auto const&) { apply_theme(); });
    _sources->Settings.Cardset.Changed.connect([&] { apply_cardset(); });

    _sources->Events.StartGame.connect([&](auto const& seed) { start_game(_sources->SelectedGame, start_reason::Resume, seed); });

    _sources->Events.VideoSettingsChanged.connect([&](video_settings const& v) {
        auto& win {window()};
        win.FullScreen = v.FullScreen;
        win.VSync      = v.VSync;
        win.Size       = v.Resolution;

        set_children_bounds(win.Size);
        start_game(_sources->Settings.LastGame, start_reason::Resume, std::nullopt);
    });
}

void main_scene::save()
{
    if (auto* game {_cardTable->game()}) {
        game->save(_saveGame);
        _saveGame.save(SAVE_NAME);
    }

    auto& config {locate_service<platform>().config()};
    config[SETTINGS_NAME] = _sources->Settings;
}

void main_scene::on_draw_to(gfx::render_target&)
{
}

void main_scene::on_update(milliseconds)
{
}

void main_scene::on_fixed_update(milliseconds deltaTime)
{
    if (auto* game {_cardTable->game()}) {
        game->update(deltaTime);
        _formControls->set_game_labels(game);
    }
#if defined(TCOB_DEBUG)
    auto const& stats {locate_service<gfx::render_system>().statistics()};
    window().Title = std::format("Solitide | avg FPS: {:.2f} best FPS: {:.2f} worst FPS: {:.2f}",
                                 stats.average_FPS(), stats.best_FPS(), stats.worst_FPS());
#endif
}

void main_scene::on_key_down(input::keyboard::event const& ev)
{
    if (_formMenu->focused_widget() != nullptr) { return; }

    using namespace tcob::enum_ops;

    if (ev.KeyCode == input::key_code::n && ev.KeyMods.control()) {
        start_game(_sources->Settings.LastGame, start_reason::Restart, std::nullopt);
        ev.Handled = true;
    }
    if (ev.KeyCode == input::key_code::t && ev.KeyMods.alt()) {
        load_themes(_sources->Themes);
        apply_theme();
        ev.Handled = true;
    }
#if defined(TCOB_DEBUG)
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
    if (ev.KeyCode == input::key_code::s && ev.KeyMods.alt()) {
        static i32 i {0};
        std::ignore = window().copy_to_image().save(std::format("img{}.png", i++));
        ev.Handled  = true;
    }
#endif
}

void main_scene::set_children_bounds(size_i size)
{
    rect_f const menuBounds {0, 0, static_cast<f32>(size.Width), static_cast<f32>(size.Height)};
    _formMenu->Bounds     = menuBounds;
    _formControls->Bounds = menuBounds;

    f32 const tableX {0};
    f32 const tableY {menuBounds.height() * 0.05f};
    f32 const tableWidth {menuBounds.width()};
    f32 const tableHeight {menuBounds.height() * 0.85f};
    _cardTable->Bounds = rect_f {{tableX, tableY}, {tableWidth, tableHeight}};
}

void main_scene::apply_theme()
{
    auto themeName {*_sources->Settings.Theme};
    if (!_sources->Themes.contains(themeName)) { themeName = "default"; }
    auto const& newTheme {_sources->Themes[themeName]};

    styles     styles {*library().get_group("solitaire")};
    auto const styleCollection {styles.load(newTheme)};
    _formMenu->Styles = styleCollection;
    _formMenu->update(milliseconds {0});

    _formControls->Styles = styleCollection;
    _formControls->update(milliseconds {0});

    if (_wizard) {
        _wizard->update_theme(newTheme);
    }

    _cardTable->set_theme(newTheme);
    _sources->Settings.Theme = themeName;
}

void main_scene::apply_cardset()
{
    auto newCardset {*_sources->Settings.Cardset};
    if (!_sources->CardSets.contains(newCardset)) { newCardset = "default"; }

    _sources->Settings.Cardset = newCardset;

    _cardTable->set_cardset(*_sources->CardSets[newCardset]);
    start_game(_sources->Settings.LastGame, start_reason::Resume, std::nullopt);
}

void main_scene::start_game(std::string const& name, start_reason reason, std::optional<u64> seed)
{
    if (seed) { reason = start_reason::Restart; } // force restart if seed is set

    if (!_sources->Games.contains(name)) { return; }
    _sources->SelectedGame = name;
    update_recent(name);
    _currentRules = generate_rule(name);

    auto& camera {window().camera()};
    camera.Position = point_f::Zero;
    camera.Zoom     = size_f::One;

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
            update_selected(gameName);
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
}

void main_scene::start_wizard()
{
    if (!_wizard) {
        _wizard = std::make_shared<wizard_scene>(parent());
        _wizard->update_theme(_sources->Themes[_sources->Settings.Theme]);
    }

    _wizard->GameGenerated.connect([&](auto const& val) {
        if (_scriptHost->run_file(val.Path)) {
            _sources->Events.GameAdded();
            _db.insert_games(_sources->Games);
            start_game(val.Name, start_reason::Resume, std::nullopt);
        }
    });
    parent().push_scene(_wizard);
}

void main_scene::update_selected(std::string const& name) const
{
    _sources->SelectedHistory = _db.get_history(name);
    _sources->SelectedRules   = generate_rule(name);
}

void main_scene::update_recent(std::string const& name)
{
    _sources->Settings.LastGame = name;

    usize static constexpr maxEntries {10};

    std::deque<std::string> recent {*_sources->Settings.Recent};

    auto it {std::ranges::find(recent, name)};
    if (it != recent.end()) { recent.erase(it); }
    if (recent.size() >= maxEntries) { recent.pop_back(); }
    recent.push_front(name);

    _sources->Settings.Recent = recent;
}

auto main_scene::generate_rule(std::string const& name) const -> data::object
{
    if (!_sources->Games.contains(name)) { return {}; }

    auto         game {_sources->Games[name].second()};
    data::object gameObj;

    auto writePileType {[&](pile_type pt, std::vector<pile*> const& piles) {
        if (piles.empty()) { return; }

        data::array  pileRulesArr;
        data::object pileObj;
        gameObj[get_pile_type_name(pt)] = pileObj;
        pileObj["count"]                = piles.size();
        pileObj["rules"]                = pileRulesArr;

        std::unordered_map<game_rule, std::vector<i32>> pileRules;
        for (auto const* pile : piles) {
            game_rule desc;
            desc.Base  = _sources->Translator.translate("rule", "base", pile->Rule.BaseHint);
            desc.Build = _sources->Translator.translate("rule", "build", pile->Rule.BuildHint);
            desc.Move  = _sources->Translator.translate("rule", "move", pile->Rule.MoveHint);
            pileRules[desc].push_back(pile->Index);
        }
        for (auto const& [k, v] : pileRules) {
            data::object rule;
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

}
