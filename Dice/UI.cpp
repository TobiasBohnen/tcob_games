// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "UI.hpp"

using namespace tcob::literals;
using namespace tcob::gfx;
using namespace tcob::ui;
using namespace std::chrono_literals;

////////////////////////////////////////////////////////////

game_form::game_form(rect_f const& bounds, assets::group const& grp)
    : form {{.Name = "game", .Bounds = rect_i {bounds}}}
{
    i32 const dotsW {48};
    i32 const dotsH {32};

    gen_styles(grp);

    auto& panel {create_container<ui::panel>(dock_style::Fill, "panel")};
    auto& layout {panel.create_layout<dock_layout>()};

    auto& ssd {layout.create_widget<seven_segment_display>(dock_style::Top, "ssd")};
    ssd.Flex = {.Width = 100_pct, .Height = 10_pct};
    ssd.draw_text("OIOI");
    auto&     dmd {layout.create_widget<dot_matrix_display>(dock_style::Top, "dmd")};
    f32 const flexHeight {(bounds.width() * (dotsH / static_cast<f32>(dotsW))) / bounds.height()};
    dmd.Flex  = {.Width = 100_pct, .Height = length(flexHeight, length::type::Relative)};
    auto dots = grid<u8> {{dotsW, dotsH}};

    dots[0, 0]   = u8 {1};
    dots[47, 31] = u8 {5};
    dmd.Dots     = dots;

    auto& btn {layout.create_widget<button>(dock_style::Bottom, "btnTurn")};
    btn.Flex = {.Width = 100_pct, .Height = 5_pct};
}

void game_form::gen_styles(assets::group const& grp)
{
    style_collection styles;
    {
        auto style {styles.create<panel>("panel", {})};
        style->Border.Size   = 3_px;
        style->Border.Radius = 5_px;
        style->Padding       = {5_px};

        style->Background        = colors::LightSteelBlue;
        style->Border.Background = colors::Black;
    }
    {
        auto style {styles.create<button>("button", {})};
        style->Border.Type         = border_type::Solid;
        style->Border.Size         = 3_px;
        style->Border.Radius       = 5_px;
        style->Text.Style          = {.IsItalic = false, .Weight = font::weight::Normal};
        style->Text.Font           = grp.get<font_family>("Poppins");
        style->Text.Size           = 50_pct;
        style->Text.Shadow.OffsetX = 0_px;
        style->Text.Shadow.OffsetY = 1_px;
        style->Text.AutoSize       = auto_size_mode::Always;
        style->Text.Alignment      = {.Horizontal = horizontal_alignment::Centered, .Vertical = vertical_alignment::Middle};
        style->Margin              = {10_px};
        style->Padding             = {2_px};
        style->Background          = colors::FireBrick;
        style->Border.Background   = colors::Black;
        style->Text.Color          = colors::White;

        auto hoverStyle {styles.create<button>("button", {.Hover = true})};
        *hoverStyle                   = *style;
        hoverStyle->Margin            = {5_px};
        hoverStyle->Background        = colors::Black;
        hoverStyle->Border.Background = colors::FireBrick;

        auto activeStyle {styles.create<button>("button", {.Focus = true, .Active = true})};
        *activeStyle        = *style;
        activeStyle->Margin = {5_px, 5_px, 10_px, 0_px};
    }
    {
        auto style {styles.create<seven_segment_display>("seven_segment_display", {})};
        style->Size          = 5_pct;
        style->Background    = colors::DimGray;
        style->ActiveColor   = colors::Red;
        style->InactiveColor = colors::White;
    }
    {
        auto style {styles.create<dot_matrix_display>("dot_matrix_display", {})};
        style->Colors[0] = colors::Black;
        style->Colors[1] = colors::Red;
        style->Colors[2] = colors::Green;
        style->Colors[3] = colors::Yellow;
        style->Colors[4] = colors::Cyan;
        style->Colors[5] = colors::Magenta;
        style->Colors[6] = colors::White;

        style->Type       = dot_matrix_display::dot_type::Disc;
        style->Background = colors::DimGray;
    }
    Styles = styles;
}
