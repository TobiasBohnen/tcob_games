// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "UI.hpp"

#include "Games.hpp"

namespace solitaire {

using namespace tcob::literals;

auto static translate(string const& name) -> string // NOLINT
{
    // TODO: translation
    if (name == "btnMenu") { return "Menu"; }
    if (name == "btnNewGame") { return "New Game"; }
    if (name == "btnWizard") { return "Solitaire Wizard"; }
    if (name == "btnHint") { return "Hint"; }
    if (name == "btnCollect") { return "Collect All"; }
    if (name == "btnUndo") { return "Undo"; }
    if (name == "btnQuit") { return "Quit"; }
    if (name == "btnFilter") { return "Clear Filter"; }
    if (name == "btnStartGame") { return "Start Game"; }
    if (name == "btnApply") { return "Apply"; }
    if (name == "btnBack") { return "Back"; }

    return "";
}

auto static make_tooltip(form* form) -> std::shared_ptr<tooltip>
{
    auto retValue {form->create_tooltip<tooltip>("tooltip")};
    auto tooltipLayout {retValue->create_layout<dock_layout>()};
    auto tooltipLabel {tooltipLayout->create_widget<label>(dock_style::Fill, "TTLabel0")};
    tooltipLabel->Class = "tooltip-label";
    retValue->Popup.connect([form, lbl = tooltipLabel.get(), tt = retValue.get()](auto const& event) {
        auto const widget {event.Widget};
        lbl->Label = translate(widget->get_name());

        auto const  wBounds {widget->Bounds()};
        auto const* lStyle {lbl->get_style<label::style>()};

        auto size {form->measure_text(
            lStyle->Text,
            lStyle->Text.calc_font_size({0, 0, wBounds.Width * 1.5f, wBounds.Height * 0.75f}),
            lbl->Label())};
        tt->Bounds = {point_f::Zero, size};
    });
    return retValue;
}

form_controls::form_controls(gfx::window* window, assets::group& resGrp)
    : form {"Controls", window}
{
    (*Controls).ActivateKey = input::key_code::UNKNOWN;

    // tooltip
    auto tooltip0 {make_tooltip(this)};

    auto mainPanel {create_container<glass>(dock_style::Fill, "main")};
    auto mainPanelLayout {mainPanel->create_layout<dock_layout>()};

    // menu
    {
        auto menuPanel {mainPanelLayout->create_widget<panel>(dock_style::Top, "menu")};
        menuPanel->Flex = {100_pct, 5_pct};
        auto menuPanelLayout {menuPanel->create_layout<grid_layout>(size_i {20, 1})};

        auto const create {[&](rect_i const& bounds, string const& name, string const& tex) {
            auto retValue {menuPanelLayout->create_widget<button>(bounds, name)};
            retValue->Icon    = resGrp.get<gfx::texture>(tex);
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
        auto statusPanelLayout {statusPanel->create_layout<grid_layout>(size_i {20, 6})};

        i32        i {0};
        auto const create {[&](rect_i const& rect, string const& text = "") {
            auto l {statusPanelLayout->create_widget<label>(rect, "lblInfo" + std::to_string(i))};
            l->Class = "label-small";
            l->Label = text;
            return l;
        }};

        _lblGameName = create({0, 0, 2, 6});

        _lblPile           = create({2, 3, 2, 3});
        _lblPileLabel      = create({2, 0, 2, 3}, "Pile");
        _lblCardCount      = create({4, 3, 1, 3});
        _lblCardCountLabel = create({4, 0, 1, 3}, "Cards");

        _lblBase             = create({6, 3, 3, 3});
        _lblBaseLabel        = create({6, 0, 3, 3});
        _lblDescription      = create({9, 3, 3, 3});
        _lblDescriptionLabel = create({9, 0, 3, 3});
        _lblMove             = create({12, 3, 3, 3});
        _lblMoveLabel        = create({12, 0, 3, 3});

        _lblTurns      = create({17, 3, 1, 3});
        _lblTurnsLabel = create({17, 0, 1, 3}, "Turns");

        _lblScore      = create({18, 3, 1, 3});
        _lblScoreLabel = create({18, 0, 1, 3}, "Score");

        _lblTime      = create({19, 3, 1, 3});
        _lblTimeLabel = create({19, 0, 1, 3}, "Time");
    }

    auto overlayPanel {mainPanelLayout->create_widget<glass>(dock_style::Fill, "overlay")};
    overlayPanel->disable();
    auto overlayPanelLayout {overlayPanel->create_layout<dock_layout>()};

    Canvas = overlayPanelLayout->create_widget<canvas_widget>(dock_style::Fill, "canvas");
}

void form_controls::set_pile_labels(pile_description const& str)
{
    _lblPile->Label      = str.Pile;
    _lblCardCount->Label = str.CardCount;

    _lblDescription->Label      = str.Description;
    _lblDescriptionLabel->Label = str.DescriptionLabel;
    _lblMove->Label             = str.Move;
    _lblMoveLabel->Label        = str.MoveLabel;
    _lblBase->Label             = str.Base;
    _lblBaseLabel->Label        = str.BaseLabel;
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

static string const TabGamesName {"conGames"};
static string const TabSettingsName {"conSettings"};
static string const TabThemesName {"conThemes"};
static string const TabCardsetsName {"conCardsets"};
static string const MenuName {"menu"};

form_menu::form_menu(gfx::window* window, assets::group& resGrp, menu_sources const& source)
    : form {"Menu", window}
{
    // tooltip
    _tooltip = make_tooltip(this);

    create_section_games(resGrp, source.Games);
    create_section_settings(resGrp);
    create_section_themes(source.Themes);
    create_section_cardset(source.Cardsets);
    create_menubar(resGrp);
}

void form_menu::submit_settings(data::config::object& obj)
{
    _panelSettings->submit(obj);
}

void form_menu::set_game_stats(game_history const& stats)
{
    _gvWL->clear_rows();
    _gvWL->add_row(
        {std::to_string(stats.Won),
         std::to_string(stats.Lost),
         stats.Lost + stats.Won > 0 ? std::format("{:.2f}%", static_cast<f32>(stats.Won) / (stats.Lost + stats.Won) * 100) : "-"});
    _gvTT->clear_rows();

    std::optional<i64> bestTime;
    std::optional<i64> bestTurns;
    std::optional<i64> bestScore;

    _gvHistory->clear_rows();
    for (auto const& entry : stats.Entries) {
        if (entry.Won) {
            bestTime  = !bestTime ? entry.Time : std::min(entry.Time, *bestTime);
            bestTurns = !bestTurns ? entry.Turns : std::min(entry.Turns, *bestTurns);
        }
        bestScore = !bestScore ? entry.Score : std::max(entry.Score, *bestScore);

        _gvHistory->add_row(
            {std::to_string(entry.ID),
             std::to_string(entry.Score),
             std::to_string(entry.Turns),
             std::format("{:%M:%S}", seconds {entry.Time / 1000.f}),
             entry.Won ? "X" : "-"});
    }
    _gvTT->add_row({bestScore ? std::to_string(*bestScore) : "-",
                    bestTurns ? std::to_string(*bestTurns) : "-",
                    bestTime ? std::format("{:%M:%S}", seconds {*bestTime / 1000.f}) : "--:--"});
}

void form_menu::update_games(std::vector<game_info> const& games)
{
    // TODO: update all listboxes
    auto lb {std::static_pointer_cast<list_box>(find_widget_by_name("lbxGames0"))};

    lb->clear_items();
    for (auto const& game : games) {
        lb->add_item(game.Name);
    }
}

void form_menu::create_section_games(assets::group& resGrp, std::vector<game_info> const& games)
{
    // Games
    auto panelGames {create_container<panel>(dock_style::Left, TabGamesName)};
    panelGames->Flex   = {85_pct, 100_pct};
    panelGames->ZOrder = 5;
    auto panelLayout {panelGames->create_layout<dock_layout>()};

    // Filter
    auto pnlFilter {panelLayout->create_widget<panel>(dock_style::Top, "pnlFilter")};
    pnlFilter->Flex = {100_pct, 5_pct};
    auto pnlFilterLayout {pnlFilter->create_layout<grid_layout>(size_i {10, 1})};
    auto txbFilter {pnlFilterLayout->create_widget<text_box>({0, 0, 9, 1}, "txbFilter")};
    txbFilter->MaxLength = 30;
    auto btnFilter {pnlFilterLayout->create_widget<button>({9, 0, 1, 1}, "btnFilter")};
    btnFilter->Icon = resGrp.get<gfx::texture>("lens");
    btnFilter->Click.connect([tb = txbFilter.get()]() { tb->Text = ""; });
    btnFilter->Tooltip = _tooltip;

    std::vector<list_box*> listBoxes;
    {
        auto tabGames {panelLayout->create_widget<tab_container>(dock_style::Left, "tabGames")};
        tabGames->Flex    = {50_pct, 100_pct};
        tabGames->MaxTabs = 5;

        auto const createListBox {[&](std::shared_ptr<dock_layout>& tabPanelLayout, std::string const& name, auto&& pred) -> std::shared_ptr<list_box> {
            auto listBox {tabPanelLayout->create_widget<list_box>(dock_style::Fill, "lbxGames" + name)};
            listBoxes.push_back(listBox.get());

            for (auto const& game : games) {
                if (pred(game)) { listBox->add_item(game.Name); }
            }

            listBox->SelectedItemIndex.Changed.connect([&, lb = listBox.get()](auto val) {
                if (val != -1) { SelectedGame = lb->get_selected_item(); }
            });

            SelectedGame.Changed.connect([&, lb = listBox.get()](auto const& val) {
                if (lb->select_item(val)) {
                    if (!lb->is_focused()) { lb->scroll_to_selected(); }
                } else {
                    lb->SelectedItemIndex = -1;
                }
            });

            listBox->DoubleClick.connect([&, lb = listBox.get()] {
                if (lb->SelectedItemIndex >= 0) { BtnStartGame->Click({lb}); }
            });
            return listBox;
        }};

        // By Name
        {
            auto tabPanel {tabGames->create_tab<panel>("byName", "By Name")};
            auto tabPanelLayout {tabPanel->create_layout<dock_layout>()};
            createListBox(tabPanelLayout, "0", [](auto const&) { return true; });
        }
        // Recent
        {
            auto tabPanel {tabGames->create_tab<panel>("recent", "Recent")};
            auto tabPanelLayout {tabPanel->create_layout<dock_layout>()};
            auto lbxRecent {createListBox(tabPanelLayout, "1", [](auto const&) { return false; })};
            RecentGames.Changed.connect([lb = lbxRecent.get()](auto const& val) {
                lb->clear_items();
                for (auto const& g : val) { lb->add_item(g); }
            });
        }
        // By Family
        {
            auto tabContainer {tabGames->create_tab<tab_container>("byFamily", "By Family")};
            tabContainer->MaxTabs = 5;

            auto const createTab {[&](family family, std::string const& name) {
                auto tabPanel {tabContainer->create_tab<panel>(name)};
                auto tabPanelLayout {tabPanel->create_layout<dock_layout>()};
                createListBox(tabPanelLayout, name, [family](auto const& game) { return game.Family == family; });
            }};
            createTab(family::BakersDozen, "Baker's Dozen");
            createTab(family::BeleagueredCastle, "Beleaguered Castle");
            createTab(family::Canfield, "Canfield");
            createTab(family::Fan, "Fan");
            createTab(family::FlowerGarden, "Flower Garden");
            createTab(family::FortyThieves, "Forty Thieves");
            createTab(family::FreeCell, "FreeCell");
            createTab(family::Golf, "Golf");
            createTab(family::Gypsy, "Gypsy");
            createTab(family::Klondike, "Klondike");
            createTab(family::Montana, "Montana");
            createTab(family::Numerica, "Numerica");
            createTab(family::PictureGallery, "Picture Gallery");
            createTab(family::Spider, "Spider");
            createTab(family::Terrace, "Terrace");
            createTab(family::Yukon, "Yukon");

            createTab(family::Other, "Other");
        }
        // By Deck Count
        {
            auto tabContainer {tabGames->create_tab<tab_container>("byDeckCount", "By Deck Count")};
            tabContainer->MaxTabs = 5;

            auto const createTab {[&](isize count, std::string const& name) {
                auto tabPanel {tabContainer->create_tab<panel>(name)};
                auto tabPanelLayout {tabPanel->create_layout<dock_layout>()};
                createListBox(tabPanelLayout, name, [count](auto const& game) { return game.DeckCount == count; });
            }};
            createTab(1, "1");
            createTab(2, "2");
            createTab(3, "3");

            {
                auto tabPanel {tabContainer->create_tab<panel>(">= 4")};
                auto tabPanelLayout {tabPanel->create_layout<dock_layout>()};
                createListBox(tabPanelLayout, "4+", [](auto const& game) { return game.DeckCount >= 4; });
            }
            {
                auto tabPanel {tabContainer->create_tab<panel>("stripped")};
                auto tabPanelLayout {tabPanel->create_layout<dock_layout>()};
                createListBox(tabPanelLayout, "stripped", [](auto const& game) { return game.DeckRanks.size() < 13 || game.DeckSuits.size() < 4; });
            }
        }

        txbFilter->Text.Changed.connect([listBoxes](auto const& val) {
            for (auto* lb : listBoxes) {
                lb->Filter = val;
            }
        });
    }
    {
        auto panelGameStats {panelLayout->create_widget<panel>(dock_style::Right, "panelGameStats")};
        panelGameStats->Flex   = {50_pct, 100_pct};
        panelGameStats->ZOrder = 1;
        auto panelGameStatsLayout {panelGameStats->create_layout<grid_layout>(size_i {20, 40})};

        _gvWL        = panelGameStatsLayout->create_widget<grid_view>({0, 1, 10, 4}, "gvWinLose");
        _gvWL->Class = "grid_view2";
        _gvWL->set_columns({"Won", "Lost", "W/L"});
        _gvTT        = panelGameStatsLayout->create_widget<grid_view>({10, 1, 10, 4}, "gvBest");
        _gvTT->Class = "grid_view2";
        _gvTT->set_columns({"Highscore", "Least Turns", "Fastest Time"});

        _gvHistory = panelGameStatsLayout->create_widget<grid_view>({1, 6, 18, 25}, "gvHistory");
        _gvHistory->set_columns({"ID", "Score", "Turns", "Time", "Won"});

        BtnStartGame       = panelGameStatsLayout->create_widget<button>({1, 36, 4, 3}, {"btnStartGame"});
        BtnStartGame->Icon = resGrp.get<gfx::texture>("play");
        BtnStartGame->Click.connect([&]() { hide(); });
        BtnStartGame->Tooltip = _tooltip;
    }
}

void form_menu::create_section_settings(assets::group& resGrp)
{
    // Setting
    _panelSettings         = create_container<panel>(dock_style::Left, TabSettingsName);
    _panelSettings->ZOrder = 4;
    _panelSettings->Flex   = {0_pct, 0_pct};
    {
        auto        panelLayout {_panelSettings->create_layout<grid_layout>(size_i {40, 40})};
        auto const& config {locate_service<data::config_file>()};

        // resolution
        {
            auto const displayModes {locate_service<gfx::render_system>().get_displays()};
            auto       ddlRes {panelLayout->create_widget<drop_down_list>({6, 1, 6, 4}, "ddlResolution")};
            ddlRes->ZOrder = 1;
            for (auto const& dm : displayModes.at(0).Modes) {
                ddlRes->add_item(std::format("{}x{}", dm.Size.Width, dm.Size.Height));
            }

            auto const res {config[Cfg::Video::Name][Cfg::Video::resolution].as<size_i>()};
            ddlRes->select_item(std::format("{}x{}", res.Width, res.Height));

            auto lbl {panelLayout->create_widget<label>({1, 2, 4, 2}, "lblResolution")};
            lbl->Label = "Resolution";
        }

        // fullscreen
        {
            auto chkFullScreen {panelLayout->create_widget<checkbox>({6, 5, 3, 4}, "chkFullScreen")};
            chkFullScreen->Checked = config[Cfg::Video::Name][Cfg::Video::fullscreen].as<bool>();

            auto lbl {panelLayout->create_widget<label>({1, 6, 4, 2}, "lblFullScreen")};
            lbl->Label = "Fullscreen";
        }

        // vsync
        {
            auto chkFullScreen {panelLayout->create_widget<checkbox>({6, 9, 3, 4}, "chkVSync")};
            chkFullScreen->Checked = config[Cfg::Video::Name][Cfg::Video::vsync].as<bool>();

            auto lbl {panelLayout->create_widget<label>({1, 10, 4, 2}, "lblVSync")};
            lbl->Label = "VSync";
        }

        BtnApplySettings          = panelLayout->create_widget<button>({34, 36, 4, 3}, "btnApply");
        BtnApplySettings->Icon    = resGrp.get<gfx::texture>("apply");
        BtnApplySettings->Tooltip = _tooltip;
    }
}

void form_menu::create_section_themes(std::vector<std::string> const& colorThemes)
{
    // Themes
    auto panelThemes {create_container<panel>(dock_style::Left, TabThemesName)};
    panelThemes->ZOrder = 3;
    panelThemes->Flex   = {0_pct, 0_pct};
    {
        auto panelLayout {panelThemes->create_layout<dock_layout>()};
        auto lbxThemes {panelLayout->create_widget<list_box>(dock_style::Fill, "lbxThemes")};
        for (auto const& colorTheme : colorThemes) { lbxThemes->add_item(colorTheme); }
        lbxThemes->SelectedItemIndex.Changed.connect([&, lb = lbxThemes.get()](auto val) {
            if (val != -1) { SelectedTheme = lb->get_selected_item(); }
        });
        lbxThemes->DoubleClick.connect([&] { hide(); });
        SelectedTheme.Changed.connect([lb = lbxThemes.get()](auto const& val) { lb->select_item(val); });
    }
}

void form_menu::create_section_cardset(std::vector<std::string> const& cardSets)
{
    // Cardsets
    auto panelCardsets {create_container<panel>(dock_style::Left, TabCardsetsName)};
    panelCardsets->ZOrder = 2;
    panelCardsets->Flex   = {0_pct, 0_pct};
    {
        auto panelLayout {panelCardsets->create_layout<dock_layout>()};
        auto lbxCardsets {panelLayout->create_widget<list_box>(dock_style::Fill, "lbxCardsets")};
        for (auto const& cardSet : cardSets) { lbxCardsets->add_item(cardSet); }
        lbxCardsets->SelectedItemIndex.Changed.connect([&, lb = lbxCardsets.get()](auto val) {
            if (val != -1) { SelectedCardset = lb->get_selected_item(); }
        });
        lbxCardsets->DoubleClick.connect([&] { hide(); });
        SelectedCardset.Changed.connect([lb = lbxCardsets.get()](auto const& val) { lb->select_item(val); });
    }
}

void form_menu::create_menubar(assets::group& resGrp)
{
    auto menu {create_container<panel>(dock_style::Fill, MenuName)};
    auto menuLayout {menu->create_layout<grid_layout>(size_i {7, 40})};

    auto const enableContainer {[](form const* f, string const& enable) {
        for (auto const& w : f->get_widgets()) {
            if (w->get_name() != MenuName) {
                w->Flex = w->get_name() == enable ? dimensions {85_pct, 100_pct} : dimensions {0_pct, 0_pct};
            }
        }
    }};

    rect_i     btnRect {1, 1, 1, 2};
    auto const create {[&](string const& text) {
        auto btn {menuLayout->create_widget<radio_button>(btnRect, "btn" + text)};

        auto lbl {menuLayout->create_widget<label>({btnRect.Width + 2, btnRect.Y, btnRect.Width + 2, btnRect.Height}, "lblBtn" + text)};
        lbl->Label = text;
        lbl->For   = btn;

        btnRect.Y += btnRect.Height + 1;
        return btn;
    }};

    {
        auto btn {create("Games")};
        btn->Checked = true;
        btn->Click.connect([enableContainer](auto const& ev) { enableContainer(ev.Sender->get_form(), TabGamesName); });
    }
    {
        auto btn {create("Settings")};
        btn->Click.connect([enableContainer](auto const& ev) { enableContainer(ev.Sender->get_form(), TabSettingsName); });
    }
    {
        auto btn {create("Themes")};
        btn->Click.connect([enableContainer](auto const& ev) { enableContainer(ev.Sender->get_form(), TabThemesName); });
    }
    {
        auto btn {create("Cardsets")};
        btn->Click.connect([enableContainer](auto const& ev) { enableContainer(ev.Sender->get_form(), TabCardsetsName); });
    }

    auto btnBack {menuLayout->create_widget<button>({1, 35, 5, 3}, "btnBack")};
    btnBack->Icon = resGrp.get<gfx::texture>("back");
    btnBack->Click.connect([&](auto&) { hide(); });
    btnBack->Tooltip = _tooltip;
}
}
