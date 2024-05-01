// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "UI.hpp"

#include "Games.hpp"
#include "StartScene.hpp"

namespace solitaire {

using namespace tcob::literals;

form_controls::form_controls(gfx::window* window, assets::group& resGrp)
    : form {"MainMenu", window}
{
    auto mainPanel {create_container<glass>(dock_style::Fill, "main")};
    auto mainPanelLayout {mainPanel->create_layout<dock_layout>()};

    // menu
    {
        auto menuPanel {mainPanelLayout->create_widget<panel>(dock_style::Top, "menu")};
        menuPanel->Flex = {100_pct, 5_pct};
        auto menuPanelLayout {menuPanel->create_layout<grid_layout>(size_i {20, 1})};

        BtnMenu       = menuPanelLayout->create_widget<button>({0, 0, 1, 1}, "btnMenu");
        BtnMenu->Icon = resGrp.get<gfx::texture>("burger");

        BtnHint       = menuPanelLayout->create_widget<button>({14, 0, 1, 1}, "btnHint");
        BtnHint->Icon = resGrp.get<gfx::texture>("hint");

        BtnNewGame       = menuPanelLayout->create_widget<button>({15, 0, 1, 1}, "BtnNewGame");
        BtnNewGame->Icon = resGrp.get<gfx::texture>("newgame");

        BtnUndo       = menuPanelLayout->create_widget<button>({16, 0, 1, 1}, "btnUndo");
        BtnUndo->Icon = resGrp.get<gfx::texture>("undo");

        BtnQuit       = menuPanelLayout->create_widget<button>({19, 0, 1, 1}, "btnQuit");
        BtnQuit->Icon = resGrp.get<gfx::texture>("exit");
    }

    // status
    {
        auto statusPanel {mainPanelLayout->create_widget<panel>(dock_style::Bottom, "status")};
        statusPanel->Flex = {100_pct, 10_pct};
        auto statusPanelLayout {statusPanel->create_layout<grid_layout>(size_i {20, 6})};

        i32  i {0};
        auto create {[&](rect_i const& rect, string const& text = "") {
            auto l {statusPanelLayout->create_widget<label>(rect, "lblInfo" + std::to_string(i))};
            l->Class = "label-small";
            l->Label = text;
            return l;
        }};

        LblGameName = create({0, 0, 2, 6});

        LblPile           = create({2, 3, 2, 3});
        LblPileLabel      = create({2, 0, 2, 3}, "Pile");
        LblCardCount      = create({4, 3, 1, 3});
        LblCardCountLabel = create({4, 0, 1, 3}, "Cards");

        LblBase             = create({6, 3, 3, 3});
        LblBaseLabel        = create({6, 0, 3, 3});
        LblDescription      = create({9, 3, 3, 3});
        LblDescriptionLabel = create({9, 0, 3, 3});
        LblMove             = create({12, 3, 3, 3});
        LblMoveLabel        = create({12, 0, 3, 3});

        LblTurns      = create({17, 3, 1, 3});
        LblTurnsLabel = create({17, 0, 1, 3}, "Turns");

        LblScore      = create({18, 3, 1, 3});
        LblScoreLabel = create({18, 0, 1, 3}, "Score");

        LblTime      = create({19, 3, 1, 3});
        LblTimeLabel = create({19, 0, 1, 3}, "Time");
    }

    auto overlayPanel {mainPanelLayout->create_widget<glass>(dock_style::Fill, "overlay")};
    overlayPanel->disable();
    auto overlayPanelLayout {overlayPanel->create_layout<dock_layout>()};

    Canvas = overlayPanelLayout->create_widget<canvas_widget>(dock_style::Fill, "canvas");
}

////////////////////////////////////////////////////////////

static string const TabGamesName {"conGames"};
static string const TabSettingsName {"conSettings"};
static string const TabThemesName {"conThemes"};
static string const TabCardsetsName {"conCardsets"};

form_menu::form_menu(gfx::window* window, assets::group& resGrp, start_scene const& scene)
    : form {"Games", window}
{
    create_section_games(scene.get_games());
    create_section_settings(resGrp);
    create_section_themes(scene.get_themes());
    create_section_cardset(scene.get_cardsets());
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

void form_menu::create_section_games(std::vector<game_info> const& games)
{
    // Games
    auto panelGames  = create_container<panel>(dock_style::Left, TabGamesName);
    panelGames->Flex = {85_pct, 100_pct};
    auto panelLayout {panelGames->create_layout<dock_layout>()};

    {
        auto tabGames {panelLayout->create_widget<tab_container>(dock_style::Left, "tabGames")};
        tabGames->Flex    = {50_pct, 100_pct};
        tabGames->MaxTabs = 5;

        auto createListBox {[&](std::shared_ptr<dock_layout>& tabPanelLayout, std::string const& name, auto&& pred) -> std::shared_ptr<list_box> {
            auto listBox {tabPanelLayout->create_widget<list_box>(dock_style::Fill, "lbxGames" + name)};
            bool check {false};
            for (auto const& game : games) {
                if (pred(game)) {
                    listBox->add_item(game.Name);
                    check = true;
                }
            }
            if (!check) { return nullptr; }

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

            listBox->DoubleClick.connect([&] { hide(); });
            return listBox;
        }};

        // By Name
        {
            auto tabPanel {tabGames->create_tab<panel>("byName", "By Name")};
            auto tabPanelLayout {tabPanel->create_layout<dock_layout>()};
            _lbxGamesByName = createListBox(tabPanelLayout, "0", [](auto const&) { return true; });
        }
        // By Family
        {
            auto tabContainer {tabGames->create_tab<tab_container>("byFamily", "By Family")};
            tabContainer->MaxTabs = 5;

            auto createTab {[&](family family, std::string const& name) {
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
            createTab(family::Spider, "Spider");
            createTab(family::Yukon, "Yukon");
            createTab(family::Other, "Other");
        }
        // By Deck Count
        {
            auto tabContainer {tabGames->create_tab<tab_container>("byDeckCount", "By Deck Count")};
            tabContainer->MaxTabs = 5;

            auto createTab {[&](isize count, std::string const& name) {
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
                if (!createListBox(tabPanelLayout, "4+", [](auto const& game) { return game.DeckCount >= 4; })) {
                    tabContainer->remove_tab(tabPanel.get());
                }
            }
            {
                auto tabPanel {tabContainer->create_tab<panel>("stripped")};
                auto tabPanelLayout {tabPanel->create_layout<dock_layout>()};
                if (!createListBox(tabPanelLayout, "stripped", [](auto const& game) { return game.DeckRanks.size() < 13 || game.DeckSuits.size() < 4; })) {
                    tabContainer->remove_tab(tabPanel.get());
                }
            }
        }
    }
    {
        auto panelGameStats {panelLayout->create_widget<panel>(dock_style::Fill, "panelGameStats")};
        auto panelGameStatsLayout {panelGameStats->create_layout<grid_layout>(size_i {40, 40})};

        _gvWL        = panelGameStatsLayout->create_widget<grid_view>({0, 1, 20, 4}, "gvWinLose");
        _gvWL->Class = "grid_view2";
        _gvWL->set_columns({"Won", "Lost", "W/L"});
        _gvTT        = panelGameStatsLayout->create_widget<grid_view>({20, 1, 20, 4}, "gvBest");
        _gvTT->Class = "grid_view2";
        _gvTT->set_columns({"Highscore", "Least Turns", "Fastest Time"});

        _gvHistory = panelGameStatsLayout->create_widget<grid_view>({1, 6, 38, 25}, "gvHistory");
        _gvHistory->set_columns({"ID", "Score", "Turns", "Time", "Won"});
    }
}

void form_menu::create_section_settings(assets::group& resGrp)
{
    // Setting
    _panelSettings       = create_container<panel>(dock_style::Left, TabSettingsName);
    _panelSettings->Flex = {0_pct, 0_pct};
    {
        auto        panelLayout {_panelSettings->create_layout<grid_layout>(size_i {40, 40})};
        auto const& config {locate_service<data::config_file>()};

        // resolution
        {
            auto const& renderSystem {locate_service<gfx::render_system>()};
            auto const  displayModes {renderSystem.get_displays()};
            auto        ddlRes {panelLayout->create_widget<drop_down_list>({6, 1, 6, 4}, "ddlResolution")}; // TODO: change to drop-down-list
            for (auto const& dm : displayModes.at(0).Modes) {
                ddlRes->add_item(std::format("{}x{}", dm.Size.Width, dm.Size.Height));
            }
            auto const res {config[Cfg::Video::Name][Cfg::Video::resolution].as<size_i>()};
            ddlRes->select_item(std::format("{}x{}", res.Width, res.Height));
            auto lbl {panelLayout->create_widget<label>({1, 2, 4, 2}, "lblResolution")};
            lbl->Label     = "Resolution";
            ddlRes->ZOrder = 1;
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

        BtnApplySettings       = panelLayout->create_widget<button>({34, 36, 5, 2}, "btnApply");
        BtnApplySettings->Icon = resGrp.get<gfx::texture>("apply");
    }
}

void form_menu::create_section_themes(std::vector<std::string> const& colorThemes)
{
    // Themes
    auto panelThemes {create_container<panel>(dock_style::Left, TabThemesName)};
    panelThemes->Flex = {0_pct, 0_pct};
    {
        auto panelLayout {panelThemes->create_layout<dock_layout>()};
        _lbxThemes = panelLayout->create_widget<list_box>(dock_style::Fill, "lbxThemes");
        for (auto const& colorTheme : colorThemes) { _lbxThemes->add_item(colorTheme); }
        _lbxThemes->SelectedItemIndex.Changed.connect([&, lb = _lbxThemes.get()](auto val) {
            if (val != -1) { SelectedTheme = lb->get_selected_item(); }
        });
        _lbxThemes->DoubleClick.connect([&] { hide(); });
        SelectedTheme.Changed.connect([&](auto const& val) {
            _lbxThemes->select_item(val);
        });
    }
}

void form_menu::create_section_cardset(std::vector<std::string> const& cardSets)
{
    // Cardsets
    auto panelCardsets {create_container<panel>(dock_style::Left, TabCardsetsName)};
    panelCardsets->Flex = {0_pct, 0_pct};
    {
        auto panelLayout {panelCardsets->create_layout<dock_layout>()};
        _lbxCardsets = panelLayout->create_widget<list_box>(dock_style::Fill, "lbxCardsets");
        for (auto const& cardSet : cardSets) { _lbxCardsets->add_item(cardSet); }
        _lbxCardsets->SelectedItemIndex.Changed.connect([&, lb = _lbxCardsets.get()](auto val) {
            if (val != -1) { SelectedCardset = lb->get_selected_item(); }
        });
        _lbxCardsets->DoubleClick.connect([&] { hide(); });
        SelectedCardset.Changed.connect([&](auto const& val) {
            _lbxCardsets->select_item(val);
        });
    }
}

void form_menu::create_menubar(assets::group& resGrp)
{
    static string const MenuName {"menu"};

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
    auto const createMenuButton {[&](string const& text) {
        auto btn {menuLayout->create_widget<radio_button>(btnRect, "btn" + text)};

        auto lbl {menuLayout->create_widget<label>({btnRect.Width + 2, btnRect.Y, btnRect.Width + 2, btnRect.Height}, "lblBtn" + text)};
        lbl->Label = text;
        lbl->For   = btn;

        btnRect.Y += btnRect.Height + 1;
        return btn;
    }};

    {
        auto btn {createMenuButton("Games")};
        btn->Checked = true;
        btn->Click.connect([enableContainer](auto const& ev) { enableContainer(ev.Sender->get_form(), TabGamesName); });
    }
    {
        auto btn {createMenuButton("Settings")};
        btn->Click.connect([enableContainer](auto const& ev) { enableContainer(ev.Sender->get_form(), TabSettingsName); });
    }
    {
        auto btn {createMenuButton("Themes")};
        btn->Click.connect([enableContainer](auto const& ev) { enableContainer(ev.Sender->get_form(), TabThemesName); });
    }
    {
        auto btn {createMenuButton("Cardsets")};
        btn->Click.connect([enableContainer](auto const& ev) { enableContainer(ev.Sender->get_form(), TabCardsetsName); });
    }

    auto btnBack {menuLayout->create_widget<button>({2, 37, 3, 2}, "btnBack")};
    btnBack->Icon = resGrp.get<gfx::texture>("back");
    btnBack->Click.connect([&](auto&) { hide(); });
}
}
