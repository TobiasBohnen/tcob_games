// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "MainMenu.hpp"

#include "../_common/UIHelper.hpp"

namespace Mines {

auto create_slider(grid_layout& layout, std::string const& name,
                   i32 row, i32 min, i32 max, i32 initialValue) -> std::shared_ptr<slider>
{
    auto lb0 {layout.create_widget<label>({1, row, 1, 1}, "lb" + name)};
    lb0->Label = name;

    auto lb1 {layout.create_widget<label>({4, row, 1, 1}, "lb" + name + "Val")};

    auto sl1 {layout.create_widget<slider>({2, row, 2, 1}, "sld" + name)};
    sl1->Value.Changed.connect([lb1](auto const& ev) { lb1->Label = std::to_string(ev); });
    sl1->Min   = min;
    sl1->Max   = max;
    sl1->Value = initialValue;
    return sl1;
}

main_menu::main_menu(assets::group const& resGrp, rect_i const& bounds)
    : form {{.Name = "MainMenu", .Bounds = bounds}}
{
    create_styles(resGrp, Styles.mut_ref());

    auto mainPanel {create_container<panel>(dock_style::Fill, "main")};

    auto& mainPanelLayout {mainPanel->create_layout<grid_layout>(size_i {5, 20})};
    BtnStart        = mainPanelLayout.create_widget<button>({1, 1, 3, 2}, "btnPlay");
    BtnStart->Label = "New Game";

    SldWidth  = create_slider(mainPanelLayout, "Width", 4, 3, 25, 10);
    SldHeight = create_slider(mainPanelLayout, "Height", 5, 3, 25, 10);
    SldMines  = create_slider(mainPanelLayout, "Mines", 6, 1, 99, 10);

    SldWidth->Value.Changed.connect([&](auto const&) { SldMines->Max = (SldWidth->Value * SldHeight->Value) - 1; });
    SldHeight->Value.Changed.connect([&](auto const&) { SldMines->Max = (SldWidth->Value * SldHeight->Value) - 1; });

    BtnQuit        = mainPanelLayout.create_widget<button>({1, 17, 3, 2}, "btnQuit");
    BtnQuit->Label = "Quit";
}

}
