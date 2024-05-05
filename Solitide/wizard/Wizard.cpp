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

    auto const createLabel {[](auto&& layout, string const& text, isize zorder = 0) {
        auto lbl {layout->template create_widget<label>("lbl" + text)};
        lbl->Class  = "label-small";
        lbl->Label  = text;
        lbl->ZOrder = zorder;
    }};

    {
        auto pnl {mainLayout->create_widget<panel>({0, 0, 39, 20}, "GameInfo")};
        auto layout {pnl->create_layout<box_layout>(size_i {2, 3})};

        createLabel(layout, "Name");
        auto tbxName {layout->create_widget<text_box>("txtName")};

        createLabel(layout, "Deck Count");
        auto spnDecks {layout->create_widget<spinner>("spnDecks")};
        spnDecks->Flex  = {30_pct, 100_pct};
        spnDecks->Min   = 1;
        spnDecks->Max   = 4;
        spnDecks->Step  = 1;
        spnDecks->Value = 1;

        createLabel(layout, "Layout"); // TODO: replace with pile position
        auto cybLayout {layout->create_widget<cycle_button>("cybLayout")};
        cybLayout->add_item("klondike");
        cybLayout->add_item("bakers_dozen");
        cybLayout->add_item("beleaguered_castle");
        cybLayout->add_item("big_harp");
        cybLayout->add_item("canfield");
        cybLayout->add_item("canister");
        cybLayout->add_item("capricieuse");
        cybLayout->add_item("double_free_cell");
        cybLayout->add_item("fastness");
        cybLayout->add_item("flipper");
        cybLayout->add_item("forty_thieves");
        cybLayout->add_item("free_cell");
        cybLayout->add_item("golf");
        cybLayout->add_item("gypsy");
        cybLayout->add_item("yukon");
        cybLayout->SelectedItemIndex = 0;
    }

    _lbxLog        = mainLayout->create_widget<list_box>({0, 20, 39, 30}, "Log");
    _lbxLog->Class = "list_box_log";

    BtnGenerate       = mainLayout->create_widget<button>({20, 72, 9, 5}, "btnGenerate");
    BtnGenerate->Icon = resGrp.get<gfx::texture>("apply");
    BtnBack           = mainLayout->create_widget<button>({30, 72, 9, 5}, "btnBack");
    BtnBack->Icon     = resGrp.get<gfx::texture>("back");

    {
        auto const createPileSize {[&](auto&& layout, string const& name) {
            createLabel(layout, "# of Piles", 20);
            auto spn {layout->template create_widget<spinner>(name + "Size")};
            spn->Flex   = {30_pct, 100_pct};
            spn->Min    = 0;
            spn->Max    = 20;
            spn->Step   = 1;
            spn->Value  = 0;
            spn->ZOrder = 19;
        }};
        auto const createCardCount {[&](auto&& layout, string const& name) {
            createLabel(layout, "# of Cards", 18);
            auto spn {layout->template create_widget<spinner>(name + "CardCount")};
            spn->Flex   = {30_pct, 100_pct};
            spn->Min    = 0;
            spn->Max    = 200;
            spn->Step   = 1;
            spn->Value  = 0;
            spn->ZOrder = 17;
        }};
        auto const createCardFace {[&](auto&& layout, string const& name) {
            createLabel(layout, "Orientation", 16);
            auto base {layout->template create_widget<drop_down_list>(name + "Orientation")};
            base->add_item("Face Up");
            base->add_item("Top Card Face Up");
            base->add_item("Alternate - First Face Up");
            base->add_item("Alternate - First Face Down");
            base->add_item("Face Down");
            base->SelectedItemIndex = 0;
            base->ZOrder            = 15;
        }};
        auto const createPileLayout {[&](auto&& layout, string const& name) {
            createLabel(layout, "Layout", 14);
            auto cyb {layout->template create_widget<cycle_button>(name + "Layout")};
            cyb->add_item("Squared");
            cyb->add_item("Column");
            cyb->add_item("Row");
            cyb->add_item("Fan");
            cyb->SelectedItemIndex = 0;
            cyb->ZOrder            = 13;
        }};
        auto const createPileRule {[&](auto&& layout, string const& name) {
            createLabel(layout, "Base", 12);
            auto base {layout->template create_widget<drop_down_list>(name + "Base")};
            base->add_item("Ace");
            base->add_item("King");
            base->add_item("Any");
            base->add_item("None");
            base->SelectedItemIndex = 0;
            base->ZOrder            = 11;

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
            build->SelectedItemIndex = 0;
            build->ZOrder            = 9;

            createLabel(layout, "Move", 8);
            auto move {layout->template create_widget<drop_down_list>(name + "Move")};
            move->add_item("Top");
            move->add_item("FaceUp");
            move->add_item("InSeq");
            move->add_item("InSeqInSuit");
            move->add_item("None");
            move->SelectedItemIndex = 0;
            move->ZOrder            = 7;
        }};

        auto tab {mainLayout->create_widget<tab_container>({41, 0, 39, 80}, "Piles")};
        {
            auto pnl {tab->create_tab<panel>("Stock", "Stock")};
            auto layout {pnl->create_layout<box_layout>(size_i {2, 10})};

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
            auto layout {pnl->create_layout<box_layout>(size_i {2, 10})};

            createPileSize(layout, "Waste");
            createPileLayout(layout, "Waste");
        }
        {
            auto pnl {tab->create_tab<panel>("Reserve", "Reserve")};
            auto layout {pnl->create_layout<box_layout>(size_i {2, 10})};

            createPileSize(layout, "Reserve");
            createCardCount(layout, "Reserve");
            createCardFace(layout, "Reserve");
            createPileLayout(layout, "Reserve");
        }
        {
            auto pnl {tab->create_tab<panel>("FreeCell", "FreeCell")};
            auto layout {pnl->create_layout<box_layout>(size_i {2, 10})};

            createPileSize(layout, "FreeCell");
            createCardCount(layout, "FreeCell");
            createPileLayout(layout, "FreeCell");
            createPileRule(layout, "FreeCell");
        }
        {
            auto pnl {tab->create_tab<panel>("Foundation", "Foundation")};
            auto layout {pnl->create_layout<box_layout>(size_i {2, 10})};

            createPileLayout(layout, "Foundation");
            createPileRule(layout, "Foundation");
        }
        {
            auto pnl {tab->create_tab<panel>("Tableau", "Tableau")};
            auto layout {pnl->create_layout<box_layout>(size_i {2, 10})};

            createPileSize(layout, "Tableau");
            createCardCount(layout, "Tableau");
            createCardFace(layout, "Tableau");
            createPileLayout(layout, "Tableau");
            createPileRule(layout, "Tableau");
        }
    }
}

void form_wizard::set_log_messages(std::vector<string> const& messages)
{
    _lbxLog->clear_items();
    for (auto const& m : messages) {
        _lbxLog->add_item(m);
    }
}

}
