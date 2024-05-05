#include "UIHelper.hpp"

namespace solitaire {

////////////////////////////////////////////////////////////

void color_theme::apply(std::shared_ptr<button::style> const& style) const
{
    style->Background        = Background;
    style->Border.Background = Border;
    style->DropShadow.Color  = Shadow;
    style->Text.Color        = Foreground;
}

void color_theme::apply(std::shared_ptr<checkbox::style> const& style) const
{
    style->Background        = Background;
    style->Border.Background = Border;
    style->DropShadow.Color  = Shadow;
    style->Tick.Foreground   = Foreground;
}

void color_theme::apply(std::shared_ptr<cycle_button::style> const& style) const
{
    style->Background        = Background;
    style->Border.Background = Border;
    style->DropShadow.Color  = Shadow;
    style->Text.Color        = Foreground;
}

void color_theme::apply(std::shared_ptr<drop_down_list::style> const& style) const
{
    style->Background                      = Background;
    style->Border.Background               = Border;
    style->DropShadow.Color                = Shadow;
    style->Text.Color                      = Foreground;
    style->VScrollBar.Bar.HigherBackground = Background;
    style->VScrollBar.Bar.LowerBackground  = Background;
}

void color_theme::apply(std::shared_ptr<grid_view::style> const& style) const
{
    style->Background                      = Background;
    style->Border.Background               = Border;
    style->DropShadow.Color                = Shadow;
    style->VScrollBar.Bar.HigherBackground = Background;
    style->VScrollBar.Bar.LowerBackground  = Background;
}

void color_theme::apply(std::shared_ptr<image_box::style> const& style) const
{
    style->Background        = Background;
    style->Border.Background = Border;
    style->DropShadow.Color  = Shadow;
}

void color_theme::apply(std::shared_ptr<label::style> const& style) const
{
    style->Background        = Label;
    style->Border.Background = Border;
    style->Text.Color        = Foreground;
}

void color_theme::apply(std::shared_ptr<list_box::style> const& style) const
{
    style->Background                      = Background;
    style->Border.Background               = Border;
    style->VScrollBar.Bar.HigherBackground = Background;
    style->VScrollBar.Bar.LowerBackground  = Background;
}

void color_theme::apply(std::shared_ptr<progress_bar::style> const& style) const
{
    style->Background           = Background;
    style->Border.Background    = Border;
    style->DropShadow.Color     = Shadow;
    style->Bar.HigherBackground = Background;
    style->Bar.LowerBackground  = Background;
}

void color_theme::apply(std::shared_ptr<radio_button::style> const& style) const
{
    style->Background        = Background;
    style->Border.Background = Border;
    style->DropShadow.Color  = Shadow;
    style->Tick.Foreground   = Foreground;
}

void color_theme::apply(std::shared_ptr<slider::style> const& style) const
{
    style->Bar.HigherBackground = Background;
    style->Bar.LowerBackground  = Background;
}

void color_theme::apply(std::shared_ptr<spinner::style> const& style) const
{
    style->Background        = Background;
    style->Border.Background = Border;
    style->DropShadow.Color  = Shadow;
    style->Text.Color        = Foreground;
}

void color_theme::apply(std::shared_ptr<text_box::style> const& style) const
{
    style->Background        = Background;
    style->Border.Background = Border;
    style->DropShadow.Color  = Shadow;
    style->Text.Color        = Foreground;
    style->Caret.Color       = Foreground;
}

void color_theme::apply(std::shared_ptr<toggle::style> const& style) const
{
    style->Background        = Background;
    style->Border.Background = Border;
    style->DropShadow.Color  = Shadow;
    style->Tick.Foreground   = Foreground;
}

void color_theme::apply(std::shared_ptr<panel::style> const& style) const
{
    style->Background                      = Container;
    style->Border.Background               = Border;
    style->VScrollBar.Bar.HigherBackground = Background;
    style->VScrollBar.Bar.LowerBackground  = Background;
    style->HScrollBar.Bar.HigherBackground = Background;
    style->HScrollBar.Bar.LowerBackground  = Background;
}

void color_theme::apply(std::shared_ptr<tab_container::style> const& style) const
{
    style->Background        = Container;
    style->Border.Background = Border;
}

void color_theme::apply(std::shared_ptr<accordion::style> const& style) const
{
    style->Background        = Container;
    style->Border.Background = Border;
}

void color_theme::apply(std::shared_ptr<tooltip::style> const& style) const
{
    style->Background        = Background;
    style->Border.Background = Border;
}

void color_theme::apply(std::shared_ptr<thumb_style> const& style) const
{
    style->Thumb.Background        = List;
    style->Thumb.Border.Background = Border;
}

void color_theme::apply(std::shared_ptr<item_style> const& style) const
{
    style->Item.Background        = List;
    style->Item.Border.Background = Border;
    style->Item.Text.Color        = Foreground;
}

void color_theme::apply(std::shared_ptr<nav_arrows_style> const& style) const
{
    style->NavArrow.Foreground        = Foreground;
    style->NavArrow.IncBackground     = Background;
    style->NavArrow.DecBackground     = Background;
    style->NavArrow.Border.Background = Border;
}

void create_styles(color_themes const& theme, assets::group& resGrp, style_collection& styles)
{
    using namespace tcob::literals;
    using namespace std::chrono_literals;
    namespace element = element;

    styles.clear();
    auto fntFam {resGrp.get<gfx::font_family>(FONT)};

    {
        auto style {styles.create<panel>("panel", {})};
        // style->Padding       = 2_px;
        style->Border.Size   = 2_px;
        style->Border.Radius = 5_px;
        theme.Normal.apply(style);
    }
    {
        auto style {styles.create<button>("button", {})};
        style->Border.Type          = element::border::type::Solid;
        style->Border.Size          = 4_px;
        style->Border.Radius        = 2_px;
        style->Text.Style           = {false, gfx::font::weight::Normal};
        style->Text.Font            = fntFam;
        style->Text.Size            = 32_px;
        style->Text.Alignment       = {gfx::horizontal_alignment::Centered, gfx::vertical_alignment::Middle};
        style->Text.Decoration.Size = {3_px};
        style->Text.AutoSize        = element::text::auto_size_mode::Always;
        style->Margin               = {5_px};
        style->Padding              = {1_px};

        auto hoverStyle {styles.create<button>("button", {.Hover = true})};
        *hoverStyle = *style;

        auto activeStyle {styles.create<button>("button", {.Active = true})};
        *activeStyle = *style;

        theme.Normal.apply(style);
        theme.Hover.apply(hoverStyle);
        theme.Active.apply(activeStyle);
    }
    {
        auto style {styles.create<cycle_button>("cycle_button", {})};
        style->Border.Size          = 3_px;
        style->Border.Radius        = 5_px;
        style->Text.Style           = {false, gfx::font::weight::Normal};
        style->Text.Font            = fntFam;
        style->Text.Size            = 32_px;
        style->Text.Alignment       = {gfx::horizontal_alignment::Centered, gfx::vertical_alignment::Middle};
        style->Text.Decoration.Size = {3_px};
        style->Text.AutoSize        = element::text::auto_size_mode::Always;
        style->Margin               = {5_px};
        style->Padding              = {1_px};

        auto hoverStyle {styles.create<cycle_button>("cycle_button", {.Hover = true})};
        *hoverStyle = *style;

        auto activeStyle {styles.create<cycle_button>("cycle_button", {.Active = true})};
        *activeStyle = *style;

        theme.Normal.apply(style);
        theme.Hover.apply(hoverStyle);
        theme.Active.apply(activeStyle);
    }
    {
        auto style {styles.create<slider>("slider", {})};
        style->Margin            = {2_px};
        style->Padding           = {2_px};
        style->ThumbClass        = "slider_thumb";
        style->Bar.Type          = element::bar::type::Continuous;
        style->Bar.Size          = 25_pct;
        style->Bar.Delay         = 100ms;
        style->Bar.Border.Size   = 2_px;
        style->Bar.Border.Radius = 5_px;

        theme.Normal.apply(style);
        theme.Hover.apply(style);
    }
    {
        auto style {styles.create<spinner>("spinner", {})};
        style->Border.Size    = 3_px;
        style->Border.Radius  = 5_px;
        style->Text.Style     = {false, gfx::font::weight::Normal};
        style->Text.Font      = fntFam;
        style->Text.Size      = 50_pct;
        style->Text.Alignment = {gfx::horizontal_alignment::Centered, gfx::vertical_alignment::Middle};
        style->Margin         = {5_px};
        style->Padding        = {1_px};
        style->NavArrowClass  = "nav_arrows";

        auto hoverStyle {styles.create<spinner>("spinner", {.Hover = true})};
        *hoverStyle = *style;

        auto activeStyle {styles.create<spinner>("spinner", {.Active = true})};
        *activeStyle = *style;

        theme.Normal.apply(style);
        theme.Hover.apply(hoverStyle);
        theme.Active.apply(activeStyle);
    }
    {
        auto style {styles.create<label>("label", {})};
        style->Border.Type    = element::border::type::Double;
        style->Border.Size    = 5_px;
        style->Border.Radius  = 2_px;
        style->Text.Style     = {false, gfx::font::weight::Normal};
        style->Text.Font      = fntFam;
        style->Text.Size      = 32_px;
        style->Text.Alignment = {gfx::horizontal_alignment::Left, gfx::vertical_alignment::Middle};
        style->Text.AutoSize  = element::text::auto_size_mode::OnlyShrink;

        theme.Normal.apply(style);
    }
    {
        auto style {styles.create<label>("label-small", {})};
        style->Margin         = {3_px};
        style->Border.Type    = element::border::type::Double;
        style->Border.Size    = 5_px;
        style->Border.Radius  = 2_px;
        style->Text.Style     = {false, gfx::font::weight::Normal};
        style->Text.Font      = fntFam;
        style->Text.Size      = 32_px;
        style->Text.Alignment = {gfx::horizontal_alignment::Centered, gfx::vertical_alignment::Middle};
        style->Text.AutoSize  = element::text::auto_size_mode::Always;
        theme.Normal.apply(style);
    }
    {
        auto style {styles.create<label>("tooltip-label", {})};
        style->Border.Type    = element::border::type::Solid;
        style->Border.Size    = 5_px;
        style->Text.Style     = {false, gfx::font::weight::Light};
        style->Text.Font      = fntFam;
        style->Text.Size      = 24_px;
        style->Text.Alignment = {gfx::horizontal_alignment::Centered, gfx::vertical_alignment::Middle};
        style->Text.AutoSize  = element::text::auto_size_mode::Always;
        theme.Normal.apply(style);
    }
    {
        auto style {styles.create<list_box>("list_box", {})};
        style->Border.Size                = 2_px;
        style->Border.Radius              = 5_px;
        style->ItemHeight                 = 5_pct;
        style->ItemClass                  = "list_items";
        style->VScrollBar.ThumbClass      = "scrollbar_thumb";
        style->VScrollBar.Bar.Type        = element::bar::type::Continuous;
        style->VScrollBar.Bar.Size        = 25_px;
        style->VScrollBar.Bar.Border.Size = 2_px;
        style->VScrollBar.Bar.Delay       = 250ms;

        auto hoverStyle {styles.create<list_box>("list_box", {.Hover = true})};
        *hoverStyle = *style;

        theme.Normal.apply(style);
        theme.Hover.apply(hoverStyle);
    }
    {
        auto style {styles.create<list_box>("list_box_log", {})};
        style->Border.Size                = 2_px;
        style->Border.Radius              = 5_px;
        style->ItemHeight                 = 20_pct;
        style->ItemClass                  = "list_items";
        style->VScrollBar.ThumbClass      = "scrollbar_thumb";
        style->VScrollBar.Bar.Type        = element::bar::type::Continuous;
        style->VScrollBar.Bar.Size        = 25_px;
        style->VScrollBar.Bar.Border.Size = 2_px;
        style->VScrollBar.Bar.Delay       = 250ms;

        auto hoverStyle {styles.create<list_box>("list_box_log", {.Hover = true})};
        *hoverStyle = *style;

        theme.Normal.apply(style);
        theme.Hover.apply(hoverStyle);
    }
    {
        auto style {styles.create<drop_down_list>("drop_down_list", {})};
        style->Border.Size                = 2_px;
        style->Border.Radius              = 5_px;
        style->Margin                     = {5_px};
        style->Padding                    = {5_px};
        style->DropShadow.Color           = color {0, 0, 0, 128};
        style->Text.Font                  = fntFam;
        style->Text.Size                  = 50_pct;
        style->Text.Alignment             = {gfx::horizontal_alignment::Left, gfx::vertical_alignment::Middle};
        style->Text.AutoSize              = element::text::auto_size_mode::Always;
        style->ItemClass                  = "list_items";
        style->ItemHeight                 = 75_pct;
        style->VScrollBar.ThumbClass      = "scrollbar_thumb";
        style->VScrollBar.Bar.Type        = element::bar::type::Continuous;
        style->VScrollBar.Bar.Size        = 20_pct;
        style->VScrollBar.Bar.Border.Size = 2_px;
        style->VScrollBar.Bar.Delay       = 250ms;

        auto hoverStyle {styles.create<drop_down_list>("drop_down_list", {.Hover = true})};
        *hoverStyle = *style;

        theme.Normal.apply(style);
        theme.Hover.apply(hoverStyle);
    }
    {
        auto style {styles.create<tab_container>("tab_container", {})};
        style->Padding        = 2_px;
        style->Border.Size    = 2_px;
        style->Border.Radius  = 5_px;
        style->TabBarHeight   = 5_pct;
        style->TabItemClass   = "tab_items";
        style->TabBarPosition = tab_container::position::Top;

        theme.Normal.apply(style);
    }
    {
        auto style {styles.create<accordion>("accordion", {})};
        style->Border.Size      = 3_px;
        style->Border.Radius    = 5_px;
        style->SectionBarHeight = 5_pct;
        style->SectionItemClass = "section_items";

        theme.Normal.apply(style);
    }
    {
        auto style {styles.create<checkbox>("checkbox", {})};
        style->Border.Size   = 3_px;
        style->Border.Radius = 5_px;
        style->Margin        = {5_px};
        style->Padding       = {2_px};
        style->Tick.Type     = element::tick::type::Rect;
        style->Tick.Size     = 95_pct;

        auto hoverStyle {styles.create<checkbox>("checkbox", {.Hover = true})};
        *hoverStyle = *style;

        auto activeStyle {styles.create<checkbox>("checkbox", {.Active = true})};
        *activeStyle = *style;

        theme.Normal.apply(style);
        theme.Hover.apply(hoverStyle);
        theme.Active.apply(activeStyle);
    }
    {
        auto style {styles.create<radio_button>("radio_button", {})};
        style->Border.Type   = element::border::type::Solid;
        style->Border.Size   = 3_px;
        style->Border.Radius = 5_px;
        style->Margin        = {5_px};
        style->Padding       = {5_px};
        style->Cursor        = "cursor2";
        style->Tick.Type     = element::tick::type::Disc;
        style->Tick.Size     = 100_pct;

        auto hoverStyle {styles.create<radio_button>("radio_button", {.Hover = true})};
        *hoverStyle = *style;

        auto activeStyle {styles.create<radio_button>("radio_button", {.Active = true})};
        *activeStyle = *style;

        theme.Normal.apply(style);
        theme.Hover.apply(hoverStyle);
        theme.Active.apply(activeStyle);
    }
    {
        auto style {styles.create<grid_view>("grid_view", {})};
        style->Border.Size                = 3_px;
        style->Border.Radius              = 5_px;
        style->Padding                    = {2_px};
        style->DropShadow.Color           = color {0, 0, 0, 128};
        style->RowHeight                  = 10_pct;
        style->HeaderClass                = "header_items";
        style->RowClass                   = "row_items";
        style->AutoSizeColumns            = false;
        style->VScrollBar.ThumbClass      = "scrollbar_thumb";
        style->VScrollBar.Bar.Type        = element::bar::type::Continuous;
        style->VScrollBar.Bar.Size        = 25_px;
        style->VScrollBar.Bar.Border.Size = 3_px;
        style->VScrollBar.Bar.Delay       = 250ms;

        auto hoverStyle {styles.create<grid_view>("grid_view", {.Hover = true})};
        *hoverStyle = *style;

        theme.Normal.apply(style);
        theme.Hover.apply(hoverStyle);

        ///
        auto style2 {styles.create<grid_view>("grid_view2", {})};
        *style2           = *style;
        style2->RowHeight = 50_pct;

        auto hoverStyle2 {styles.create<grid_view>("grid_view2", {.Hover = true})};
        *hoverStyle2 = *style2;

        theme.Normal.apply(style2);
        theme.Hover.apply(hoverStyle2);
    }
    {
        auto style {styles.create<tooltip>("tooltip", {})};
        style->Delay  = 500ms;
        style->FadeIn = 150ms;
        //   theme.Normal.apply(style);
    }
    {
        auto style {styles.create<text_box>("text_box", {})};
        style->Border.Type     = element::border::type::Solid;
        style->Border.Size     = 3_px;
        style->Border.Radius   = 5_px;
        style->Text.Style      = {false, gfx::font::weight::Normal};
        style->Text.Font       = fntFam;
        style->Text.Size       = 50_pct;
        style->Text.Alignment  = {gfx::horizontal_alignment::Left, gfx::vertical_alignment::Middle};
        style->Padding         = {2_px};
        style->Caret.BlinkRate = 500ms;

        auto hoverStyle {styles.create<text_box>("text_box", {.Hover = true})};
        *hoverStyle = *style;

        theme.Normal.apply(style);
        theme.Hover.apply(hoverStyle);
    }
    // items
    {
        auto style {styles.create<thumb_style>("slider_thumb", {}, {})};
        style->Thumb.Type          = element::thumb::type::Rectangle;
        style->Thumb.LongSide      = 25_pct;
        style->Thumb.ShortSide     = 80_pct;
        style->Thumb.Border.Size   = 3_px;
        style->Thumb.Border.Radius = 5_px;

        auto hoverStyle {styles.create<thumb_style>("slider_thumb", {.Hover = true})};
        hoverStyle->Thumb = style->Thumb;

        auto activeStyle {styles.create<thumb_style>("slider_thumb", {.Active = true})};
        activeStyle->Thumb = style->Thumb;

        theme.Normal.apply(style);
        theme.Hover.apply(hoverStyle);
        theme.Active.apply(activeStyle);
    }
    {
        auto style {styles.create<thumb_style>("scrollbar_thumb", {}, {})};
        style->Thumb.Type        = element::thumb::type::Rectangle;
        style->Thumb.LongSide    = 25_pct;
        style->Thumb.ShortSide   = 80_pct;
        style->Thumb.Border.Size = 2_px;

        auto hoverStyle {styles.create<thumb_style>("scrollbar_thumb", {.Hover = true})};
        hoverStyle->Thumb = style->Thumb;

        auto activeStyle {styles.create<thumb_style>("scrollbar_thumb", {.Active = true})};
        activeStyle->Thumb = style->Thumb;

        theme.Normal.apply(style);
        theme.Hover.apply(hoverStyle);
        theme.Active.apply(activeStyle);
    }
    {
        auto style {styles.create<item_style>("list_items", {}, {})};
        style->Item.Padding        = {2_px};
        style->Item.Text.Style     = {false, gfx::font::weight::Normal};
        style->Item.Text.Font      = fntFam;
        style->Item.Text.Size      = 24_px;
        style->Item.Text.Alignment = {gfx::horizontal_alignment::Left, gfx::vertical_alignment::Middle};
        style->Item.Text.AutoSize  = element::text::auto_size_mode::Always;
        style->Item.Border.Size    = 5_px;

        auto hoverStyle {styles.create<item_style>("list_items", {.Hover = true})};
        hoverStyle->Item = style->Item;

        auto activeStyle {styles.create<item_style>("list_items", {.Active = true})};
        activeStyle->Item = style->Item;

        theme.Normal.apply(style);
        theme.Hover.apply(hoverStyle);
        theme.Active.apply(activeStyle);
    }
    {
        auto style {styles.create<item_style>("tab_items", {}, {})};
        style->Item.Padding        = {2_px};
        style->Item.Text.Style     = {false, gfx::font::weight::Bold};
        style->Item.Text.Font      = fntFam;
        style->Item.Text.Size      = 32_px;
        style->Item.Text.Alignment = {gfx::horizontal_alignment::Centered, gfx::vertical_alignment::Middle};
        style->Item.Text.AutoSize  = element::text::auto_size_mode::Always;
        style->Item.Border.Size    = 2_px;

        auto hoverStyle {styles.create<item_style>("tab_items", {.Hover = true})};
        hoverStyle->Item = style->Item;

        auto activeStyle {styles.create<item_style>("tab_items", {.Active = true})};
        activeStyle->Item = style->Item;

        theme.Normal.apply(style);
        theme.Hover.apply(hoverStyle);
        theme.Active.apply(activeStyle);
    }
    {
        auto style {styles.create<item_style>("section_items", {}, {})};
        style->Item.Padding        = {2_px};
        style->Item.Text.Style     = {false, gfx::font::weight::Normal};
        style->Item.Text.Font      = fntFam;
        style->Item.Text.Size      = 32_px;
        style->Item.Text.Alignment = {gfx::horizontal_alignment::Centered, gfx::vertical_alignment::Middle};
        style->Item.Text.AutoSize  = element::text::auto_size_mode::Always;
        style->Item.Border.Size    = 3_px;

        auto hoverStyle {styles.create<item_style>("section_items", {.Hover = true})};
        hoverStyle->Item = style->Item;

        auto activeStyle {styles.create<item_style>("section_items", {.Active = true})};
        activeStyle->Item = style->Item;

        theme.Normal.apply(style);
        theme.Hover.apply(hoverStyle);
        theme.Active.apply(activeStyle);
    }
    {
        auto style {styles.create<nav_arrows_style>("nav_arrows", {}, {})};
        style->NavArrow.Height        = {0.75f, length::type::Relative};
        style->NavArrow.Width         = {0.25f, length::type::Relative};
        style->NavArrow.Border.Size   = 3_px;
        style->NavArrow.Border.Radius = 0_px;

        auto hoverStyle {styles.create<nav_arrows_style>("nav_arrows", {.Hover = true})};
        hoverStyle->NavArrow = style->NavArrow;

        auto activeStyle {styles.create<nav_arrows_style>("nav_arrows", {.Active = true})};
        activeStyle->NavArrow = style->NavArrow;

        theme.Normal.apply(style);
        theme.Hover.apply(hoverStyle);
        theme.Active.apply(activeStyle);
    }
    {
        auto style {styles.create<item_style>("header_items", {}, {})};
        style->Item.Padding        = {5_px};
        style->Item.Text.Style     = {false, gfx::font::weight::Bold};
        style->Item.Text.Font      = fntFam;
        style->Item.Text.AutoSize  = element::text::auto_size_mode::Always;
        style->Item.Text.Size      = 32_px;
        style->Item.Text.Alignment = {gfx::horizontal_alignment::Centered, gfx::vertical_alignment::Middle};
        style->Item.Border.Size    = 5_px;

        auto hoverStyle {styles.create<item_style>("header_items", {.Hover = true})};
        hoverStyle->Item = style->Item;

        auto activeStyle {styles.create<item_style>("header_items", {.Active = true})};
        activeStyle->Item = style->Item;

        theme.Normal.apply(style);
        theme.Hover.apply(hoverStyle);
        theme.Active.apply(activeStyle);
    }
    {
        auto style {styles.create<item_style>("row_items", {}, {})};
        style->Item.Padding        = {5_px};
        style->Item.Text.Style     = {false, gfx::font::weight::Normal};
        style->Item.Text.Font      = fntFam;
        style->Item.Text.AutoSize  = element::text::auto_size_mode::Always;
        style->Item.Text.Size      = 32_px;
        style->Item.Text.Alignment = {gfx::horizontal_alignment::Right, gfx::vertical_alignment::Middle};
        style->Item.Border.Size    = 3_px;

        auto hoverStyle {styles.create<item_style>("row_items", {.Hover = true})};
        hoverStyle->Item = style->Item;

        auto activeStyle {styles.create<item_style>("row_items", {.Active = true})};
        activeStyle->Item = style->Item;

        theme.Normal.apply(style);
        theme.Hover.apply(hoverStyle);
        theme.Active.apply(activeStyle);
    }
}

auto load_themes() -> std::map<std::string, color_themes>
{
    std::map<std::string, color_themes> retValue;

    if (io::is_file("themes.json")) {
        data::config::object themes {};
        themes.load("themes.json");

        for (auto const& [k, v] : themes) {
            color_theme normal;
            normal.Background = colors::LightSkyBlue;
            normal.Border     = colors::MidnightBlue;
            normal.Shadow     = color {0, 0, 0, 128};
            normal.Foreground = colors::Black;
            normal.List       = colors::LightGray;
            normal.Container  = colors::LavenderBlush;
            normal.Label      = colors::Gray;

            color_theme hover;
            hover.Background = normal.Background;
            hover.Border     = colors::IndianRed;
            hover.Shadow     = color {0, 0, 0, 128};
            hover.Foreground = hover.Border;
            hover.List       = colors::LightBlue;
            hover.Container  = normal.Container;

            color_theme active;
            active.Background = hover.Border;
            active.Border     = colors::ForestGreen;
            active.Shadow     = color {0, 0, 0, 128};
            active.Foreground = colors::White;
            active.List       = normal.Background;
            active.Container  = normal.Container;

            data::config::object theme {v.as<data::config::object>()};

            auto getColor {[&](color& target, std::string const& group, std::string const& name) {
                if (!theme.has(group, name)) { return; }

                if (theme[group][name].is<std::string>()) {
                    target = color::FromString(theme[group][name].as<std::string>());
                } else {
                    target = theme[group][name].as<color>();
                }
            }};

            getColor(normal.Background, "normal", "Background");
            getColor(normal.Border, "normal", "Border");
            getColor(normal.Shadow, "normal", "Shadow");
            getColor(normal.Foreground, "normal", "Foreground");
            getColor(normal.List, "normal", "List");
            getColor(normal.Container, "normal", "Container");
            getColor(normal.Label, "normal", "Label");

            getColor(hover.Background, "hover", "Background");
            getColor(hover.Border, "hover", "Border");
            getColor(hover.Shadow, "hover", "Shadow");
            getColor(hover.Foreground, "hover", "Foreground");
            getColor(hover.List, "hover", "List");
            getColor(hover.Container, "hover", "Container");

            getColor(active.Background, "active", "Background");
            getColor(active.Border, "active", "Border");
            getColor(active.Shadow, "active", "Shadow");
            getColor(active.Foreground, "active", "Foreground");
            getColor(active.List, "active", "List");
            getColor(active.Container, "active", "Container");

            color_themes ct;
            ct.TableBackgroundA = colors::OliveDrab;
            ct.TableBackgroundB = colors::DarkGreen;

            getColor(ct.TableBackgroundA, "table", "A");
            getColor(ct.TableBackgroundB, "table", "B");

            ct.Normal   = normal;
            ct.Hover    = hover;
            ct.Active   = active;
            retValue[k] = ct;
        }
    }

    return retValue;
}

}
