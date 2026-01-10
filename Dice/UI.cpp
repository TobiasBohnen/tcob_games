// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "UI.hpp"

using namespace tcob::literals;
using namespace tcob::gfx;
using namespace tcob::ui;
using namespace std::chrono_literals;

////////////////////////////////////////////////////////////

game_form::game_form(rect_f const& bounds, assets::group const& grp, shared_state& state, event_bus& events)
    : form {{.Name = "game", .Bounds = rect_i {bounds}}, size_i {100, 100}}
    , _sharedState {state}
{
    gen_styles(grp);

    _dmdTexture->resize(DMD_SIZE, 1, gfx::texture::format::RGBA8);
    _dmdTexture->regions()["default"] = texture_region {.UVRect = {0, 0, 1, 1}, .Level = 0};
    // _dmdTexture->Filtering            = gfx::texture::filtering::Linear;

    {
        auto& panel1 {create_container<ui::panel>(rect_i {0, 0, 100, 73}, "panel1")};
        panel1.disable();
        auto& layout {panel1.create_layout<grid_layout>(size_i {100, 100})};

        layout.create_widget<seven_segment_display>({0, 0, 50, 10}, "ssd0");
        _sharedState.Score.Changed.connect([&]() { _updateSsd0 = true; });

        layout.create_widget<seven_segment_display>({50, 0, 50, 10}, "ssd1");
        _sharedState.SSD.Changed.connect([&]() { _updateSsd1 = true; });

        auto& dmd {layout.create_widget<image_box>({0, 10, 100, 90}, "dmd")};
        dmd.Image = {.Texture = _dmdTexture};
        _sharedState.DMD.Changed.connect([&]() { _updateDmd = true; });
        dmd.Bounds.Changed.connect([this, &dmd]() {
            _sharedState.DMDBounds = {
                local_to_screen(dmd, dmd.content_bounds().Position),
                {dmd.content_bounds().width(), dmd.content_bounds().width() / DMD_SIZE.Width * DMD_SIZE.Height}};
        });
    }
    {
        auto& panel2 {create_container<ui::panel>(rect_i {0, 73, 100, 25}, "panel2")};
        auto& layout {panel2.create_layout<grid_layout>(size_i {4, 4})};

        auto& btn0 {layout.create_widget<button>({0, 0, 3, 4}, "btn0")};
        btn0.Label = "GO";
        btn0.Click.connect([&events]() { events.StartTurn(); });
        _sharedState.CanStart.Changed.connect([&btn0](auto val) {
            if (val) {
                btn0.enable();
            } else {
                btn0.disable();
            }
        });

        auto& btn1 {layout.create_widget<button>({3, 0, 1, 2}, "btn1")};
        btn1.Label = "RESET";
        btn1.Class = "button2";
        btn1.Click.connect([&events]() { events.Restart(); });

        auto& btn2 {layout.create_widget<button>({3, 2, 1, 2}, "btn2")};
        btn2.Label = "OFF";
        btn2.Class = "button2";
        btn2.Click.connect([&events]() { events.Quit(); });
    }

    update(0ms);
}

void game_form::on_update(milliseconds deltaTime)
{
    if (_updateDmd) {
        _dmdTexture->update_data(*_sharedState.DMD, 0);
        _updateDmd = false;
        find_widget_by_name("dmd")->queue_redraw();
    }
    if (_updateSsd0) {
        auto* ssd {dynamic_cast<seven_segment_display*>(find_widget_by_name("ssd0"))};
        ssd->draw_text(std::format("{:07}", std::clamp(*_sharedState.Score, 0, 9'999'999)));
        _updateSsd0 = false;
    }
    if (_updateSsd1) {
        auto* ssd {dynamic_cast<seven_segment_display*>(find_widget_by_name("ssd1"))};
        ssd->draw_text(*_sharedState.SSD);
        _updateSsd1 = false;
    }
    form_base::on_update(deltaTime);
}

void game_form::gen_styles(assets::group const& grp)
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
        style->Border.Size       = 3_pct;
        style->Border.Radius     = 1_pct;
        style->Text.Style        = {.IsItalic = false, .Weight = font::weight::Normal};
        style->Text.Font         = grp.get<font_family>("Font");
        style->Text.Size         = 50_pct;
        style->Text.AutoSize     = auto_size_mode::OnlyShrink;
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
        auto style {styles.create<button>("button2", {})};
        style->Border.Type       = border_type::Solid;
        style->Border.Size       = 3_pct;
        style->Border.Radius     = 10_pct;
        style->Text.Style        = {.IsItalic = false, .Weight = font::weight::Normal};
        style->Text.Font         = grp.get<font_family>("Font");
        style->Text.Size         = 50_pct;
        style->Text.AutoSize     = auto_size_mode::Always;
        style->Text.Alignment    = {.Horizontal = horizontal_alignment::Centered, .Vertical = vertical_alignment::Middle};
        style->Margin            = {5_pct};
        style->Padding           = {2_pct};
        style->Background        = colors::FireBrick;
        style->Border.Background = colors::Gray;
        style->Text.Color        = colors::White;

        auto hoverStyle {styles.create<button>("button2", {.Hover = true})};
        *hoverStyle                   = *style;
        hoverStyle->Background        = colors::FireBrick;
        hoverStyle->Border.Background = colors::Black;

        auto activeStyle {styles.create<button>("button2", {.Focus = true, .Active = true})};
        *activeStyle                   = *style;
        activeStyle->Margin            = {1_pct, 1_pct, 2_pct, 0_pct};
        activeStyle->Background        = colors::Black;
        activeStyle->Border.Background = colors::FireBrick;
    }
    {
        auto style {styles.create<seven_segment_display>("seven_segment_display", {})};
        style->Size              = 11.0_pct;
        style->Padding           = {3_pct, 0_pct, 2.5_pct, 0_pct};
        style->Margin            = {0_pct, 0_pct, 1.5_pct, 2_pct};
        style->Border.Size       = 2_pct;
        style->Background        = colors::Black;
        style->ActiveColor       = colors::FireBrick;
        style->InactiveColor     = colors::Transparent;
        style->Border.Type       = border_type::Inset;
        style->Border.Background = colors::Gray;
    }
    {
        auto style {styles.create<image_box>("image_box", {})};
        style->Background = colors::Transparent;
    }
    Styles = styles;
}

////////////////////////////////////////////////////////////

constexpr i32 NUM_BOXES {5};

auto game_select_form::make_tooltip(u32 id) -> std::shared_ptr<tooltip>
{
    auto retValue {create_tooltip<>("tooltip")};
    retValue->Class = "tooltip";
    TooltipOffset   = {40, 40};

    auto& tooltipLayout {retValue->create_layout<dock_layout>()};
    auto& tooltipLabel {tooltipLayout.create_widget<label>(dock_style::Fill, "")};
    tooltipLabel.Class = "label";

    retValue->Popup.connect([this, &tooltipLabel, id, tt = retValue.get()](auto const& event) -> void {
        auto* const widget {event.Widget};
        auto const& game {_games[id]};
        tooltipLabel.Label = std::format("{}\n{}", game.Name, game.Info.Genre);

        rect_f const bounds {widget->Bounds};
        auto const*  style {dynamic_cast<label::style const*>(tooltipLabel.current_style())};
        assert(style);
        auto* const font {style->Text.Font->get_font(style->Text.Style, style->Text.calc_font_size(bounds.height() * 0.2f)).ptr()};
        tt->Bounds = {point_f::Zero, gfx::text_formatter::measure(*tooltipLabel.Label, *font, -1, true)};
    });
    return retValue;
}

game_select_form::game_select_form(rect_f const& bounds, assets::group const& grp, std::map<u32, game_def> const& games)
    : form {{.Name = "game", .Bounds = rect_i {bounds}}}
    , _games {games}
{
    gen_styles(grp);

    {
        auto&        panel1 {create_container<ui::panel>(rect_f {bounds.Position, {bounds.width(), bounds.height() / 2}}, "panel1")};
        size_i const gridSize {NUM_BOXES * 4, 1};
        auto&        layout {panel1.create_layout<grid_layout>(gridSize)};

        for (i32 i {0}; i < gridSize.area(); ++i) {
            i32 const x {((i % NUM_BOXES) * 4) + 1};
            i32 const y {(i / NUM_BOXES)};

            auto& imgBox {layout.create_widget<image_box>({x, y, 3, 1}, "")};
            imgBox.Fit = fit_mode::Contain;
            auto& imgBoxLbl {layout.create_widget<label>({x - 1, y, 1, 1}, "")};

            u32 const gameNumber {static_cast<u32>(i + 1)};
            imgBoxLbl.Label = std::format("#{}", gameNumber);

            if (games.contains(gameNumber)) {
                auto const& game {games.at(gameNumber)};
                imgBox.Image = {.Texture = game.CoverTex};
                imgBox.Click.connect([gameNumber, this] { StartGame(gameNumber); });
                imgBox.Tooltip = make_tooltip(gameNumber);
            }
        }
    }
}

void game_select_form::gen_styles(assets::group const& grp)
{
    style_collection styles;
    {
        auto style {styles.create<panel>("panel", {})};
        style->Padding     = 1_pct;
        style->Border.Size = 1_pct;
        style->Border.Type = border_type::Double;

        style->Background        = colors::Black;
        style->Border.Background = colors::White;
    }
    {
        auto style {styles.create<label>("label", {})};
        style->Text.Style     = {.IsItalic = false, .Weight = font::weight::Normal};
        style->Text.Font      = grp.get<font_family>("Font");
        style->Text.Size      = 50_pct;
        style->Text.AutoSize  = auto_size_mode::Always;
        style->Text.Alignment = {.Horizontal = horizontal_alignment::Centered, .Vertical = vertical_alignment::Middle};
        style->Padding        = {2_pct};
        style->Background     = colors::DarkGray;
        style->Text.Color     = colors::White;
    }
    {
        auto style {styles.create<image_box>("image_box", {})};
        style->Background = colors::LightGray;
        style->Margin     = 1_pct;
        style->Alignment  = {.Horizontal = horizontal_alignment::Centered, .Vertical = vertical_alignment::Middle};

        auto hoverStyle {styles.create<image_box>("image_box", {.Hover = true})};
        *hoverStyle            = *style;
        hoverStyle->Background = colors::FireBrick;
    }
    {
        auto style {styles.create<tooltip>("tooltip", {})};
        style->Border.Size = 1_pct;
        style->Border.Type = border_type::Centered;

        style->Background        = colors::Black;
        style->Border.Background = colors::White;
    }
    Styles = styles;
}
