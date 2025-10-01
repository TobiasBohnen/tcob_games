// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "MainMenu.hpp"

using namespace tcob::ui;
using namespace tcob::gfx;
using namespace tcob::literals;

namespace Snake {

main_menu::main_menu(assets::group const& resGrp, rect_i const& bounds)
    : form {{.Name = "MainMenu", .Bounds = bounds}}
    , _font {resGrp.get<gfx::font_family>("Poppins")}
{
    gen_styles();

    auto& mainPanel {create_container<panel>(dock_style::Fill, "main")};

    auto& mainPanelLayout {mainPanel.create_layout<grid_layout>(size_i {5, 20})};

    auto& lblScore {mainPanelLayout.create_widget<label>({0, 1, 5, 3}, "lblScore")};
    lblScore.Label = "0";
    Score.connect([&](auto const val) { lblScore.Label = val; });

    auto& btnStart {mainPanelLayout.create_widget<button>({1, 5, 3, 2}, "btnPlay")};
    btnStart.Label = "New Game";
    btnStart.Click.connect([&] { Start(); });

    auto& btnQuit {mainPanelLayout.create_widget<button>({1, 17, 3, 2}, "btnQuit")};
    btnQuit.Label = "Quit";
    btnQuit.Click.connect([&] { Quit(); });

    for (auto* widget : all_widgets()) {
        widget->TransitionDuration = 250ms;
    }
}

void main_menu::gen_styles()
{
    style_collection styles;
    {
        auto style {styles.create<panel>("panel", {})};
        style->Border.Type   = border_type::Dotted;
        style->Border.Size   = 3_px;
        style->Border.Radius = 5_px;
        style->Margin        = {2_px};
        style->Padding       = {2_px};

        style->Background        = colors::DarkSeaGreen;
        style->Border.Background = colors::Black;
    }
    {
        auto style {styles.create<button>("button", {})};
        style->Border.Type    = border_type::Solid;
        style->Border.Size    = 3_px;
        style->Border.Radius  = 10_px;
        style->Text.Style     = {.IsItalic = true, .Weight = font::weight::ExtraBold};
        style->Text.Font      = _font;
        style->Text.Size      = 50_pct;
        style->Text.Alignment = {.Horizontal = horizontal_alignment::Centered, .Vertical = vertical_alignment::Middle};
        style->Text.AutoSize  = auto_size_mode::OnlyShrink;
        style->Margin         = {2_px};
        style->Padding        = {2_px};

        style->Background        = colors::Snow;
        style->Border.Background = colors::SpringGreen;
        style->Text.Color        = colors::Black;

        auto activeStyle {styles.create<button>("button", {.Active = true, .Hover = true})};
        *activeStyle = *style;

        activeStyle->Background        = colors::SpringGreen;
        activeStyle->Border.Background = colors::Snow;
        activeStyle->Text.Color        = colors::White;

        auto hoverStyle {styles.create<button>("button", {.Hover = true})};
        *hoverStyle = *style;

        hoverStyle->Background        = colors::SpringGreen;
        hoverStyle->Border.Background = colors::DarkGray;
        hoverStyle->Text.Color        = colors::Black;
    }
    {
        auto style {styles.create<label>("label", {})};
        style->Border.Type    = border_type::Double;
        style->Border.Size    = 3_px;
        style->Text.Style     = {.IsItalic = false, .Weight = font::weight::Normal};
        style->Text.Font      = _font;
        style->Text.Size      = 40_pct;
        style->Text.Alignment = {.Horizontal = horizontal_alignment::Centered, .Vertical = vertical_alignment::Middle};
        style->Text.AutoSize  = auto_size_mode::OnlyShrink;
        style->Margin         = {2_px};
        style->Padding        = {5_px, 5_px, 0_px, 0_px};

        style->Background        = colors::Snow;
        style->Border.Background = colors::Black;
        style->Text.Color        = colors::Black;
    }
    Styles = styles;
}

}
