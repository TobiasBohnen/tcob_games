// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "UI.hpp"

namespace solitaire {

form_controls::form_controls(gfx::window* window, rect_f bounds)
    : form {"MainMenu", window, bounds}
{
    auto mainPanel {create_container<panel>(dock_style::Fill, "main")};

    auto mainPanelLayout {mainPanel->create_layout<grid_layout>(size_i {20, 6})};

    BtnMenu        = mainPanelLayout->create_widget<button>({0, 0, 3, 6}, "btnGames");
    BtnMenu->Label = "Menu";

    LblPile             = mainPanelLayout->create_widget<label>({4, 0, 2, 6}, "lblInfo0");
    LblPile->Class      = "label-small";
    LblRule             = mainPanelLayout->create_widget<label>({6, 0, 4, 6}, "lblInfo1");
    LblRule->Class      = "label-small";
    LblCardCount        = mainPanelLayout->create_widget<label>({10, 0, 1, 6}, "lblInfo2");
    LblCardCount->Class = "label-small";

    BtnUndo        = mainPanelLayout->create_widget<button>({12, 0, 2, 6}, "btnUndo");
    BtnUndo->Label = "Undo";

    BtnStart        = mainPanelLayout->create_widget<button>({14, 0, 2, 6}, "btnPlay");
    BtnStart->Label = "Restart";

    BtnQuit        = mainPanelLayout->create_widget<button>({18, 0, 2, 6}, "btnQuit");
    BtnQuit->Label = "Quit";
}

////////////////////////////////////////////////////////////

form_menu::form_menu(gfx::window* window, rect_f bounds,
                     std::vector<games::game_info> const& games, std::vector<std::string> const& colorThemes)
    : form {"Games", window, bounds}
{
    using namespace tcob::literals;

    // Games
    auto tabGames {create_container<tab_container>(dock_style::Left, "tabGames")};
    tabGames->Flex = {85_pct, 100_pct};

    auto createListBox {[&](std::shared_ptr<dock_layout>& tabPanelLayout, std::string const& name, auto&& pred) {
        auto listBox {tabPanelLayout->create_widget<list_box>(dock_style::Fill, "lbxGames" + name)};
        listBox->Class = "list_box_games";
        bool retValue {false};
        for (auto const& game : games) {
            if (pred(game)) {
                listBox->add_item(game.Name);
                retValue = true;
            }
        }
        listBox->SelectedItemIndex.Changed.connect([&, lb = listBox.get()](auto val) {
            if (val != -1) { SelectedGame = lb->get_selected_item(); }
        });
        listBox->DoubleClick.connect([&] { hide(); });
        return retValue;
    }};

    // By Name
    {
        auto tabPanel {tabGames->create_tab<panel>("byName", "By Name")};
        auto tabPanelLayout {tabPanel->create_layout<dock_layout>()};
        createListBox(tabPanelLayout, "0", [](auto const&) { return true; });
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
        createTab(games::family::Spider, "Spider");
        createTab(games::family::Yukon, "Yukon");
        createTab(games::family::Other, "Other");
    }
    // By Type
    {
        auto tabContainer {tabGames->create_tab<tab_container>("byType", "By Type")};
        auto createTab {[&](std::shared_ptr<tab_container>& parent, games::type type, std::string const& name) {
            auto tabPanel {parent->create_tab<panel>(name)};
            auto tabPanelLayout {tabPanel->create_layout<dock_layout>()};

            if (!createListBox(tabPanelLayout, name, [type](auto const& game) { return game.Type == type; })) {
                parent->remove_tab(tabPanel.get());
            }
        }};
        {
            auto tabGroup {tabContainer->create_tab<tab_container>("Closed")};
            createTab(tabGroup, games::type::SimpleBuilder, "Simple Builder");
            createTab(tabGroup, games::type::ReservedBuilder, "Reserved Builder");
            createTab(tabGroup, games::type::SimplePacker, "Simple Packer");
            createTab(tabGroup, games::type::ReservedPacker, "Reserved Packer");
            createTab(tabGroup, games::type::ClosedNonBuilder, "Closed Non-Builder");
        }
        {
            auto tabGroup {tabContainer->create_tab<tab_container>("Half-Open")};
            createTab(tabGroup, games::type::Builder, "Builder");
            createTab(tabGroup, games::type::Blockade, "Blockade");
            createTab(tabGroup, games::type::Planner, "Planner");
            createTab(tabGroup, games::type::Packer, "Packer");
            createTab(tabGroup, games::type::Spider, "Spider");
        }
        {
            auto tabGroup {tabContainer->create_tab<tab_container>("Open")};
            createTab(tabGroup, games::type::OpenBuilder, "Open Builder");
            createTab(tabGroup, games::type::OpenPacker, "Open Packer");
            createTab(tabGroup, games::type::OpenNonBuilder, "Open Non-Builder");
        }
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

        auto tabPanel {tabContainer->create_tab<panel>(">= 4")};
        auto tabPanelLayout {tabPanel->create_layout<dock_layout>()};
        if (!createListBox(tabPanelLayout, "4+", [](auto const& game) { return game.DeckCount >= 4; })) {
            tabContainer->remove_tab(tabPanel.get());
        }
    }

    // Themes
    auto panelThemes {create_container<panel>(dock_style::Left, "panelThemes")};
    panelThemes->Flex = {0_pct, 0_pct};
    {
        auto panelLayout {panelThemes->create_layout<dock_layout>()};
        auto listBox {panelLayout->create_widget<list_box>(dock_style::Fill, "lbxThemes")};
        listBox->Class = "list_box_games";
        for (auto const& colorTheme : colorThemes) {
            listBox->add_item(colorTheme);
        }
        listBox->SelectedItemIndex.Changed.connect([&, lb = listBox.get()](auto val) {
            if (val != -1) { SelectedTheme = lb->get_selected_item(); }
        });
        listBox->DoubleClick.connect([&] { hide(); });
    }

    // menu
    auto menu {create_container<panel>(dock_style::Fill, "menu")};
    auto menuLayout {menu->create_layout<grid_layout>(size_i {5, 20})};

    auto btnGames {menuLayout->create_widget<button>({1, 1, 3, 2}, "btnGames")};
    btnGames->Label = "Games";
    btnGames->Click.connect([tabT = panelThemes.get(), tabG = tabGames.get()](auto&) {
        tabT->Flex = {0_pct, 0_pct};
        tabG->Flex = {85_pct, 100_pct};
    });

    auto btnThemes {menuLayout->create_widget<button>({1, 4, 3, 2}, "btnThemes")};
    btnThemes->Label = "Themes";
    btnThemes->Click.connect([tabT = panelThemes.get(), tabG = tabGames.get()](auto&) {
        tabG->Flex = {0_pct, 0_pct};
        tabT->Flex = {85_pct, 100_pct};
    });

    auto btnBack {menuLayout->create_widget<button>({1, 18, 3, 2}, "btnBack")};
    btnBack->Label = "Back";
    btnBack->Click.connect([&](auto&) { hide(); });
}

}
