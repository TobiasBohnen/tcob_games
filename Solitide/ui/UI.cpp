// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "UI.hpp"

#include "games/GameInfo.hpp"

namespace solitaire {

using namespace tcob::literals;

static auto set_to_string(std::set<i32> const& numbers) -> std::string
{
    if (numbers.empty()) { return ""; }

    std::ostringstream oss;
    auto               it {numbers.begin()};
    i32                start {*it}, end {*it};
    ++it;

    while (it != numbers.end()) {
        if (*it == end + 1) {
            end = *it;
        } else {
            if (start == end) {
                oss << (start + 1);
            } else {
                oss << (start + 1) << "-" << (end + 1);
            }
            oss << ",";
            start = end = *it;
        }
        ++it;
    }

    if (start == end) {
        oss << (start + 1);
    } else {
        oss << (start + 1) << "-" << (end + 1);
    }

    return oss.str();
}

static auto get_size(std::string_view str) -> size_i
{
    i32                    width {}, height {};
    size_t const           xPos {str.find('x')};
    std::string_view const widthStrView(str.data(), xPos);
    std::string_view const heightStrView(str.data() + xPos + 1);
    std::from_chars(widthStrView.data(), widthStrView.data() + widthStrView.size(), width);
    std::from_chars(heightStrView.data(), heightStrView.data() + heightStrView.size(), height);
    return {width, height};
}

static auto make_tooltip(sources& sources, form_base* form) -> std::shared_ptr<tooltip>
{
    auto retValue {form->create_tooltip<>("tooltip")};
    retValue->Class     = "tooltip";
    form->TooltipOffset = {40, 40};

    auto& tooltipLayout {retValue->create_layout<dock_layout>()};
    auto& tooltipLabel {tooltipLayout.create_widget<label>(dock_style::Fill, "TTLabel0")};
    tooltipLabel.Class = "tooltip_label";

    retValue->Popup.connect([&sources, &tooltipLabel, tt = retValue.get()](auto const& event) -> void {
        auto* const widget {event.Widget};
        sources.Translator.bind(tooltipLabel.Label, "ux", widget->name());

        rect_f const bounds {widget->Bounds};
        auto const*  style {dynamic_cast<label::style const*>(tooltipLabel.current_style())};
        assert(style);
        auto* const font {style->Text.Font->get_font(style->Text.Style, style->Text.calc_font_size(bounds.height() * 0.75f)).ptr()};
        tt->Bounds = {point_f::Zero, gfx::text_formatter::measure(*tooltipLabel.Label, *font, -1, true)};
    });
    return retValue;
}

form_controls::form_controls(gfx::window& window, assets::group& resGrp, std::shared_ptr<sources> sources)
    : form {{.Name = "Controls", .Bounds = window.bounds()}}
    , _sources {std::move(sources)}
{
    Controls.mutate([&](auto& ctrls) { ctrls.ActivateKey = input::key_code::UNKNOWN; });

    auto tooltip0 {make_tooltip(*_sources, this)};

    auto& mainPanel {create_container<glass>(dock_style::Fill, "main")};
    auto& mainPanelLayout {mainPanel.create_layout<dock_layout>()};

    // menu
    {
        auto& menuPanel {mainPanelLayout.create_widget<panel>(dock_style::Top, "menu")};
        menuPanel.Flex = {.Width = 100_pct, .Height = 5_pct};
        auto& menuPanelLayout {menuPanel.create_layout<grid_layout>(size_i {20, 1})};

        auto const create {[&](rect_i const& bounds, std::string const& name, std::string const& tex) -> button& {
            auto& retValue {menuPanelLayout.create_widget<button>(bounds, name)};
            retValue.Icon    = {.Texture = resGrp.get<gfx::texture>(tex)};
            retValue.Tooltip = tooltip0;
            return retValue;
        }};

        create({0, 0, 1, 1}, "btnMenu", "burger").Click.connect([&] { _sources->Events.ShowMenu(); });
        create({2, 0, 1, 1}, "btnWizard", "wand").Click.connect([&] { _sources->Events.ShowWizard(); });
        create({12, 0, 1, 1}, "btnNewGame", "newgame").Click.connect([&] { _sources->Events.RestartGame(); });
        create({14, 0, 1, 1}, "btnHint", "hint").Click.connect([&] { _sources->Events.ShowHint(); });
        create({15, 0, 1, 1}, "btnCollect", "collect").Click.connect([&] { _sources->Events.Collect(); });
        create({16, 0, 1, 1}, "btnUndo", "undo").Click.connect([&] { _sources->Events.Undo(); });
        create({19, 0, 1, 1}, "btnQuit", "exit").Click.connect([&] { _sources->Events.Quit(); });
    }

    // status
    {
        auto& statusPanel {mainPanelLayout.create_widget<panel>(dock_style::Bottom, "status")};
        statusPanel.Flex = {.Width = 100_pct, .Height = 10_pct};
        auto& statusPanelLayout {statusPanel.create_layout<grid_layout>(size_i {40, 8})};

        i32        lbID {0};
        auto const create {[&](rect_i const& rect, std::string const& id = "") {
            auto& l {statusPanelLayout.create_widget<label>(rect, "lblStatus" + std::to_string(lbID++))};
            l.Class = "status_label";
            if (!id.empty()) {
                _sources->Translator.bind(l.Label, "status", id);
            }
            return &l;
        }};

        _lblGameName = create({0, 0, 4, 8});

        _lblPile           = create({4, 3, 4, 5});
        _lblPileLabel      = create({4, 0, 4, 3}, "Pile");
        _lblCardCount      = create({8, 3, 2, 5});
        _lblCardCountLabel = create({8, 0, 2, 3}, "Cards");

        _lblBase             = create({11, 3, 6, 5});
        _lblBaseLabel        = create({11, 0, 6, 3});
        _lblDescription      = create({17, 3, 10, 5});
        _lblDescriptionLabel = create({17, 0, 10, 3});
        _lblMove             = create({27, 3, 6, 5});
        _lblMoveLabel        = create({27, 0, 6, 3});

        _lblTurns      = create({34, 3, 2, 5});
        _lblTurnsLabel = create({34, 0, 2, 3}, "Turns");

        _lblScore      = create({36, 3, 2, 5});
        _lblScoreLabel = create({36, 0, 2, 3}, "Score");

        _lblTime      = create({38, 3, 2, 5});
        _lblTimeLabel = create({38, 0, 2, 3}, "Time");
    }
}

void form_controls::set_pile_labels(pile const* pile, data::object const& currentRules, game_state const& state)
{
    if (!pile) {
        set_pile_labels({});
        return;
    }

    pile_description str;
    str.Pile      = get_pile_type_name(pile->Type);
    str.CardCount = std::to_string(pile->size());

    data::array rules;
    auto const& gameObj {currentRules};
    if (!gameObj.try_get(rules, str.Pile, "rules")) { return; }

    for (auto const& rule : rules) {
        auto const ruleObj {rule.as<data::object>()};
        if (std::unordered_set<i32> piles; ruleObj.try_get(piles, "piles")) {
            if (!piles.contains(pile->Index)) { continue; }
        }

        switch (pile->Type) {
        case pile_type::Stock: {
            str.Description      = state.Redeals < 0 ? "∞" : std::to_string(state.Redeals);
            str.DescriptionLabel = "Redeals";
        } break;
        case pile_type::Waste:
        case pile_type::Reserve:
        case pile_type::FreeCell:
        case pile_type::Foundation:
        case pile_type::Tableau:    {
            str.Description      = ruleObj["build"].as<std::string>();
            str.DescriptionLabel = "Build";
            str.Move             = ruleObj["move"].as<std::string>();
            str.MoveLabel        = "Move";
            str.Base             = ruleObj["base"].as<std::string>();
            str.BaseLabel        = "Base";
            break;
        }
        }

        set_pile_labels(str);
        return;
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

form_menu::form_menu(gfx::window& window, assets::group& resGrp, std::shared_ptr<sources> sources)
    : form {{.Name = "Menu", .Bounds = window.bounds()}}
    , _resGrp {resGrp}
    , _sources {std::move(sources)}
{
    _tooltip = make_tooltip(*_sources, this);

    auto& tabContainer {create_container<tab_container>(dock_style::Right, "tabMenu")};
    tabContainer.Class = "tab_container_hidden";
    tabContainer.Flex  = {.Width = 85_pct, .Height = 100_pct};

    create_section_games(tabContainer);
    create_section_settings(tabContainer);
    create_section_themes(tabContainer);
    create_section_cardset(tabContainer);
    create_menubar(tabContainer);
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
    auto& panelGames {parent.create_tab<panel>("tabGames")};
    auto& panelLayout {panelGames.create_layout<dock_layout>()};

    create_game_details(panelLayout);
    create_game_lists(panelLayout);
}

void form_menu::create_game_lists(dock_layout& panelLayout)
{
    // filter
    auto& panelFilter {panelLayout.create_widget<panel>(dock_style::Top, "panelFilter")};
    panelFilter.Class = "inner_panel";
    panelFilter.Flex  = {.Width = 100_pct, .Height = 5_pct};

    auto& panelFilterLayout {panelFilter.create_layout<grid_layout>(size_i {10, 1})};
    auto& txbFilter {panelFilterLayout.create_widget<text_box>({0, 0, 9, 1}, "txbFilter")};
    txbFilter.MaxLength = 30;
    auto& btnClearFilter {panelFilterLayout.create_widget<button>({9, 0, 1, 1}, "btnClearFilter")};
    btnClearFilter.Icon = {.Texture = _resGrp.get<gfx::texture>("cross")};
    btnClearFilter.Click.connect([&txbFilter] -> void { txbFilter.Text = ""; });
    btnClearFilter.Tooltip = _tooltip;

    // games
    std::vector<list_box*> listBoxes;
    auto&                  tabGames {panelLayout.create_widget<tab_container>(dock_style::Left, "tabGames")};
    tabGames.Flex = {.Width = 50_pct, .Height = 100_pct};

    i32        lbID {0};
    auto const createListBox {[&](dock_layout& tabPanelLayout, auto&& pred) -> list_box* {
        auto& listBox {tabPanelLayout.create_widget<list_box>(dock_style::Fill, "lbxGames" + std::to_string(lbID++))};
        listBoxes.push_back(&listBox);

        std::vector<item> listBoxItems;
        for (auto const& game : _sources->Games) {
            if (pred(game.second.first)) { listBoxItems.push_back({.Text = game.first}); }
        }
        listBox.Items = listBoxItems;

        listBox.SelectedItemIndex.Changed.connect([this, &listBox](auto val) {
            if (val != -1) { _sources->SelectedGame = listBox.selected_item().Text; }
        });
        _sources->SelectedGame.Changed.connect([&listBox](auto const& val) {
            if (listBox.select_item(val)) {
                if (!listBox.is_focused()) { listBox.scroll_to_selected(); }
            } else {
                listBox.SelectedItemIndex = -1;
            }
        });
        listBox.DoubleClick.connect([this, &listBox] {
            if (listBox.SelectedItemIndex >= 0) { start_game(); }
        });
        return &listBox;
    }};

    // By Name
    {
        auto& tabPanel {tabGames.create_tab<panel>("tabByName")};
        tabPanel.Class = "inner_panel";
        _sources->Translator.bind_tab(&tabGames, &tabPanel);

        auto& tabPanelLayout {tabPanel.create_layout<dock_layout>()};
        auto* listBox {createListBox(tabPanelLayout, [](auto const&) { return true; })};
        _sources->Events.GameAdded.connect([this, listBox] -> void {
            // TODO: update all listboxes
            std::vector<item> listBoxItems;
            for (auto const& game : _sources->Games) { listBoxItems.push_back({.Text = game.first}); }
            listBox->Items = listBoxItems;
        });
    }
    // Recent
    {
        auto const updateList {[this] {
            std::vector<item> items;
            for (auto const& game : *_sources->Settings.Recent) {
                items.push_back({.Text = game});
            }
            return items;
        }};

        auto& tabPanel {tabGames.create_tab<panel>("tabRecent")};
        tabPanel.Class = "inner_panel";
        _sources->Translator.bind_tab(&tabGames, &tabPanel);

        auto& tabPanelLayout {tabPanel.create_layout<dock_layout>()};
        auto* listBox {createListBox(tabPanelLayout, [](auto const&) { return false; })};

        listBox->Items = updateList();
        _sources->Settings.Recent.Changed.connect([listBox, updateList] {
            listBox->Items             = updateList();
            listBox->SelectedItemIndex = -1;
        });
    }
    // By Family
    {
        auto& acc {tabGames.create_tab<accordion>("tabByFamily")};
        acc.Class                 = "accordion2";
        acc.MaximizeActiveSection = true;
        _sources->Translator.bind_tab(&tabGames, &acc);

        auto const createSection {[&](family family) {
            auto& secPanel {acc.create_section<panel>("")};
            secPanel.Class = "inner_panel";
            auto& secPanelLayout {secPanel.create_layout<dock_layout>()};
            _sources->Translator.bind(
                [tabC = &acc, tabP = &secPanel](std::string const& val) { tabC->change_section_label(tabP, val); },
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
        auto& tabContainer {tabGames.create_tab<tab_container>("tabByDeckCount")};
        _sources->Translator.bind_tab(&tabGames, &tabContainer);

        auto const createTab {[&](isize count, std::string const& name) {
            auto& tabPanel {tabContainer.create_tab<panel>(name)};
            tabPanel.Class = "inner_panel";
            auto& tabPanelLayout {tabPanel.create_layout<dock_layout>()};
            createListBox(tabPanelLayout, [count](auto const& gameInfo) { return gameInfo.DeckCount == count; });
        }};
        createTab(1, "1");
        createTab(2, "2");
        createTab(3, "3");

        {
            auto& tabPanel {tabContainer.create_tab<panel>(">= 4")};
            tabPanel.Class = "inner_panel";
            auto& tabPanelLayout {tabPanel.create_layout<dock_layout>()};
            createListBox(tabPanelLayout, [](auto const& gameInfo) { return gameInfo.DeckCount >= 4; });
        }
        {
            auto& tabPanel {tabContainer.create_tab<panel>("tabStripped")};
            tabPanel.Class = "inner_panel";
            _sources->Translator.bind_tab(&tabContainer, &tabPanel);

            auto& tabPanelLayout {tabPanel.create_layout<dock_layout>()};
            createListBox(tabPanelLayout, [](auto const& gameInfo) { return gameInfo.DeckRanks.size() < 13 || gameInfo.DeckSuits.size() < 4; });
        }
    }

    txbFilter.Text.Changed.connect([listBoxes](auto const& val) {
        for (auto* lb : listBoxes) {
            lb->Filter = val;
        }
    });
}

void form_menu::create_game_details(dock_layout& panelLayout)
{
    auto& panelGameDetails {panelLayout.create_widget<panel>(dock_style::Right, "panelGameDetails")};
    panelGameDetails.Class  = "inner_panel";
    panelGameDetails.Flex   = {.Width = 50_pct, .Height = 100_pct};
    panelGameDetails.ZOrder = 1;
    auto& panelGameStatsLayout {panelGameDetails.create_layout<grid_layout>(size_i {20, 40})};

    auto& lblSeed {panelGameStatsLayout.create_widget<label>({1, 33, 4, 2}, "lblSeed")};
    _sources->Translator.bind(lblSeed.Label, "ux", lblSeed.name());
    lblSeed.Class = "label";
    _txbSeed      = &panelGameStatsLayout.create_widget<text_box>({6, 33, 8, 2}, "txbSeed");
    _txbSeed->BeforeTextInserted.connect([](text_event& ev) {
        if (ev.Text.empty()) { return; }
        if (ev.Text.size() != 1 || !std::isdigit(ev.Text[0])) {
            ev.Text = "";
        }
    });

    auto& btnStartGame {panelGameStatsLayout.create_widget<button>({1, 36, 4, 3}, "btnStartGame")};
    btnStartGame.Icon = {.Texture = _resGrp.get<gfx::texture>("play")};
    btnStartGame.Click.connect([this] { start_game(); });
    btnStartGame.Tooltip = _tooltip;

    auto& tabGameDetails {panelGameStatsLayout.create_widget<tab_container>({0, 0, 20, 32}, "tabGameDetails")};

    // info tab
    {
        auto& tabPanel {tabGameDetails.create_tab<panel>("tabInfo")};
        tabPanel.Class = "inner_panel";
        _sources->Translator.bind_tab(&tabGameDetails, &tabPanel);

        auto& tabPanelLayout {tabPanel.create_layout<grid_layout>(size_i {40, 30})};

        auto& gvInfo {tabPanelLayout.create_widget<grid_view>({2, 1, 36, 5}, "gvInfo")};
        gvInfo.Class      = "grid_view2";
        gvInfo.SelectMode = grid_select_mode::None;
        auto addRow {[this, infoGV = &gvInfo](auto const& game) {
            infoGV->Grid.mutate([&](auto& grid) {
                grid.resize({3, 1});

                if (!_sources->Games.contains(game)) { return; }
                auto gameInfo {_sources->Games[game].first};

                grid.assign({0, 0},
                            {{_sources->Translator.translate("family", gameInfo.Family)},
                             {std::to_string(gameInfo.DeckCount)},
                             {gameInfo.Redeals < 0 ? "∞" : std::to_string(gameInfo.Redeals)}});
            });
        }};
        _sources->Translator.bind(
            [this, addRow, &gvInfo](std::vector<std::string> const& val) {
                std::vector<item> items;
                items.reserve(val.size());
                for (auto const& str : val) { items.push_back({.Text = str}); }
                gvInfo.Header = items;
                addRow(_sources->SelectedGame);
            },
            "columns", "info");

        _sources->SelectedGame.Changed.connect(addRow);

        auto& accRules {tabPanelLayout.create_widget<accordion>({1, 7, 38, 22}, "gvRules")};

        _sources->SelectedRules.Changed.connect([&accRules](data::object const& piles) {
            accRules.clear_sections();

            for (auto const& pile : piles) {
                auto const& name {pile.first};
                auto&       pilePanel {accRules.create_section<panel>(name)};
                pilePanel.Class = "inner_panel";
                auto const& pileObj {pile.second.as<data::object>()};
                //    auto const  count {pileObj["count"].as<std::string>()};
                auto const& rulesArr {pileObj["rules"].as<data::array>()};

                auto createRule {[&](grid_layout& layout, data::object const& rule) {
                    auto const create {[&](rect_i const& rect, std::string const& text) {
                        auto& l {layout.create_widget<label>(rect, "")};
                        l.Class = "label";
                        l.Label = text;
                        return &l;
                    }};

                    create({1, 1, 10, 5}, "Base");
                    create({12, 1, 17, 5}, rule["base"].as<std::string>());
                    create({1, 7, 10, 5}, "Build");
                    create({12, 7, 17, 5}, rule["build"].as<std::string>());
                    create({1, 13, 10, 5}, "Move");
                    create({12, 13, 17, 5}, rule["move"].as<std::string>());
                }};

                if (rulesArr.size() == 1) {
                    auto& pilePanelLayout {pilePanel.create_layout<grid_layout>(size_i {30, 25})};
                    createRule(pilePanelLayout, rulesArr[0].as<data::object>());
                } else {
                    auto& pilePanelLayout {pilePanel.create_layout<dock_layout>()};
                    auto& tabRules {pilePanelLayout.create_widget<tab_container>(dock_style::Fill, "")};
                    tabRules.Class = "tab_container_small";
                    for (auto const& rule : rulesArr) {
                        auto const& ruleObj {rule.as<data::object>()};
                        auto&       panelRule {tabRules.create_tab<panel>(set_to_string(ruleObj["piles"].as<std::set<i32>>()))};
                        panelRule.Class = "inner_panel";
                        auto& panelRuleLayout {panelRule.create_layout<grid_layout>(size_i {30, 25})};
                        createRule(panelRuleLayout, ruleObj);
                    }
                }
            }
        });
    }
    // stats tab
    {
        auto& tabPanel {tabGameDetails.create_tab<panel>("tabStats")};
        tabPanel.Class = "inner_panel";
        _sources->Translator.bind_tab(&tabGameDetails, &tabPanel);

        auto& tabPanelLayout {tabPanel.create_layout<grid_layout>(size_i {40, 30})};

        auto& gvWL {tabPanelLayout.create_widget<grid_view>({1, 1, 18, 5}, "gvWinLose")};
        gvWL.Class      = "grid_view2";
        gvWL.SelectMode = grid_select_mode::None;
        _sources->Translator.bind_grid_header(&gvWL, "wl");

        auto& gvScore {tabPanelLayout.create_widget<grid_view>({21, 1, 18, 5}, "gvBest")};
        gvScore.Class      = "grid_view2";
        gvScore.SelectMode = grid_select_mode::None;
        _sources->Translator.bind_grid_header(&gvScore, "score");

        auto& gvHistory {tabPanelLayout.create_widget<grid_view>({1, 7, 38, 22}, "gvHistory")};
        gvHistory.SelectMode = grid_select_mode::Row;
        _sources->Translator.bind_grid_header(&gvHistory, "history");

        _sources->SelectedHistory.Changed.connect([&gvWL, &gvScore, &gvHistory](auto const& stats) {
            grid<item> wlGrid {{3, 1}};
            wlGrid.assign({0, 0},
                          {{.Text = std::to_string(stats.Won)},
                           {.Text = std::to_string(stats.Lost)},
                           {.Text = stats.Lost + stats.Won > 0 ? std::format("{:.2f}%", static_cast<f32>(stats.Won) / (stats.Lost + stats.Won) * 100) : "-"}});
            gvWL.Grid = wlGrid;

            std::optional<i64> bestTime;
            std::optional<i64> bestTurns;
            std::optional<i64> bestScore;

            grid<item> historyGrid {{5, 0}};
            for (auto const& entry : stats.Entries) {
                // FIXME: performance
                if (entry.Won) {
                    bestTime  = !bestTime ? entry.Time : std::min(entry.Time, *bestTime);
                    bestTurns = !bestTurns ? entry.Turns : std::min(entry.Turns, *bestTurns);
                }
                bestScore = !bestScore ? entry.Score : std::max(entry.Score, *bestScore);

                historyGrid.append(
                    {{std::to_string(entry.Seed)},
                     {std::to_string(entry.Score)},
                     {std::to_string(entry.Turns)},
                     {std::format("{:%M:%S}", seconds {entry.Time / 1000.f})},
                     {entry.Won ? "X" : "-"}});
            }
            gvHistory.Grid = historyGrid;

            grid<item> ttGrid {{3, 1}};
            ttGrid.assign({0, 0},
                          {{.Text = bestScore ? std::to_string(*bestScore) : "-"},
                           {.Text = bestTurns ? std::to_string(*bestTurns) : "-"},
                           {.Text = bestTime ? std::format("{:%M:%S}", seconds {*bestTime / 1000.f}) : "--:--"}});
            gvScore.Grid = ttGrid;
        });
    }
}

void form_menu::create_section_settings(tab_container& parent)
{
    // Setting
    auto& tabSettings {parent.create_tab<tab_container>(TabSettingsName)};

    create_settings_video(tabSettings);
    create_settings_hints(tabSettings);
}

void form_menu::create_settings_video(tab_container& tabContainer)
{
    auto const& config {locate_service<platform>().config()};
    auto&       tabPanel {tabContainer.create_tab<panel>("tabVideo")};
    _sources->Translator.bind_tab(&tabContainer, &tabPanel);

    auto& tabPanelLayout {tabPanel.create_layout<grid_layout>(size_i {40, 40})};

    // resolution
    {
        auto const          displayModes {locate_service<platform>().displays().begin()->second.Modes};
        std::vector<size_i> displaySizes;
        for (auto const& dm : displayModes) {
            if (displaySizes.empty() || displaySizes.back() != dm.Size) {
                displaySizes.push_back(dm.Size);
            }
        }

        auto& ddlRes {tabPanelLayout.create_widget<drop_down_list>({6, 1, 6, 4}, "ddlResolution")};
        ddlRes.ZOrder = 1;
        ddlRes.Items.mutate([&](auto& items) {
            for (auto const& ds : displaySizes) {
                items.push_back({std::format("{}x{}", ds.Width, ds.Height)});
            }
        });

        auto const res {config[Cfg::Video::Name][Cfg::Video::resolution].as<size_i>()};
        ddlRes.select_item(std::format("{}x{}", res.Width, res.Height));

        auto& lbl {tabPanelLayout.create_widget<label>({1, 2, 4, 2}, "lblResolution")};
        _sources->Translator.bind(lbl.Label, "ux", lbl.name());
    }

    // fullscreen
    {
        auto& chk {tabPanelLayout.create_widget<checkbox>({6, 6, 2, 2}, "chkFullScreen")};
        chk.Checked = config[Cfg::Video::Name][Cfg::Video::fullscreen].as<bool>();

        auto& lbl {tabPanelLayout.create_widget<label>({1, 6, 4, 2}, "lblFullScreen")};
        _sources->Translator.bind(lbl.Label, "ux", lbl.name());
    }

    // vsync
    {
        auto& chk {tabPanelLayout.create_widget<checkbox>({6, 10, 2, 2}, "chkVSync")};
        chk.Checked = config[Cfg::Video::Name][Cfg::Video::vsync].as<bool>();

        auto& lbl {tabPanelLayout.create_widget<label>({1, 10, 4, 2}, "lblVSync")};
        _sources->Translator.bind(lbl.Label, "ux", lbl.name());
    }

    auto& btnApplyVideoSettings {tabPanelLayout.create_widget<button>({33, 34, 4, 4}, "btnApplyVideoSettings")};
    btnApplyVideoSettings.Icon    = {.Texture = _resGrp.get<gfx::texture>("apply")};
    btnApplyVideoSettings.Tooltip = _tooltip;
    btnApplyVideoSettings.Click.connect([this] {
        auto const res {get_size(dynamic_cast<drop_down_list*>(find_widget_by_name("ddlResolution"))->selected_item().Text)};
        _sources->Events.VideoSettingsChanged({.FullScreen = dynamic_cast<checkbox*>(find_widget_by_name("chkFullScreen"))->Checked,
                                               .VSync      = dynamic_cast<checkbox*>(find_widget_by_name("chkVSync"))->Checked,
                                               .Resolution = res});
    });
}

void form_menu::create_settings_hints(tab_container& tabContainer)
{
    auto& tabPanel {tabContainer.create_tab<panel>("tabHints")};
    _sources->Translator.bind_tab(&tabContainer, &tabPanel);

    auto& tabPanelLayout {tabPanel.create_layout<grid_layout>(size_i {40, 40})};

    // highlight movable
    {
        auto& chk {tabPanelLayout.create_widget<toggle>({10, 2, 3, 2}, "chkHintMovable")};
        chk.Checked = _sources->Settings.HintMovable;
        chk.Checked.Changed.connect([this](auto val) { _sources->Settings.HintMovable = val; });

        auto& lbl {tabPanelLayout.create_widget<label>({1, 2, 8, 2}, "lblHintMovable")};
        _sources->Translator.bind(lbl.Label, "ux", lbl.name());
    }
    // highlight drops
    {
        auto& chk {tabPanelLayout.create_widget<toggle>({10, 6, 3, 2}, "chkHintDrops")};
        chk.Checked = _sources->Settings.HintTarget;
        chk.Checked.Changed.connect([this](auto val) { _sources->Settings.HintTarget = val; });

        auto& lbl {tabPanelLayout.create_widget<label>({1, 6, 8, 2}, "lblHintDrops")};
        _sources->Translator.bind(lbl.Label, "ux", lbl.name());
    }
}

void form_menu::create_section_themes(tab_container& parent)
{
    // Themes
    auto& panelThemes {parent.create_tab<panel>("tabThemes")};
    {
        auto& panelLayout {panelThemes.create_layout<dock_layout>()};
        auto& lbxThemes {panelLayout.create_widget<list_box>(dock_style::Fill, "lbxThemes")};
        lbxThemes.Items.mutate([&](auto& items) {
            for (auto const& colorTheme : _sources->Themes) { items.push_back({colorTheme.first}); }
        });

        lbxThemes.SelectedItemIndex.Changed.connect([this, &lbxThemes](auto val) {
            if (val != -1) { _sources->Settings.Theme = lbxThemes.selected_item().Text; }
        });
        lbxThemes.DoubleClick.connect([this] { hide(); });
        _sources->Settings.Theme.Changed.connect([&lbxThemes](auto const& val) { lbxThemes.select_item(val); });
        lbxThemes.select_item(_sources->Settings.Theme);
    }
}

void form_menu::create_section_cardset(tab_container& parent)
{
    // Cardsets
    auto& panelCardsets {parent.create_tab<panel>("tabCardset")};
    auto& panelLayout {panelCardsets.create_layout<dock_layout>()};

    // listbox
    auto& lbxCardsets {panelLayout.create_widget<list_box>(dock_style::Top, "lbxCardsets")};
    lbxCardsets.Class = "list_box_log";
    lbxCardsets.Flex  = {.Width = 50_pct, .Height = 25_pct};
    lbxCardsets.Items.mutate([&](auto& items) {
        for (auto const& cardSet : _sources->CardSets) { items.push_back({cardSet.first}); }
    });
    lbxCardsets.SelectedItemIndex.Changed.connect([this, &lbxCardsets](auto val) {
        if (val != -1) { _sources->Settings.Cardset = lbxCardsets.selected_item().Text; }
    });
    lbxCardsets.DoubleClick.connect([this, &lbxCardsets] {
        if (lbxCardsets.SelectedItemIndex >= 0) { hide(); }
    });

    // preview
    auto& panelCards {panelLayout.create_widget<panel>(dock_style::Bottom, "panelCardsets")};
    panelCards.Flex   = {.Width = 100_pct, .Height = 75_pct};
    panelCards.ZOrder = 1;

    auto const cardsetChanged {[this, &lbxCardsets, &panelCards](auto const& val) {
        lbxCardsets.select_item(val);

        auto& layout {panelCards.create_layout<box_layout>(size_i {13, 4})};

        auto const& cards {_sources->CardSets.at(val)};
        auto const& tex {cards->get_material()->Texture};

        for (u8 s {static_cast<u8>(suit::Hearts)}; s <= static_cast<u8>(suit::Spades); ++s) {
            for (u8 r {static_cast<u8>(rank::Ace)}; r <= static_cast<u8>(rank::King); ++r) {
                auto& imgBox {layout.create_widget<image_box>("")};
                imgBox.Image = {tex, card {static_cast<suit>(s), static_cast<rank>(r), 0, false}.get_texture_name()};
            }
        }
    }};
    _sources->Settings.Cardset.Changed.connect([cardsetChanged](auto const& val) { cardsetChanged(val); });
    cardsetChanged(_sources->Settings.Cardset);
}

void form_menu::create_menubar(tab_container& parent)
{
    auto& menu {create_container<panel>(dock_style::Fill, MenuName)};
    auto& menuLayout {menu.create_layout<grid_layout>(size_i {12, 20})};

    i32        y {5};
    auto const create {[&](std::string const& text) {
        auto& btn {menuLayout.create_widget<radio_button>({9, y, 2, 2}, "btn" + text)};

        auto& lbl {menuLayout.create_widget<label>({1, y, 7, 2}, "lblBtn" + text)};
        _sources->Translator.bind(lbl.Label, "ux", lbl.name());
        lbl.For = &btn;

        y += 2;
        return &btn;
    }};

    {
        auto* btn {create("Games")};
        btn->Checked = true;
        btn->Click.connect([p = &parent](auto const&) { p->ActiveTabIndex = 0; });
    }
    {
        auto* btn {create("Settings")};
        btn->Click.connect([p = &parent](auto const&) { p->ActiveTabIndex = 1; });
    }
    {
        auto* btn {create("Themes")};
        btn->Click.connect([p = &parent](auto const&) { p->ActiveTabIndex = 2; });
    }
    {
        auto* btn {create("CardSets")};
        btn->Click.connect([p = &parent](auto const&) { p->ActiveTabIndex = 3; });
    }

    auto& btnBack {menuLayout.create_widget<button>({0, 0, 4, 1}, "btnBack")};
    btnBack.Icon = {.Texture = _resGrp.get<gfx::texture>("back")};
    btnBack.Click.connect([this](auto&) { hide(); });
    btnBack.Tooltip = _tooltip;
}

void form_menu::start_game()
{
    _sources->Events.StartGame(helper::to_number<u64>(*_txbSeed->Text));
    _txbSeed->Text = "";
    hide();
}
}
