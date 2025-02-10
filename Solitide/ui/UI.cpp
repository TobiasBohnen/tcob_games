// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "UI.hpp"

#include <utility>

#include "CardSet.hpp" // IWYU pragma: keep
#include "GameInfo.hpp"
#include "Themes.hpp"  // IWYU pragma: keep

namespace solitaire {

using namespace tcob::literals;

auto static make_tooltip(menu_sources& sources, form* form) -> std::shared_ptr<tooltip>
{
    auto retValue {form->create_tooltip<tooltip>("tooltip")};

    auto tooltipLayout {retValue->create_layout<dock_layout>()};
    auto tooltipLabel {tooltipLayout->create_widget<label>(dock_style::Fill, "TTLabel0")};
    tooltipLabel->Class = "tooltip-label";

    retValue->Popup.connect([&, lbl = tooltipLabel.get(), tt = retValue.get()](auto const& event) {
        auto const widget {event.Widget};
        sources.Translator.bind(lbl->Label, "ux", widget->name());

        auto const  bounds {widget->Bounds()};
        auto const& style {lbl->current_style<label::style>()->Text};
        auto* const font {style.Font->get_font(style.Style, style.calc_font_size({0, 0, bounds.width() * 1.5f, bounds.height() * 0.75f})).ptr()};
        tt->Bounds = {point_f::Zero, gfx::text_formatter::measure(lbl->Label(), *font, -1, true)};
    });
    return retValue;
}

form_controls::form_controls(gfx::window* window, assets::group& resGrp, std::shared_ptr<menu_sources> sources)
    : form {"Controls", window}
    , _sources {std::move(sources)}
{
    (*Controls).ActivateKey = input::key_code::UNKNOWN;

    auto tooltip0 {make_tooltip(*_sources, this)};

    auto mainPanel {create_container<glass>(dock_style::Fill, "main")};
    auto mainPanelLayout {mainPanel->create_layout<dock_layout>()};

    // menu
    {
        auto menuPanel {mainPanelLayout->create_widget<panel>(dock_style::Top, "menu")};
        menuPanel->Flex = {100_pct, 5_pct};
        auto menuPanelLayout {menuPanel->create_layout<grid_layout>(size_i {20, 1})};

        auto const create {[&](rect_i const& bounds, std::string const& name, std::string const& tex) {
            auto retValue {menuPanelLayout->create_widget<button>(bounds, name)};
            retValue->Icon    = {resGrp.get<gfx::texture>(tex)};
            retValue->Tooltip = tooltip0;
            return retValue;
        }};

        BtnMenu    = create({0, 0, 1, 1}, "btnMenu", "burger");
        BtnWizard  = create({2, 0, 1, 1}, "btnWizard", "wand");
        BtnNewGame = create({12, 0, 1, 1}, "btnNewGame", "newgame");
        BtnHint    = create({14, 0, 1, 1}, "btnHint", "hint");
        BtnCollect = create({15, 0, 1, 1}, "btnCollect", "collect");
        BtnUndo    = create({16, 0, 1, 1}, "btnUndo", "undo");
        BtnQuit    = create({19, 0, 1, 1}, "btnQuit", "exit");
    }

    // status
    {
        auto statusPanel {mainPanelLayout->create_widget<panel>(dock_style::Bottom, "status")};
        statusPanel->Flex = {100_pct, 10_pct};
        auto statusPanelLayout {statusPanel->create_layout<grid_layout>(size_i {40, 6})};

        i32        lbID {0};
        auto const create {[&](rect_i const& rect, std::string const& id = "") {
            auto l {statusPanelLayout->create_widget<label>(rect, "lblStatus" + std::to_string(lbID++))};
            l->Class = "label-margin";
            if (!id.empty()) {
                _sources->Translator.bind(l->Label, "status", id);
            }
            return l;
        }};

        _lblGameName = create({0, 0, 4, 6});

        _lblPile           = create({4, 3, 4, 3});
        _lblPileLabel      = create({4, 0, 4, 3}, "Pile");
        _lblCardCount      = create({8, 3, 2, 3});
        _lblCardCountLabel = create({8, 0, 2, 3}, "Cards");

        _lblBase             = create({11, 3, 6, 3});
        _lblBaseLabel        = create({11, 0, 6, 3});
        _lblDescription      = create({17, 3, 10, 3});
        _lblDescriptionLabel = create({17, 0, 10, 3});
        _lblMove             = create({27, 3, 6, 3});
        _lblMoveLabel        = create({27, 0, 6, 3});

        _lblTurns      = create({34, 3, 2, 3});
        _lblTurnsLabel = create({34, 0, 2, 3}, "Turns");

        _lblScore      = create({36, 3, 2, 3});
        _lblScoreLabel = create({36, 0, 2, 3}, "Score");

        _lblTime      = create({38, 3, 2, 3});
        _lblTimeLabel = create({38, 0, 2, 3}, "Time");
    }
}

void form_controls::set_pile_labels(pile_description const& str)
{
    _lblPile->Label      = str.Pile;
    _lblCardCount->Label = str.CardCount;

    _lblDescription->Label = str.Description;
    if (!str.DescriptionLabel.empty()) { _sources->Translator.bind(_lblDescriptionLabel->Label, "status", str.DescriptionLabel); }

    _lblMove->Label = str.Move;
    if (!str.MoveLabel.empty()) { _sources->Translator.bind(_lblMoveLabel->Label, "status", str.MoveLabel); }

    _lblBase->Label = str.Base;
    if (!str.BaseLabel.empty()) { _sources->Translator.bind(_lblBaseLabel->Label, "status", str.BaseLabel); }
}

void form_controls::set_game_labels(base_game* game)
{
    auto const& info {game->info()};
    _lblGameName->Label = info.Name;
    auto const& state {game->state()};
    _lblTurns->Label = std::to_string(state.Turns);
    _lblScore->Label = std::to_string(state.Score);
    _lblTime->Label  = std::format("{:%M:%S}", seconds {state.Time.count() / 1000});
}

////////////////////////////////////////////////////////////

static std::string const TabSettingsName {"conSettings"};
static std::string const MenuName {"menu"};

form_menu::form_menu(gfx::window* window, assets::group& resGrp, std::shared_ptr<menu_sources> sources)
    : form {"Menu", window}
    , _resGrp {resGrp}
    , _sources {std::move(sources)}
{
    _tooltip = make_tooltip(*_sources, this);

    auto tabContainer {create_container<tab_container>(dock_style::Right, "tabMenu")};
    tabContainer->Class = "tab_container_hidden";
    tabContainer->Flex  = {85_pct, 100_pct};

    create_section_games(*tabContainer);
    create_section_settings(*tabContainer);
    create_section_themes(*tabContainer);
    create_section_cardset(*tabContainer);
    create_menubar(*tabContainer);
}

void form_menu::submit_settings(data::config::object& obj)
{
    dynamic_cast<tab_container*>(find_widget_by_name(TabSettingsName).get())->submit(obj);
}

void form_menu::on_key_down(input::keyboard::event const& ev)
{
    if (ev.KeyCode == input::key_code::ESCAPE) {
        hide();
        ev.Handled = true;
        return;
    }
    form::on_key_down(ev);
}

void form_menu::create_section_games(tab_container& parent)
{
    auto panelGames {parent.create_tab<panel>("tabGames")};
    auto panelLayout {panelGames->create_layout<dock_layout>()};

    create_game_lists(*panelLayout);
    create_game_details(*panelLayout);
}

void form_menu::create_game_lists(dock_layout& panelLayout)
{
    auto panelFilter {panelLayout.create_widget<panel>(dock_style::Top, "panelFilter")};
    panelFilter->Flex = {100_pct, 5_pct};

    auto panelFilterLayout {panelFilter->create_layout<grid_layout>(size_i {10, 1})};
    auto txbFilter {panelFilterLayout->create_widget<text_box>({0, 0, 9, 1}, "txbFilter")};
    txbFilter->MaxLength = 30;
    auto btnClearFilter {panelFilterLayout->create_widget<button>({9, 0, 1, 1}, "btnClearFilter")};
    btnClearFilter->Icon = {_resGrp.get<gfx::texture>("cross")};
    btnClearFilter->Click.connect([tb = txbFilter.get()]() { tb->Text = ""; });
    btnClearFilter->Tooltip = _tooltip;

    std::vector<list_box*> listBoxes;
    auto                   tabGames {panelLayout.create_widget<tab_container>(dock_style::Left, "tabGames")};
    tabGames->MaxTabsPerRow = 6;
    tabGames->Flex          = {50_pct, 100_pct};

    i32        lbID {0};
    auto const createListBox {[&](std::shared_ptr<dock_layout>& tabPanelLayout, auto&& pred) -> std::shared_ptr<list_box> {
        auto listBox {tabPanelLayout->create_widget<list_box>(dock_style::Fill, "lbxGames" + std::to_string(lbID++))};
        listBoxes.push_back(listBox.get());

        for (auto const& game : _sources->Games) {
            if (pred(game.second.first)) { listBox->add_item(game.first); }
        }

        listBox->SelectedItemIndex.Changed.connect([&, lb = listBox.get()](auto val) {
            if (val != -1) { _sources->SelectedGame = lb->selected_item().Text; }
        });
        _sources->SelectedGame.Changed.connect([&, lb = listBox.get()](auto const& val) {
            if (lb->select_item(val)) {
                if (!lb->is_focused()) { lb->scroll_to_selected(); }
            } else {
                lb->SelectedItemIndex = -1;
            }
        });
        listBox->DoubleClick.connect([&, lb = listBox.get()] {
            if (lb->SelectedItemIndex >= 0) { start_game(); }
        });
        return listBox;
    }};

    // By Name
    {
        auto tabPanel {tabGames->create_tab<panel>("tabByName")};
        _sources->Translator.bind_tab(tabGames.get(), tabPanel.get());

        auto tabPanelLayout {tabPanel->create_layout<dock_layout>()};
        auto listBox {createListBox(tabPanelLayout, [](auto const&) { return true; })};
        _sources->GameAdded.connect([&, lb = listBox.get()] {
            // TODO: update all listboxes
            lb->clear_items();
            for (auto const& game : _sources->Games) { lb->add_item(game.first); }
        });
    }
    // Recent
    {
        auto tabPanel {tabGames->create_tab<panel>("tabRecent")};
        _sources->Translator.bind_tab(tabGames.get(), tabPanel.get());

        auto tabPanelLayout {tabPanel->create_layout<dock_layout>()};
        auto listBox {createListBox(tabPanelLayout, [](auto const&) { return false; })};
        for (auto const& game : _sources->Settings.Recent()) { listBox->add_item(game); }
        _sources->Settings.Recent.Changed.connect([&, lb = listBox.get()] {
            lb->clear_items();
            for (auto const& game : _sources->Settings.Recent()) { lb->add_item(game); }
        });
    }
    // By Family
    {
        auto acc {tabGames->create_tab<accordion>("tabByFamily")};
        acc->Class                 = "accordion2";
        acc->MaximizeActiveSection = true;
        _sources->Translator.bind_tab(tabGames.get(), acc.get());

        auto const createSection {[&](family family) {
            auto secPanel {acc->create_section<panel>("")};
            auto secPanelLayout {secPanel->create_layout<dock_layout>()};
            _sources->Translator.bind(
                [tabC = acc.get(), tabP = secPanel.get()](std::string const& val) { tabC->change_section_label(tabP, val); },
                "family", family);
            createListBox(secPanelLayout, [family](auto const& gameInfo) { return gameInfo.Family.contains(family); });
        }};
        createSection(family::BakersDozen);
        createSection(family::BeleagueredCastle);
        createSection(family::Canfield);
        createSection(family::Fan);
        createSection(family::FlowerGarden);
        createSection(family::FortyThieves);
        createSection(family::FreeCell);
        createSection(family::Golf);
        createSection(family::Gypsy);
        createSection(family::Klondike);
        createSection(family::Montana);
        createSection(family::Numerica);
        createSection(family::Pairing);
        createSection(family::PictureGallery);
        createSection(family::Spider);
        createSection(family::Terrace);
        createSection(family::Yukon);
        createSection(family::Other);
    }
    // By Deck Count
    {
        auto tabContainer {tabGames->create_tab<tab_container>("tabByDeckCount")};
        _sources->Translator.bind_tab(tabGames.get(), tabContainer.get());

        auto const createTab {[&](isize count, std::string const& name) {
            auto tabPanel {tabContainer->create_tab<panel>(name)};
            auto tabPanelLayout {tabPanel->create_layout<dock_layout>()};
            createListBox(tabPanelLayout, [count](auto const& gameInfo) { return gameInfo.DeckCount == count; });
        }};
        createTab(1, "1");
        createTab(2, "2");
        createTab(3, "3");

        {
            auto tabPanel {tabContainer->create_tab<panel>(">= 4")};
            auto tabPanelLayout {tabPanel->create_layout<dock_layout>()};
            createListBox(tabPanelLayout, [](auto const& gameInfo) { return gameInfo.DeckCount >= 4; });
        }
        {
            auto tabPanel {tabContainer->create_tab<panel>("tabStripped")};
            _sources->Translator.bind_tab(tabContainer.get(), tabPanel.get());

            auto tabPanelLayout {tabPanel->create_layout<dock_layout>()};
            createListBox(tabPanelLayout, [](auto const& gameInfo) { return gameInfo.DeckRanks.size() < 13 || gameInfo.DeckSuits.size() < 4; });
        }
    }

    txbFilter->Text.Changed.connect([listBoxes](auto const& val) {
        for (auto* lb : listBoxes) {
            lb->Filter = val;
        }
    });
}

void form_menu::create_game_details(dock_layout& panelLayout)
{
    auto panelGameDetails {panelLayout.create_widget<panel>(dock_style::Right, "panelGameDetails")};
    panelGameDetails->Flex   = {50_pct, 100_pct};
    panelGameDetails->ZOrder = 1;
    auto panelGameStatsLayout {panelGameDetails->create_layout<grid_layout>(size_i {20, 40})};

    auto lblSeed {panelGameStatsLayout->create_widget<label>({1, 33, 4, 2}, "lblSeed")};
    _sources->Translator.bind(lblSeed->Label, "ux", lblSeed->name());
    lblSeed->Class = "label-margin";
    _txbSeed       = panelGameStatsLayout->create_widget<text_box>({6, 33, 8, 2}, "txbSeed");
    _txbSeed->BeforeTextInserted.connect([](text_event& ev) {
        if (ev.Text.empty()) { return; }
        if (ev.Text.size() != 1 || !std::isdigit(ev.Text[0])) {
            ev.Text = "";
        }
    });

    auto btnStartGame {panelGameStatsLayout->create_widget<button>({1, 36, 4, 3}, "btnStartGame")};
    btnStartGame->Icon = {_resGrp.get<gfx::texture>("play")};
    btnStartGame->Click.connect([&]() { start_game(); });
    btnStartGame->Tooltip = _tooltip;

    auto tabGameDetails {panelGameStatsLayout->create_widget<tab_container>({0, 0, 20, 32}, "tabGameDetails")};
    tabGameDetails->MaxTabsPerRow = 6;

    // info tab
    {
        auto tabPanel {tabGameDetails->create_tab<panel>("tabInfo")};
        _sources->Translator.bind_tab(tabGameDetails.get(), tabPanel.get());

        auto tabPanelLayout {tabPanel->create_layout<grid_layout>(size_i {40, 30})};

        auto gvInfo {tabPanelLayout->create_widget<grid_view>({2, 1, 36, 5}, "gvInfo")};
        gvInfo->Class = "grid_view2";
        auto addRow {
            [&, infoGV = gvInfo.get()](auto const& game) {
                infoGV->clear_rows();
                if (!_sources->Games.contains(game)) { return; }
                auto gameInfo {_sources->Games[game].first};
                infoGV->add_row(
                    {_sources->Translator.translate("family", gameInfo.Family),
                     std::to_string(gameInfo.DeckCount),
                     gameInfo.Redeals < 0 ? "∞" : std::to_string(gameInfo.Redeals)});
            }};
        _sources->Translator.bind(
            [&, addRow, infoGV = gvInfo.get()](std::vector<std::string> const& val) {
                infoGV->set_columns(val, false);
                addRow(_sources->SelectedGame);
            },
            "columns", "info");

        _sources->SelectedGame.Changed.connect(addRow);

        auto accRules {tabPanelLayout->create_widget<accordion>({1, 7, 38, 22}, "gvRules")};

        _sources->SelectedRules.Changed.connect([rulesAcc = accRules.get()](data::config::object const& piles) {
            rulesAcc->clear_sections();

            for (auto const& pile : piles) {
                auto const& name {pile.first};
                auto        pilePanel {rulesAcc->create_section<panel>(name)};
                auto const& pileObj {pile.second.as<data::config::object>()};
                //    auto const  count {pileObj["count"].as<std::string>()};
                auto const& rulesArr {pileObj["rules"].as<data::config::array>()};

                auto createRule {[&](grid_layout& layout, data::config::object const& rule) {
                    auto const create {[&](rect_i const& rect, std::string const& text) {
                        auto l {layout.create_widget<label>(rect, "")};
                        l->Class = "label-margin";
                        l->Label = text;
                        return l;
                    }};

                    create({1, 1, 10, 5}, "Base");
                    create({12, 1, 17, 5}, rule["base"].as<std::string>());
                    create({1, 7, 10, 5}, "Build");
                    create({12, 7, 17, 5}, rule["build"].as<std::string>());
                    create({1, 13, 10, 5}, "Move");
                    create({12, 13, 17, 5}, rule["move"].as<std::string>());
                }};

                if (rulesArr.size() == 1) {
                    auto pilePanelLayout {pilePanel->create_layout<grid_layout>(size_i {30, 25})};
                    createRule(*pilePanelLayout, rulesArr[0].as<data::config::object>());
                } else {
                    auto pilePanelLayout {pilePanel->create_layout<dock_layout>()};
                    auto tabRules {pilePanelLayout->create_widget<tab_container>(dock_style::Fill, "")};
                    tabRules->Class         = "tab_container_small";
                    tabRules->MaxTabsPerRow = 5;
                    for (auto const& rule : rulesArr) {
                        auto const& ruleObj {rule.as<data::config::object>()};
                        auto        panelRule {tabRules->create_tab<panel>(set_to_string(ruleObj["piles"].as<std::set<i32>>()))};
                        auto        panelRuleLayout {panelRule->create_layout<grid_layout>(size_i {30, 25})};
                        createRule(*panelRuleLayout, ruleObj);
                    }
                }
            }
        });
    }
    // stats tab
    {
        auto tabPanel {tabGameDetails->create_tab<panel>("tabStats")};
        _sources->Translator.bind_tab(tabGameDetails.get(), tabPanel.get());

        auto tabPanelLayout {tabPanel->create_layout<grid_layout>(size_i {40, 30})};

        auto gvWL {tabPanelLayout->create_widget<grid_view>({1, 1, 18, 5}, "gvWinLose")};
        gvWL->Class = "grid_view2";
        _sources->Translator.bind_grid_header(gvWL.get(), "wl");

        auto gvScore {tabPanelLayout->create_widget<grid_view>({21, 1, 18, 5}, "gvBest")};
        gvScore->Class = "grid_view2";
        _sources->Translator.bind_grid_header(gvScore.get(), "score");

        auto gvHistory {tabPanelLayout->create_widget<grid_view>({1, 7, 38, 22}, "gvHistory")};
        _sources->Translator.bind_grid_header(gvHistory.get(), "history");

        _sources->SelectedHistory.Changed.connect([wl = gvWL.get(), tt = gvScore.get(), history = gvHistory.get()](auto const& stats) {
            wl->clear_rows();
            wl->add_row({std::to_string(stats.Won), std::to_string(stats.Lost),
                         stats.Lost + stats.Won > 0 ? std::format("{:.2f}%", static_cast<f32>(stats.Won) / (stats.Lost + stats.Won) * 100) : "-"});
            tt->clear_rows();

            std::optional<i64> bestTime;
            std::optional<i64> bestTurns;
            std::optional<i64> bestScore;

            history->clear_rows();
            for (auto const& entry : stats.Entries) {
                if (entry.Won) {
                    bestTime  = !bestTime ? entry.Time : std::min(entry.Time, *bestTime);
                    bestTurns = !bestTurns ? entry.Turns : std::min(entry.Turns, *bestTurns);
                }
                bestScore = !bestScore ? entry.Score : std::max(entry.Score, *bestScore);

                history->add_row(
                    {std::to_string(entry.Seed),
                     std::to_string(entry.Score),
                     std::to_string(entry.Turns),
                     std::format("{:%M:%S}", seconds {entry.Time / 1000.f}),
                     entry.Won ? "X" : "-"});
            }
            tt->add_row({bestScore ? std::to_string(*bestScore) : "-",
                         bestTurns ? std::to_string(*bestTurns) : "-",
                         bestTime ? std::format("{:%M:%S}", seconds {*bestTime / 1000.f}) : "--:--"});
        });
    }
}

void form_menu::create_section_settings(tab_container& parent)
{
    // Setting
    auto tabSettings {parent.create_tab<tab_container>(TabSettingsName)};

    create_settings_video(*tabSettings);
    create_settings_hints(*tabSettings);
}

void form_menu::create_settings_video(tab_container& tabContainer)
{
    auto const& config {locate_service<platform>().config()};
    auto        tabPanel {tabContainer.create_tab<panel>("tabVideo")};
    _sources->Translator.bind_tab(&tabContainer, tabPanel.get());

    auto tabPanelLayout {tabPanel->create_layout<grid_layout>(size_i {40, 40})};

    // resolution
    {
        auto const displayModes {locate_service<gfx::render_system>().displays()};
        auto       ddlRes {tabPanelLayout->create_widget<drop_down_list>({6, 1, 6, 4}, "ddlResolution")};
        ddlRes->ZOrder = 1;
        for (auto const& dm : displayModes.at(0).Modes) {
            ddlRes->add_item(std::format("{}x{}", dm.Size.Width, dm.Size.Height));
        }

        auto const res {config[Cfg::Video::Name][Cfg::Video::resolution].as<size_i>()};
        ddlRes->select_item(std::format("{}x{}", res.Width, res.Height));

        auto lbl {tabPanelLayout->create_widget<label>({1, 2, 4, 2}, "lblResolution")};
        _sources->Translator.bind(lbl->Label, "ux", lbl->name());
    }

    // fullscreen
    {
        auto chk {tabPanelLayout->create_widget<checkbox>({6, 6, 2, 2}, "chkFullScreen")};
        chk->Checked = config[Cfg::Video::Name][Cfg::Video::fullscreen].as<bool>();

        auto lbl {tabPanelLayout->create_widget<label>({1, 6, 4, 2}, "lblFullScreen")};
        _sources->Translator.bind(lbl->Label, "ux", lbl->name());
    }

    // vsync
    {
        auto chk {tabPanelLayout->create_widget<checkbox>({6, 10, 2, 2}, "chkVSync")};
        chk->Checked = config[Cfg::Video::Name][Cfg::Video::vsync].as<bool>();

        auto lbl {tabPanelLayout->create_widget<label>({1, 10, 4, 2}, "lblVSync")};
        _sources->Translator.bind(lbl->Label, "ux", lbl->name());
    }

    auto btnApplyVideoSettings {tabPanelLayout->create_widget<button>({33, 34, 4, 4}, "btnApplyVideoSettings")};
    btnApplyVideoSettings->Icon    = {_resGrp.get<gfx::texture>("apply")};
    btnApplyVideoSettings->Tooltip = _tooltip;
    btnApplyVideoSettings->Click.connect([&]() { VideoSettingsChanged(); });
}

void form_menu::create_settings_hints(tab_container& tabContainer)
{
    auto tabPanel {tabContainer.create_tab<panel>("tabHints")};
    _sources->Translator.bind_tab(&tabContainer, tabPanel.get());

    auto tabPanelLayout {tabPanel->create_layout<grid_layout>(size_i {40, 40})};

    // highlight movable
    {
        auto chk {tabPanelLayout->create_widget<toggle>({10, 2, 3, 2}, "chkHintMovable")};
        chk->Checked = _sources->Settings.HintMovable;
        chk->Checked.Changed.connect([this](auto val) { _sources->Settings.HintMovable = val; });

        auto lbl {tabPanelLayout->create_widget<label>({1, 2, 8, 2}, "lblHintMovable")};
        _sources->Translator.bind(lbl->Label, "ux", lbl->name());
    }
    // highlight drops
    {
        auto chk {tabPanelLayout->create_widget<toggle>({10, 6, 3, 2}, "chkHintDrops")};
        chk->Checked = _sources->Settings.HintTarget;
        chk->Checked.Changed.connect([this](auto val) { _sources->Settings.HintTarget = val; });

        auto lbl {tabPanelLayout->create_widget<label>({1, 6, 8, 2}, "lblHintDrops")};
        _sources->Translator.bind(lbl->Label, "ux", lbl->name());
    }
}

void form_menu::create_section_themes(tab_container& parent)
{
    // Themes
    auto panelThemes {parent.create_tab<panel>("tabThemes")};
    {
        auto panelLayout {panelThemes->create_layout<dock_layout>()};
        auto lbxThemes {panelLayout->create_widget<list_box>(dock_style::Fill, "lbxThemes")};
        for (auto const& colorTheme : _sources->Themes) { lbxThemes->add_item(colorTheme.first); }
        lbxThemes->SelectedItemIndex.Changed.connect([&, lb = lbxThemes.get()](auto val) {
            if (val != -1) { _sources->Settings.Theme = lb->selected_item().Text; }
        });
        lbxThemes->DoubleClick.connect([&] { hide(); });
        _sources->Settings.Theme.Changed.connect([lb = lbxThemes.get()](auto const& val) { lb->select_item(val); });
        lbxThemes->select_item(_sources->Settings.Theme);
    }
}

void form_menu::create_section_cardset(tab_container& parent)
{
    // Cardsets
    auto panelCardsets {parent.create_tab<panel>("tabCardset")};
    auto panelLayout {panelCardsets->create_layout<dock_layout>()};

    // listbox
    auto lbxCardsets {panelLayout->create_widget<list_box>(dock_style::Top, "lbxCardsets")};
    lbxCardsets->Class = "list_box_log";
    lbxCardsets->Flex  = {50_pct, 25_pct};
    for (auto const& cardSet : _sources->CardSets) { lbxCardsets->add_item(cardSet.first); }
    lbxCardsets->SelectedItemIndex.Changed.connect([&, lb = lbxCardsets.get()](auto val) {
        if (val != -1) { _sources->Settings.Cardset = lb->selected_item().Text; }
    });
    lbxCardsets->DoubleClick.connect([&, lb = lbxCardsets.get()] {
        if (lb->SelectedItemIndex >= 0) { hide(); }
    });

    // preview
    auto panelCards {panelLayout->create_widget<panel>(dock_style::Bottom, "panelCardsets")};
    panelCards->Flex   = {100_pct, 75_pct};
    panelCards->ZOrder = 1;
    auto panelCardsLayout {panelCards->create_layout<grid_layout>(size_i {20, 40})};

    auto const cardsetChanged {[&, lb = lbxCardsets.get(), panel = panelCards.get()](auto const& val) {
        lb->select_item(val);

        auto layout {panel->create_layout<box_layout>(size_i {13, 4})};

        auto const& cards {_sources->CardSets.at(val)};
        auto const& tex {cards->get_material()->Texture};

        for (u8 s {static_cast<u8>(suit::Hearts)}; s <= static_cast<u8>(suit::Spades); ++s) {
            for (u8 r {static_cast<u8>(rank::Ace)}; r <= static_cast<u8>(rank::King); ++r) {
                auto imgBox {layout->create_widget<image_box>("")};
                imgBox->Image = {tex, card {static_cast<suit>(s), static_cast<rank>(r), 0, false}.get_texture_name()};
            }
        }
    }};
    _sources->Settings.Cardset.Changed.connect([cardsetChanged](auto const& val) { cardsetChanged(val); });
    cardsetChanged(_sources->Settings.Cardset);
}

void form_menu::create_menubar(tab_container& parent)
{
    auto menu {create_container<panel>(dock_style::Fill, MenuName)};
    auto menuLayout {menu->create_layout<grid_layout>(size_i {12, 20})};

    i32        y {1};
    auto const create {[&](std::string const& text) {
        auto btn {menuLayout->create_widget<radio_button>({9, y, 2, 2}, "btn" + text)};

        auto lbl {menuLayout->create_widget<label>({1, y, 7, 2}, "lblBtn" + text)};
        _sources->Translator.bind(lbl->Label, "ux", lbl->name());
        lbl->For = btn;

        y += 2;
        return btn;
    }};

    {
        auto btn {create("Games")};
        btn->Checked = true;
        btn->Click.connect([p = &parent](auto const&) { p->ActiveTabIndex = 0; });
    }
    {
        auto btn {create("Settings")};
        btn->Click.connect([p = &parent](auto const&) { p->ActiveTabIndex = 1; });
    }
    {
        auto btn {create("Themes")};
        btn->Click.connect([p = &parent](auto const&) { p->ActiveTabIndex = 2; });
    }
    {
        auto btn {create("CardSets")};
        btn->Click.connect([p = &parent](auto const&) { p->ActiveTabIndex = 3; });
    }

    auto btnBack {menuLayout->create_widget<button>({1, 17, 10, 2}, "btnBack")};
    btnBack->Icon = {_resGrp.get<gfx::texture>("back")};
    btnBack->Click.connect([&](auto&) { hide(); });
    btnBack->Tooltip = _tooltip;
}

void form_menu::start_game()
{
    StartGame(helper::to_number<u64>(_txbSeed->Text()));
    _txbSeed->Text = "";
    hide();
}
}
