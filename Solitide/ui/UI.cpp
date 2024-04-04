// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "UI.hpp"

namespace solitaire {

form_controls::form_controls(gfx::window* window, rect_f bounds)
    : form {"MainMenu", window, bounds}
{
    using namespace tcob::literals;

    auto mainPanel {create_container<glass>(dock_style::Fill, "main")};
    mainPanel->Class = "panel-transparent";
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
    overlayPanel->Class = "panel-transparent";
    overlayPanel->disable();
    auto overlayPanelLayout {overlayPanel->create_layout<dock_layout>()};

    Canvas = overlayPanelLayout->create_widget<canvas_widget>(dock_style::Fill, "canvas");
}

////////////////////////////////////////////////////////////

form_menu::form_menu(gfx::window* window, rect_f bounds,
                     std::vector<games::game_info> const& games, std::vector<std::string> const& colorThemes, std::vector<std::string> const& cardSets)
    : form {"Games", window, bounds}
{
    using namespace tcob::literals;

    // Games
    auto tabGames {create_container<tab_container>(dock_style::Left, "tabGames")};
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

    // Themes
    auto panelThemes {create_container<panel>(dock_style::Left, "panelThemes")};
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
    auto panelCardsets {create_container<panel>(dock_style::Left, "panelCardsets")};
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

    // menu
    auto menu {create_container<panel>(dock_style::Fill, "menu")};
    auto menuLayout {menu->create_layout<grid_layout>(size_i {6, 20})};

    auto const enableContainer {[](form const* f, string const& enable) {
        for (auto const& w : f->get_widgets()) {
            if (w->get_name() != "menu") {
                w->Flex = w->get_name() == enable ? dimensions {85_pct, 100_pct} : dimensions {0_pct, 0_pct};
            }
        }
    }};
    auto       btnGames {menuLayout->create_widget<radio_button>({0, 1, 3, 2}, "btnGames")};
    auto       btnGamesL {menuLayout->create_widget<label>({3, 1, 2, 2}, "btnGamesL")};
    btnGames->Checked = true;
    btnGames->Click.connect([enableContainer](auto const& ev) { enableContainer(ev.Sender->get_form(), "tabGames"); });
    btnGamesL->Label = "Games";
    btnGamesL->For   = btnGames;

    auto btnThemes {menuLayout->create_widget<radio_button>({0, 4, 3, 2}, "btnThemes")};
    auto btnThemesL {menuLayout->create_widget<label>({3, 4, 2, 2}, "btnThemesL")};
    btnThemes->Click.connect([enableContainer](auto const& ev) { enableContainer(ev.Sender->get_form(), "panelThemes"); });
    btnThemesL->Label = "Themes";
    btnThemesL->For   = btnThemes;

    auto btnCardsets {menuLayout->create_widget<radio_button>({0, 7, 3, 2}, "btnCardsets")};
    auto btnCardsetsL {menuLayout->create_widget<label>({3, 7, 2, 2}, "btnCardsetsL")};
    btnCardsets->Click.connect([enableContainer](auto const& ev) { enableContainer(ev.Sender->get_form(), "panelCardsets"); });
    btnCardsetsL->Label = "Cardsets";
    btnCardsetsL->For   = btnCardsets;

    auto btnBack {menuLayout->create_widget<button>({1, 18, 3, 2}, "btnBack")};
    btnBack->Label = "Back";
    btnBack->Click.connect([&](auto&) { hide(); });
}

}
