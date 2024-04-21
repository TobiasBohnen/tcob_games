// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "UI.hpp"

namespace solitaire {

using namespace tcob::literals;

form_controls::form_controls(gfx::window* window)
    : form {"MainMenu", window}
{
    auto mainPanel {create_container<glass>(dock_style::Fill, "main")};
    auto mainPanelLayout {mainPanel->create_layout<dock_layout>()};

    // menu
    {
        auto menuPanel {mainPanelLayout->create_widget<panel>(dock_style::Top, "menu")};
        menuPanel->Flex = {100_pct, 10_pct};
        auto menuPanelLayout {menuPanel->create_layout<grid_layout>(size_i {20, 6})};

        BtnMenu        = menuPanelLayout->create_widget<button>({0, 0, 3, 6}, "btnGames");
        BtnMenu->Label = "Menu";

        BtnNewGame        = menuPanelLayout->create_widget<button>({4, 0, 2, 6}, "BtnNewGame");
        BtnNewGame->Label = "New Game";

        BtnHint        = menuPanelLayout->create_widget<button>({13, 0, 2, 6}, "btnHint");
        BtnHint->Label = "Hint";

        BtnUndo        = menuPanelLayout->create_widget<button>({15, 0, 2, 6}, "btnUndo");
        BtnUndo->Label = "Undo";

        BtnQuit        = menuPanelLayout->create_widget<button>({18, 0, 2, 6}, "btnQuit");
        BtnQuit->Label = "Quit";
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

        LblTurn      = create({18, 3, 1, 3});
        LblTurnLabel = create({18, 0, 1, 3}, "Turn");

        LblTime      = create({19, 3, 1, 3});
        LblTimeLabel = create({19, 0, 1, 3}, "Time");
    }

    auto overlayPanel {mainPanelLayout->create_widget<glass>(dock_style::Fill, "overlay")};
    overlayPanel->disable();
    auto overlayPanelLayout {overlayPanel->create_layout<dock_layout>()};

    Canvas = overlayPanelLayout->create_widget<canvas_widget>(dock_style::Fill, "canvas");
}

////////////////////////////////////////////////////////////

static string const TabGamesName {"tabGames"};
static string const TabSettingsName {"tabSettings"};
static string const TabThemesName {"tabThemes"};
static string const TabCardsetsName {"tabCardsets"};

form_menu::form_menu(gfx::window*                         window,
                     std::vector<games::game_info> const& games,
                     std::vector<std::string> const&      colorThemes,
                     std::vector<std::string> const&      cardSets)
    : form {"Games", window}
{
    // Games
    auto tabGames {create_container<tab_container>(dock_style::Left, TabGamesName)};
    tabGames->Flex = {85_pct, 100_pct};

    auto createListBox {[&](std::shared_ptr<dock_layout>& tabPanelLayout, std::string const& name, auto&& pred) -> std::shared_ptr<list_box> {
        auto listBox {tabPanelLayout->create_widget<list_box>(dock_style::Fill, "lbxGames" + name)};
        listBox->Class = "list_box_games";
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
        listBox->DoubleClick.connect([&] { hide(); });
        return listBox;
    }};

    // By Name
    {
        auto tabPanel {tabGames->create_tab<panel>("byName", "By Name")};
        auto tabPanelLayout {tabPanel->create_layout<dock_layout>()};
        LbxGamesByName = createListBox(tabPanelLayout, "0", [](auto const&) { return true; });
    }
    // By Family
    {
        auto tabContainer {tabGames->create_tab<tab_container>("byFamily", "By Family")};

        auto createTab {[&](games::family family, std::string const& name) {
            auto tabPanel {tabContainer->create_tab<panel>(name)};
            auto tabPanelLayout {tabPanel->create_layout<dock_layout>()};
            createListBox(tabPanelLayout, name, [family](auto const& game) { return game.Family == family; });
        }};
        createTab(games::family::BakersDozen, "Baker's Dozen");
        createTab(games::family::BeleagueredCastle, "Beleaguered Castle");
        createTab(games::family::Canfield, "Canfield");
        createTab(games::family::Fan, "Fan");
        createTab(games::family::FortyThieves, "Forty Thieves");
        createTab(games::family::FreeCell, "FreeCell");
        createTab(games::family::Golf, "Golf");
        createTab(games::family::Gypsy, "Gypsy");
        createTab(games::family::Klondike, "Klondike");
        createTab(games::family::Montana, "Montana");
        createTab(games::family::Raglan, "Raglan");
        createTab(games::family::Spider, "Spider");
        createTab(games::family::Yukon, "Yukon");
        createTab(games::family::Other, "Other");
    }
    // By Deck Count
    {
        auto tabContainer {tabGames->create_tab<tab_container>("byDeckCount", "By Deck Count")};

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

    // Setting
    PanelSettings       = create_container<panel>(dock_style::Left, TabSettingsName);
    PanelSettings->Flex = {0_pct, 0_pct};
    {
        auto        panelLayout {PanelSettings->create_layout<grid_layout>(size_i {40, 40})};
        auto const& config {locate_service<data::config_file>()};

        // resolution
        {
            auto const& renderSystem {locate_service<gfx::render_system>()};
            auto const  displayModes {renderSystem.get_displays()};
            auto        ddlRes {panelLayout->create_widget<drop_down_list>({6, 1, 6, 3}, "ddlResolution")}; // TODO: change to drop-down-list
            for (auto const& dm : displayModes.at(0).Modes) {
                ddlRes->add_item(std::format("{}x{}", dm.Size.Width, dm.Size.Height));
            }
            auto const res {config[Cfg::Video::Name][Cfg::Video::resolution].as<size_i>()};
            ddlRes->select_item(std::format("{}x{}", res.Width, res.Height));
            auto lbl {panelLayout->create_widget<label>({1, 1, 4, 3}, "lblResolution")};
            lbl->Label     = "Resolution";
            ddlRes->ZOrder = 1;
        }

        // fullscreen
        {
            auto chkFullScreen {panelLayout->create_widget<checkbox>({6, 5, 6, 3}, "chkFullScreen")};
            chkFullScreen->Checked = config[Cfg::Video::Name][Cfg::Video::fullscreen].as<bool>();
            auto lbl {panelLayout->create_widget<label>({1, 5, 4, 3}, "lblFullScreen")};
            lbl->Label = "Fullscreen";
        }

        // vsync
        {
            auto chkFullScreen {panelLayout->create_widget<checkbox>({6, 9, 6, 3}, "chkVSync")};
            chkFullScreen->Checked = config[Cfg::Video::Name][Cfg::Video::vsync].as<bool>();
            auto lbl {panelLayout->create_widget<label>({1, 9, 4, 3}, "lblVSync")};
            lbl->Label = "VSync";
        }

        BtnApplySettings        = panelLayout->create_widget<button>({36, 36, 5, 3}, "btnApply");
        BtnApplySettings->Label = "Apply";
    }

    // Themes
    auto panelThemes {create_container<panel>(dock_style::Left, TabThemesName)};
    panelThemes->Flex = {0_pct, 0_pct};
    {
        auto panelLayout {panelThemes->create_layout<dock_layout>()};
        LbxThemes        = panelLayout->create_widget<list_box>(dock_style::Fill, "lbxThemes");
        LbxThemes->Class = "list_box_games";
        for (auto const& colorTheme : colorThemes) { LbxThemes->add_item(colorTheme); }
        LbxThemes->SelectedItemIndex.Changed.connect([&, lb = LbxThemes.get()](auto val) {
            if (val != -1) { SelectedTheme = lb->get_selected_item(); }
        });
        LbxThemes->DoubleClick.connect([&] { hide(); });
    }

    // Cardsets
    auto panelCardsets {create_container<panel>(dock_style::Left, TabCardsetsName)};
    panelCardsets->Flex = {0_pct, 0_pct};
    {
        auto panelLayout {panelCardsets->create_layout<dock_layout>()};
        LbxCardsets        = panelLayout->create_widget<list_box>(dock_style::Fill, "lbxCardsets");
        LbxCardsets->Class = "list_box_games";
        for (auto const& cardSet : cardSets) { LbxCardsets->add_item(cardSet); }
        LbxCardsets->SelectedItemIndex.Changed.connect([&, lb = LbxCardsets.get()](auto val) {
            if (val != -1) { SelectedCardset = lb->get_selected_item(); }
        });
        LbxCardsets->DoubleClick.connect([&] { hide(); });
    }

    create_menubar();
}

void form_menu::create_menubar()
{
    static string const MenuName {"menu"};

    auto menu {create_container<panel>(dock_style::Fill, MenuName)};
    auto menuLayout {menu->create_layout<grid_layout>(size_i {6, 20})};

    auto const enableContainer {[](form const* f, string const& enable) {
        for (auto const& w : f->get_widgets()) {
            if (w->get_name() != MenuName) {
                w->Flex = w->get_name() == enable ? dimensions {85_pct, 100_pct} : dimensions {0_pct, 0_pct};
            }
        }
    }};

    rect_i btnRect {0, 1, 3, 2};

    auto const createMenuButton {[&](string const& text) {
        auto btn {menuLayout->create_widget<radio_button>(btnRect, "btn" + text)};
        auto lbl {menuLayout->create_widget<label>({btnRect.Width, btnRect.Y, btnRect.Width - 1, btnRect.Height}, "lblBtn" + text)};

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

    auto btnBack {menuLayout->create_widget<button>({1, 18, 4, 2}, "btnBack")};
    btnBack->Label = "Back";
    btnBack->Click.connect([&](auto&) { hide(); });
}

}
