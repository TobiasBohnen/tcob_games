// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "MainMenu.hpp"

#include "../_common/UIHelper.hpp"

namespace Snake {

main_menu::main_menu(assets::group const& resGrp, rect_i const& bounds)
    : form {{"MainMenu", bounds}}
{
    create_styles(resGrp, *Styles);

    auto mainPanel {create_container<panel>(dock_style::Fill, "main")};

    auto& mainPanelLayout {mainPanel->create_layout<grid_layout>(size_i {5, 20})};
    BtnStart        = mainPanelLayout.create_widget<button>({1, 1, 3, 2}, "btnPlay");
    BtnStart->Label = "New Game";

    LblScore        = mainPanelLayout.create_widget<label>({1, 4, 3, 2}, "lblScore");
    LblScore->Label = "0";

    BtnQuit        = mainPanelLayout.create_widget<button>({1, 17, 3, 2}, "btnQuit");
    BtnQuit->Label = "Quit";
}

}
