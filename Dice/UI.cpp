// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "UI.hpp"

using namespace tcob::literals;
using namespace tcob::gfx;
using namespace tcob::ui;
using namespace std::chrono_literals;

void gen_styles(form_base& base, assets::group const& grp)
{
    style_collection styles;
    {
        auto style {styles.create<panel>("panel", {})};
        style->Padding     = 2_pct;
        style->Border.Size = 2_pct;
        style->Border.Type = border_type::Cornered;

        style->Background        = colors::Beige;
        style->Border.Background = colors::Maroon;
    }
    {
        auto style {styles.create<button>("button", {})};
        style->Border.Type       = border_type::Solid;
        style->Border.Size       = 1_pct;
        style->Border.Radius     = 1_pct;
        style->Text.Style        = {.IsItalic = false, .Weight = font::weight::Normal};
        style->Text.Font         = grp.get<font_family>("Font");
        style->Text.Size         = 50_pct;
        style->Text.AutoSize     = auto_size_mode::Always;
        style->Text.Alignment    = {.Horizontal = horizontal_alignment::Centered, .Vertical = vertical_alignment::Middle};
        style->Margin            = {1_pct};
        style->Padding           = {2_pct};
        style->Background        = colors::FireBrick;
        style->Border.Background = colors::Gray;
        style->Text.Color        = colors::White;

        auto hoverStyle {styles.create<button>("button", {.Hover = true})};
        *hoverStyle                   = *style;
        hoverStyle->Background        = colors::FireBrick;
        hoverStyle->Border.Background = colors::Black;

        auto activeStyle {styles.create<button>("button", {.Focus = true, .Active = true})};
        *activeStyle                   = *style;
        activeStyle->Margin            = {1_pct, 1_pct, 2_pct, 0_pct};
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
        style->Size              = 11.0_pct;
        style->Padding           = {3_pct, 0_pct, 2.5_pct, 0_pct};
        style->Margin            = {0_pct, 0_pct, 1.5_pct, 2_pct};
        style->Border.Size       = 2_pct;
        style->Background        = colors::Black;
        style->ActiveColor       = colors::FireBrick;
        style->InactiveColor     = colors::Black;
        style->Border.Type       = border_type::Inset;
        style->Border.Background = colors::Gray;
    }
    {
        auto style {styles.create<dot_matrix_display>("dot_matrix_display", {})};
        style->Type = dot_matrix_display::dot_type::Disc;
        for (i32 i {0}; i < PALETTE.size(); ++i) {
            style->Colors[i] = PALETTE[i];
        }

        style->Background = colors::Black;
    }
    {
        auto style {styles.create<image_box>("image_box", {})};
        style->Background        = colors::Black;
        style->Margin            = 1_pct;
        style->Alignment         = {.Horizontal = horizontal_alignment::Centered, .Vertical = vertical_alignment::Middle};
        style->Border.Size       = 2_pct;
        style->Border.Background = colors::Gray;

        auto hoverStyle {styles.create<image_box>("image_box", {.Hover = true})};
        *hoverStyle                   = *style;
        hoverStyle->Background        = colors::FireBrick;
        hoverStyle->Border.Background = colors::Black;
    }
    base.Styles = styles;
}

////////////////////////////////////////////////////////////

game_form::game_form(rect_f const& bounds, assets::group const& grp, shared_state& state, event_bus& events)
    : form {{.Name = "game", .Bounds = rect_i {bounds}}, size_i {100, 100}}
    , _sharedState {state}
{
    gen_styles(*this, grp);

    {
        auto& panel1 {create_container<ui::panel>(rect_i {0, 0, 100, 73}, "panel1")};
        panel1.disable();
        auto& layout {panel1.create_layout<grid_layout>(size_i {100, 100})};

        auto& ssd0 {layout.create_widget<seven_segment_display>({0, 0, 50, 10}, "ssd0")};
        _sharedState.Score.Changed.connect([&]() { _updateSsd0 = true; });
        ssd0.disable();

        auto& ssd1 {layout.create_widget<seven_segment_display>({50, 0, 50, 10}, "ssd1")};
        _sharedState.SSDValue.Changed.connect([&]() { _updateSsd1 = true; });
        ssd1.disable();

        auto& dmd {layout.create_widget<dot_matrix_display>({0, 10, 100, 90}, "dmd")};
        _sharedState.DMD.Changed.connect([&]() { _updateDmd = true; });
        dmd.disable();
        dmd.Bounds.Changed.connect([this, &panel1, &dmd](rect_f const& rect) {
            _sharedState.DMDBounds = {
                local_to_screen(dmd, dmd.content_bounds().Position),
                {dmd.content_bounds().width(), dmd.content_bounds().width() / DMD_SIZE.Width * DMD_SIZE.Height}};
        });
    }
    {
        auto& panel2 {create_container<ui::panel>(rect_i {0, 73, 100, 25}, "panel2")};
        auto& layout {panel2.create_layout<dock_layout>()};

        auto& btn {layout.create_widget<button>(dock_style::Bottom, "btnTurn")};
        btn.Flex  = {.Width = 100_pct, .Height = 100_pct};
        btn.Label = "->";
        btn.Click.connect([&events]() { events.StartTurn(); });
        _sharedState.CanStart.Changed.connect([&btn](auto val) { val ? btn.enable() : btn.disable(); });
        btn.disable();
    }

    update(0ms);
}

void game_form::on_update(milliseconds deltaTime)
{
    if (_updateDmd) {
        dynamic_cast<dot_matrix_display*>(find_widget_by_name("dmd"))->Dots = *_sharedState.DMD;
        _updateDmd                                                          = false;
    }
    if (_updateSsd0) {
        auto* ssd {dynamic_cast<seven_segment_display*>(find_widget_by_name("ssd0"))};
        ssd->draw_text(std::format("{:07}", std::clamp(*_sharedState.Score, 0, 9'999'999)));
        _updateSsd0 = false;
    }
    if (_updateSsd1) {
        auto* ssd {dynamic_cast<seven_segment_display*>(find_widget_by_name("ssd1"))};
        ssd->draw_text(*_sharedState.SSDValue);
        _updateSsd1 = false;
    }
    form_base::on_update(deltaTime);
}

////////////////////////////////////////////////////////////

constexpr size_i NUM_BOXES {5, 5};

game_select_form::game_select_form(rect_f const& bounds, assets::group const& grp, std::map<u32, game_def> const& games)
    : form {{.Name = "game", .Bounds = rect_i {bounds}}}
{
    gen_styles(*this, grp);

    {
        auto& panel1 {create_container<ui::panel>(bounds, "panel1")};
        auto& layout {panel1.create_layout<grid_layout>(NUM_BOXES)};
        for (i32 i {0}; i < NUM_BOXES.area(); ++i) {
            auto& imgBox {layout.create_widget<image_box>({i % NUM_BOXES.Width, i / NUM_BOXES.Width, 1, 1}, "")};
            imgBox.Fit = fit_mode::Contain;
            if (games.contains(i + 1)) {
                imgBox.Image = {.Texture = games.at(i + 1).Cover};
                imgBox.Click.connect([i, this] {
                    StartGame(static_cast<u32>(i + 1));
                });
            }
        }
    }
}
