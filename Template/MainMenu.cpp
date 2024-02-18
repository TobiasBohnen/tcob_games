// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "MainMenu.hpp"

#include "../_common/UIHelper.hpp"

namespace GAME_NAME {

main_menu::main_menu(gfx::window* window, rect_f bounds)
    : form {"MainMenu", window, bounds}
{
    auto& resMgr {locate_service<assets::library>()};
    auto& resGrp {resMgr.create_or_get_group("GAME_NAME_LOWER")};
    create_styles(resGrp, *Styles);

    auto mainPanel {create_container<panel>("main")};
    mainPanel->Dock = dock_style::Fill;

    auto mainPanelLayout {mainPanel->create_layout<grid_layout>(size_i {5, 20})};
    BtnStart        = mainPanelLayout->create_widget<button>({1, 1, 3, 2}, "btnPlay");
    BtnStart->Label = "New Game";

    BtnQuit        = mainPanelLayout->create_widget<button>({1, 17, 3, 2}, "btnQuit");
    BtnQuit->Label = "Quit";
}

}
