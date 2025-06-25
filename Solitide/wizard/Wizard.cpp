// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Wizard.hpp"

namespace solitaire {

using namespace tcob::literals;

form_wizard::form_wizard(gfx::window& window, assets::group& resGrp)
    : form {{"Wizard", window.bounds()}}
{
    auto  main {create_container<panel>(dock_style::Fill, "Wizard")};
    auto& mainLayout {main->create_layout<grid_layout>(size_i {80, 80})};

    auto const createLabel {[](auto&& layout, std::string const& text) {
        auto lbl {layout.template create_widget<label>("lbl" + text)};
        lbl->Class = "label-margin";
        lbl->Label = text;
    }};

    {
        auto  pnl {mainLayout.create_widget<panel>({0, 0, 30, 10}, "GameInfo")};
        auto& layout {pnl->create_layout<box_layout>(size_i {2, 2})};

        createLabel(layout, "Name");
        auto tbxName {layout.create_widget<text_box>("Name")};

        createLabel(layout, "Deck Count");
        auto spnDecks {layout.create_widget<spinner>("Decks")};
        spnDecks->Flex  = {30_pct, 100_pct};
        spnDecks->Min   = 1;
        spnDecks->Max   = 4;
        spnDecks->Step  = 1;
        spnDecks->Value = 1;
    }

    {
        auto const createPileSize {[&](auto&& layout, std::string const& name) {
            createLabel(layout, "# of Piles");
            auto spn {layout.template create_widget<spinner>(name + "Size")};
            spn->Flex  = {30_pct, 100_pct};
            spn->Min   = 0;
            spn->Max   = 20;
            spn->Step  = 1;
            spn->Value = 0;
        }};
        auto const createCardCount {[&](auto&& layout, std::string const& name) {
            createLabel(layout, "# of Cards");
            auto spn {layout.template create_widget<spinner>(name + "CardCount")};
            spn->Flex  = {30_pct, 100_pct};
            spn->Min   = 0;
            spn->Max   = 208;
            spn->Step  = 1;
            spn->Value = 0;
        }};
        auto const createCardFace {[&](auto&& layout, std::string const& name) {
            createLabel(layout, "Orientation");
            auto face {layout.template create_widget<drop_down_list>(name + "Orientation")};
            face->Items.mutate([](auto& items) {
                items.push_back({"Face Up"});
                items.push_back({"Top Card Face Up"});
                items.push_back({"Alternate - First Face Up"});
                items.push_back({"Alternate - First Face Down"});
                items.push_back({"Face Down"});
            });

            face->SelectedItemIndex = 0;
            face->Class             = "drop_down_list_wizard";
        }};
        auto const createPileLayout {[&](auto&& layout, std::string const& name) {
            createLabel(layout, "Layout");
            auto cyb {layout.template create_widget<cycle_button>(name + "Layout")};
            cyb->Items.mutate([](auto& items) {
                items.push_back({"Squared"});
                items.push_back({"Column"});
                items.push_back({"Row"});
                items.push_back({"Fan"});
            });
            cyb->SelectedItemIndex = 0;
        }};
        struct rule {
            std::string base;
            std::string build;
            std::string move;
        };
        auto const createPileRule {
            [&](auto&& layout, std::string const& name, rule const& select) {
                createLabel(layout, "Base");
                auto base {layout.template create_widget<drop_down_list>(name + "Base")};
                base->Items.mutate([](auto& items) {
                    items.push_back({"Ace"});
                    items.push_back({"King"});
                    items.push_back({"Any"});
                    items.push_back({"None"});
                });
                base->select_item(select.base);
                base->Class = "drop_down_list_wizard";

                createLabel(layout, "Build");
                auto build {layout.template create_widget<drop_down_list>(name + "Build")};
                build->Items.mutate([](auto& items) {
                    items.push_back({"Any"});
                    items.push_back({"InRank"});
                    items.push_back({"InRankOrDownByRank"});
                    items.push_back({"RankPack"});
                    items.push_back({"UpOrDownByRank"});
                    items.push_back({"DownByRank"});
                    items.push_back({"UpByRank"});
                    items.push_back({"UpOrDownAnyButOwnSuit"});
                    items.push_back({"DownAnyButOwnSuit"});
                    items.push_back({"UpAnyButOwnSuit"});
                    items.push_back({"UpOrDownInSuit"});
                    items.push_back({"DownInSuit"});
                    items.push_back({"UpInSuit"});
                    items.push_back({"UpOrDownInColor"});
                    items.push_back({"DownInColor"});
                    items.push_back({"UpInColor"});
                    items.push_back({"UpOrDownAlternateColors"});
                    items.push_back({"DownAlternateColors"});
                    items.push_back({"UpAlternateColors"});
                    items.push_back({"None"});
                });
                build->select_item(select.build);
                build->Class = "drop_down_list_wizard";

                createLabel(layout, "Move");
                auto move {layout.template create_widget<drop_down_list>(name + "Move")};
                move->Items.mutate([](auto& items) {
                    items.push_back({"Top"});
                    items.push_back({"FaceUp"});
                    items.push_back({"InSeq"});
                    items.push_back({"InSeqInSuit"});
                    items.push_back({"None"});
                });
                move->select_item(select.move);
                move->Class = "drop_down_list_wizard";
            }};

        auto         tab {mainLayout.create_widget<tab_container>({0, 20, 30, 60}, "Piles")};
        size_i const boxSize {2, 12};
        {
            auto  pnl {tab->create_tab<panel>("Stock", {.Text = "Stock", .Icon = {}, .UserData = {}})};
            auto& layout {pnl->create_layout<box_layout>(boxSize)};

            createLabel(layout, "Deal");
            auto cybStockTarget {layout.create_widget<cycle_button>("StockTarget")};
            cybStockTarget->Items.mutate([](auto& items) {
                items.push_back({"To Waste"});
                items.push_back({"To Waste by Threes"});
                items.push_back({"To Tableau"});
            });
            cybStockTarget->select_item(cybStockTarget->get_item_at(0).Text);

            createLabel(layout, "Redeals");
            auto spnRedeals {layout.create_widget<spinner>("Redeals")};
            spnRedeals->Flex  = {.Width = 30_pct, .Height = 100_pct};
            spnRedeals->Min   = -1;
            spnRedeals->Max   = 5;
            spnRedeals->Step  = 1;
            spnRedeals->Value = 1;
        }
        {
            auto  pnl {tab->create_tab<panel>("Waste", {"Waste", {}, {}})};
            auto& layout {pnl->create_layout<box_layout>(boxSize)};

            createPileSize(layout, "Waste");
            createPileLayout(layout, "Waste");
        }
        {
            auto  pnl {tab->create_tab<panel>("Reserve", {"Reserve", {}, {}})};
            auto& layout {pnl->create_layout<box_layout>(boxSize)};

            createPileSize(layout, "Reserve");
            createCardCount(layout, "Reserve");
            createCardFace(layout, "Reserve");
            createPileLayout(layout, "Reserve");
        }
        {
            auto  pnl {tab->create_tab<panel>("FreeCell", {"FreeCell", {}, {}})};
            auto& layout {pnl->create_layout<box_layout>(boxSize)};

            createPileSize(layout, "FreeCell");
            createCardCount(layout, "FreeCell");
            createPileLayout(layout, "FreeCell");
            createPileRule(layout, "FreeCell", {.base = "Any", .build = "None", .move = "Top"});
        }
        {
            auto  pnl {tab->create_tab<panel>("Foundation", {.Text = "Foundation", .Icon = {}, .UserData = {}})};
            auto& layout {pnl->create_layout<box_layout>(boxSize)};

            createPileLayout(layout, "Foundation");
            createPileRule(layout, "Foundation", {.base = "Ace", .build = "UpInSuit", .move = "Top"});
        }
        {
            auto  pnl {tab->create_tab<panel>("Tableau", {.Text = "Tableau", .Icon = {}, .UserData = {}})};
            auto& layout {pnl->create_layout<box_layout>(boxSize)};

            createPileSize(layout, "Tableau");
            createCardCount(layout, "Tableau");
            createCardFace(layout, "Tableau");
            createPileLayout(layout, "Tableau");
            createPileRule(layout, "Tableau", {.base = "Any", .build = "Any", .move = "Top"});
        }
    }

    _lbxLog        = mainLayout.create_widget<list_box>({32, 2, 36, 30}, "Log");
    _lbxLog->Class = "list_box_log";
    _lbxLog->HoveredItemIndex.Changed.connect([lb = _lbxLog.get()] {
        lb->HoveredItemIndex = -1;
    });

    BtnGenerate       = mainLayout.create_widget<button>({60, 72, 9, 5}, "btnGenerate");
    BtnGenerate->Icon = {.Texture = resGrp.get<gfx::texture>("apply")};

    BtnBack       = mainLayout.create_widget<button>({70, 72, 9, 5}, "btnBack");
    BtnBack->Icon = {.Texture = resGrp.get<gfx::texture>("back")};
}

void form_wizard::set_log_messages(std::vector<std::string> const& messages)
{
    std::vector<item> items;
    items.reserve(messages.size());
    for (auto const& m : messages) { items.push_back({.Text = m}); }
    _lbxLog->Items = items;
}
}
