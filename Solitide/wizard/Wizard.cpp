// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Wizard.hpp"

namespace solitaire {

using namespace tcob::literals;

form_wizard::form_wizard(gfx::window* window, assets::group& resGrp)
    : form {"Wizard", window}
{
    auto main {create_container<panel>(dock_style::Fill, "Wizard")};
    auto mainLayout {main->create_layout<grid_layout>(size_i {80, 80})};

    auto const createLabel {[](auto&& layout, std::string const& text, isize zorder = 0) {
        auto lbl {layout->template create_widget<label>("lbl" + text)};
        lbl->Class  = "label-margin";
        lbl->Label  = text;
        lbl->ZOrder = zorder;
    }};

    {
        auto pnl {mainLayout->create_widget<panel>({0, 0, 30, 10}, "GameInfo")};
        auto layout {pnl->create_layout<box_layout>(size_i {2, 2})};

        createLabel(layout, "Name");
        auto tbxName {layout->create_widget<text_box>("Name")};

        createLabel(layout, "Deck Count");
        auto spnDecks {layout->create_widget<spinner>("Decks")};
        spnDecks->Flex  = {30_pct, 100_pct};
        spnDecks->Min   = 1;
        spnDecks->Max   = 4;
        spnDecks->Step  = 1;
        spnDecks->Value = 1;
    }

    {
        auto const createPileSize {[&](auto&& layout, std::string const& name) {
            createLabel(layout, "# of Piles", 20);
            auto spn {layout->template create_widget<spinner>(name + "Size")};
            spn->Flex   = {30_pct, 100_pct};
            spn->Min    = 0;
            spn->Max    = 20;
            spn->Step   = 1;
            spn->Value  = 0;
            spn->ZOrder = 19;
        }};
        auto const createCardCount {[&](auto&& layout, std::string const& name) {
            createLabel(layout, "# of Cards", 18);
            auto spn {layout->template create_widget<spinner>(name + "CardCount")};
            spn->Flex   = {30_pct, 100_pct};
            spn->Min    = 0;
            spn->Max    = 208;
            spn->Step   = 1;
            spn->Value  = 0;
            spn->ZOrder = 17;
        }};
        auto const createCardFace {[&](auto&& layout, std::string const& name) {
            createLabel(layout, "Orientation", 16);
            auto face {layout->template create_widget<drop_down_list>(name + "Orientation")};
            face->add_item("Face Up");
            face->add_item("Top Card Face Up");
            face->add_item("Alternate - First Face Up");
            face->add_item("Alternate - First Face Down");
            face->add_item("Face Down");

            face->SelectedItemIndex = 0;
            face->ZOrder            = 15;
            face->Class             = "drop_down_list_wizard";
        }};
        auto const createPileLayout {[&](auto&& layout, std::string const& name) {
            createLabel(layout, "Layout", 14);
            auto cyb {layout->template create_widget<cycle_button>(name + "Layout")};
            cyb->add_item("Squared");
            cyb->add_item("Column");
            cyb->add_item("Row");
            cyb->add_item("Fan");
            cyb->SelectedItemIndex = 0;
            cyb->ZOrder            = 13;
        }};
        struct rule {
            std::string base;
            std::string build;
            std::string move;
        };
        auto const createPileRule {[&](auto&& layout, std::string const& name, rule const& select) {
            createLabel(layout, "Base", 12);
            auto base {layout->template create_widget<drop_down_list>(name + "Base")};
            base->add_item("Ace");
            base->add_item("King");
            base->add_item("Any");
            base->add_item("None");
            base->select_item(select.base);
            base->ZOrder = 11;
            base->Class  = "drop_down_list_wizard";

            createLabel(layout, "Build", 10);
            auto build {layout->template create_widget<drop_down_list>(name + "Build")};
            build->add_item("Any");
            build->add_item("InRank");
            build->add_item("InRankOrDownByRank");
            build->add_item("RankPack");
            build->add_item("UpOrDownByRank");
            build->add_item("DownByRank");
            build->add_item("UpByRank");
            build->add_item("UpOrDownAnyButOwnSuit");
            build->add_item("DownAnyButOwnSuit");
            build->add_item("UpAnyButOwnSuit");
            build->add_item("UpOrDownInSuit");
            build->add_item("DownInSuit");
            build->add_item("UpInSuit");
            build->add_item("UpOrDownInColor");
            build->add_item("DownInColor");
            build->add_item("UpInColor");
            build->add_item("UpOrDownAlternateColors");
            build->add_item("DownAlternateColors");
            build->add_item("UpAlternateColors");
            build->add_item("None");
            build->select_item(select.build);
            build->ZOrder = 9;
            build->Class  = "drop_down_list_wizard";

            createLabel(layout, "Move", 8);
            auto move {layout->template create_widget<drop_down_list>(name + "Move")};
            move->add_item("Top");
            move->add_item("FaceUp");
            move->add_item("InSeq");
            move->add_item("InSeqInSuit");
            move->add_item("None");
            move->select_item(select.move);
            move->ZOrder = 7;
            move->Class  = "drop_down_list_wizard";
        }};

        auto         tab {mainLayout->create_widget<tab_container>({0, 20, 30, 60}, "Piles")};
        size_i const boxSize {2, 12};
        {
            auto pnl {tab->create_tab<panel>("Stock", "Stock")};
            auto layout {pnl->create_layout<box_layout>(boxSize)};

            createLabel(layout, "Deal");
            auto cybStockTarget {layout->create_widget<cycle_button>("StockTarget")};
            cybStockTarget->add_item("To Waste");
            cybStockTarget->add_item("To Waste by Threes");
            cybStockTarget->add_item("To Tableau");
            cybStockTarget->select_item(cybStockTarget->get_item_at(0));

            createLabel(layout, "Redeals");
            auto spnRedeals {layout->create_widget<spinner>("Redeals")};
            spnRedeals->Flex  = {30_pct, 100_pct};
            spnRedeals->Min   = -1;
            spnRedeals->Max   = 5;
            spnRedeals->Step  = 1;
            spnRedeals->Value = 1;
        }
        {
            auto pnl {tab->create_tab<panel>("Waste", "Waste")};
            auto layout {pnl->create_layout<box_layout>(boxSize)};

            createPileSize(layout, "Waste");
            createPileLayout(layout, "Waste");
        }
        {
            auto pnl {tab->create_tab<panel>("Reserve", "Reserve")};
            auto layout {pnl->create_layout<box_layout>(boxSize)};

            createPileSize(layout, "Reserve");
            createCardCount(layout, "Reserve");
            createCardFace(layout, "Reserve");
            createPileLayout(layout, "Reserve");
        }
        {
            auto pnl {tab->create_tab<panel>("FreeCell", "FreeCell")};
            auto layout {pnl->create_layout<box_layout>(boxSize)};

            createPileSize(layout, "FreeCell");
            createCardCount(layout, "FreeCell");
            createPileLayout(layout, "FreeCell");
            createPileRule(layout, "FreeCell", {.base = "Any", .build = "None", .move = "Top"});
        }
        {
            auto pnl {tab->create_tab<panel>("Foundation", "Foundation")};
            auto layout {pnl->create_layout<box_layout>(boxSize)};

            createPileLayout(layout, "Foundation");
            createPileRule(layout, "Foundation", {.base = "Ace", .build = "UpInSuit", .move = "Top"});
        }
        {
            auto pnl {tab->create_tab<panel>("Tableau", "Tableau")};
            auto layout {pnl->create_layout<box_layout>(boxSize)};

            createPileSize(layout, "Tableau");
            createCardCount(layout, "Tableau");
            createCardFace(layout, "Tableau");
            createPileLayout(layout, "Tableau");
            createPileRule(layout, "Tableau", {.base = "Any", .build = "Any", .move = "Top"});
        }
    }

    _lbxLog        = mainLayout->create_widget<list_box>({32, 2, 36, 30}, "Log");
    _lbxLog->Class = "list_box_log";
    _lbxLog->HoveredItemIndex.Changed.connect([lb = _lbxLog.get()] {
        lb->HoveredItemIndex = -1;
    });

    BtnGenerate       = mainLayout->create_widget<button>({60, 72, 9, 5}, "btnGenerate");
    BtnGenerate->Icon = resGrp.get<gfx::texture>("apply");

    BtnBack       = mainLayout->create_widget<button>({70, 72, 9, 5}, "btnBack");
    BtnBack->Icon = resGrp.get<gfx::texture>("back");
}

void form_wizard::set_log_messages(std::vector<std::string> const& messages)
{
    _lbxLog->clear_items();
    for (auto const& m : messages) {
        _lbxLog->add_item(m);
    }
}

}
