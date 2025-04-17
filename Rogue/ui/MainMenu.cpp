// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "MainMenu.hpp"

namespace Rogue {

using namespace tcob::ui;

main_menu::main_menu(assets::group const& resGrp, rect_i const& bounds)
    : form {{"MainMenu", bounds}}
{
    style_collection styles;
    {
        auto style {styles.create<panel>("panel", {})};
        style->Background = colors::White;
    }
    {
        auto style {styles.create<terminal>("terminal", {})};
        style->FlashDuration   = 100ms;
        style->Text.Font       = resGrp.get<gfx::font_family>("Font");
        style->Text.Style      = {.IsItalic = false, .Weight = gfx::font::weight::Normal};
        style->Text.Size       = {static_cast<f32>(bounds.height() / TermSize.Height), length::type::Absolute};
        style->Caret.BlinkRate = 0ms;
        style->Caret.Color     = colors::Transparent;
        style->Background      = colors::White;
    }

    auto mainPanel {create_container<panel>(dock_style::Fill, "main")};

    auto& mainPanelLayout {mainPanel->create_layout<dock_layout>()};
    Terminal       = mainPanelLayout.create_widget<terminal>(dock_style::Fill, "Terminal");
    Terminal->Size = TermSize;
    Terminal->curs_set(false);
    Terminal->mouse_set(true);
    Terminal->noecho();

    Styles = styles;
}

}
