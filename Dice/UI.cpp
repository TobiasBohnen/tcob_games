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

game_form::game_form(rect_f const& bounds, assets::group const& grp, shared_state const& state)
    : form {{.Name = "game", .Bounds = rect_i {bounds}}}
    , _sharedState {state}
{
    gen_styles(grp);

    auto& panel {create_container<ui::panel>(dock_style::Fill, "panel")};
    auto& layout {panel.create_layout<dock_layout>()};

    auto& ssd {layout.create_widget<seven_segment_display>(dock_style::Top, "ssd")};
    ssd.Flex = {.Width = 100_pct, .Height = 5_pct};
    ssd.draw_text("OIOI");

    auto& dmd {layout.create_widget<dot_matrix_display>(dock_style::Top, "dmd")};

    f32 const flexHeight {(bounds.width() * (DMD_HEIGHT / static_cast<f32>(DMD_WIDTH))) / bounds.height()};
    dmd.Flex  = {.Width = 100_pct, .Height = length(flexHeight, length::type::Relative)};
    auto dots = grid<u8> {{DMD_WIDTH, DMD_HEIGHT}, 3};

    dots[0, 0]                          = u8 {1};
    dots[DMD_WIDTH - 1, DMD_HEIGHT - 1] = u8 {5};
    dmd.Dots                            = dots;

    auto& btn {layout.create_widget<button>(dock_style::Bottom, "btnTurn")};
    btn.Flex  = {.Width = 100_pct, .Height = 15_pct};
    btn.Label = ">";
    btn.Click.connect([&]() { StartTurn(); });
    btn.disable();
}

void game_form::on_update(milliseconds deltaTime)
{
    if (_sharedState.CanStart) {
        find_widget_by_name("btnTurn")->enable();
    } else {
        find_widget_by_name("btnTurn")->disable();
    }

    dynamic_cast<dot_matrix_display*>(find_widget_by_name("dmd"))->Dots = _sharedState.Dots;

    form_base::on_update(deltaTime);
}

void game_form::gen_styles(assets::group const& grp)
{
    style_collection styles;
    {
        auto style {styles.create<panel>("panel", {})};
        style->Border.Size = 5_pct;
        style->Padding     = {1_pct};

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
        style->Border.Background   = colors::Gray;
        style->Text.Color          = colors::White;

        auto hoverStyle {styles.create<button>("button", {.Hover = true})};
        *hoverStyle                   = *style;
        hoverStyle->Background        = colors::FireBrick;
        hoverStyle->Border.Background = colors::Black;

        auto activeStyle {styles.create<button>("button", {.Focus = true, .Active = true})};
        *activeStyle                   = *style;
        activeStyle->Margin            = {5_px, 5_px, 10_px, 0_px};
        activeStyle->Background        = colors::Black;
        activeStyle->Border.Background = colors::FireBrick;

        auto disableStyle {styles.create<button>("button", {.Disabled = true})};
        *disableStyle                   = *style;
        disableStyle->Background        = colors::Gray;
        disableStyle->Border.Background = colors::Black;
        disableStyle->Text.Color        = colors::Black;
    }
    {
        auto style {styles.create<seven_segment_display>("seven_segment_display", {})};
        style->Size              = 5_pct;
        style->Border.Size       = 2_pct;
        style->Background        = colors::DimGray;
        style->ActiveColor       = colors::Red;
        style->InactiveColor     = colors::White;
        style->Border.Background = colors::Black;
    }
    {
        auto style {styles.create<dot_matrix_display>("dot_matrix_display", {})};
        style->Border.Size = 2_pct;
        style->Type        = dot_matrix_display::dot_type::Disc;
        style->Colors[0]   = {0, 0, 0, 0};
        style->Colors[1]   = {0, 0, 0, 255};
        style->Colors[2]   = {29, 43, 83, 255};
        style->Colors[3]   = {126, 37, 83, 255};
        style->Colors[4]   = {0, 135, 81, 255};
        style->Colors[5]   = {171, 82, 54, 255};
        style->Colors[6]   = {95, 87, 79, 255};
        style->Colors[7]   = {194, 195, 199, 255};
        style->Colors[8]   = {255, 241, 232, 255};
        style->Colors[9]   = {255, 0, 77, 255};
        style->Colors[10]  = {255, 163, 0, 255};
        style->Colors[11]  = {255, 236, 39, 255};
        style->Colors[12]  = {0, 228, 54, 255};
        style->Colors[13]  = {41, 173, 255, 255};
        style->Colors[14]  = {131, 118, 156, 255};

        style->Background        = colors::DimGray;
        style->Border.Background = colors::Black;
    }
    Styles = styles;
}
