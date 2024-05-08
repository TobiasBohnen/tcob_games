#include "UIHelper.hpp"

////////////////////////////////////////////////////////////

void color_theme::apply(std::shared_ptr<button::style> const& style) const
{
    style->Background            = Background;
    style->Border.Background     = Border;
    style->DropShadow.Color      = DropShadow;
    style->Text.Color            = Text;
    style->Text.Decoration.Color = TextDecoration;
    style->Text.Shadow.Color     = TextShadow;
}

void color_theme::apply(std::shared_ptr<checkbox::style> const& style) const
{
    style->Background        = Background;
    style->Border.Background = Border;
    style->DropShadow.Color  = DropShadow;
    style->Tick.Foreground   = Tick;
}

void color_theme::apply(std::shared_ptr<cycle_button::style> const& style) const
{
    style->Background            = Background;
    style->Border.Background     = Border;
    style->DropShadow.Color      = DropShadow;
    style->Text.Color            = Text;
    style->Text.Decoration.Color = TextDecoration;
    style->Text.Shadow.Color     = TextShadow;
}

void color_theme::apply(std::shared_ptr<grid_view::style> const& style) const
{
    style->Background                      = Background;
    style->Border.Background               = Border;
    style->DropShadow.Color                = DropShadow;
    style->VScrollBar.Bar.HigherBackground = BarHigher;
    style->VScrollBar.Bar.LowerBackground  = BarLower;
}

void color_theme::apply(std::shared_ptr<image_box::style> const& style) const
{
    style->Background        = Background;
    style->Border.Background = Border;
    style->DropShadow.Color  = DropShadow;
}

void color_theme::apply(std::shared_ptr<label::style> const& style) const
{
    style->Text.Color            = Text;
    style->Text.Decoration.Color = TextDecoration;
    style->Text.Shadow.Color     = TextShadow;
}

void color_theme::apply(std::shared_ptr<list_box::style> const& style) const
{
    style->Background                      = Background;
    style->Border.Background               = Border;
    style->DropShadow.Color                = DropShadow;
    style->VScrollBar.Bar.HigherBackground = BarHigher;
    style->VScrollBar.Bar.LowerBackground  = BarLower;
}

void color_theme::apply(std::shared_ptr<progress_bar::style> const& style) const
{
    style->Background           = Background;
    style->Border.Background    = Border;
    style->DropShadow.Color     = DropShadow;
    style->Bar.HigherBackground = BarHigher;
    style->Bar.LowerBackground  = BarLower;
}

void color_theme::apply(std::shared_ptr<radio_button::style> const& style) const
{
    style->Background        = Background;
    style->Border.Background = Border;
    style->DropShadow.Color  = DropShadow;
    style->Tick.Foreground   = Tick;
}

void color_theme::apply(std::shared_ptr<slider::style> const& style) const
{
    style->Bar.HigherBackground = BarHigher;
    style->Bar.LowerBackground  = BarLower;
}

void color_theme::apply(std::shared_ptr<spinner::style> const& style) const
{
    style->Background            = Background;
    style->Border.Background     = Border;
    style->DropShadow.Color      = DropShadow;
    style->Text.Color            = Text;
    style->Text.Decoration.Color = TextDecoration;
    style->Text.Shadow.Color     = TextShadow;
}

void color_theme::apply(std::shared_ptr<text_box::style> const& style) const
{
    style->Background            = Background;
    style->Border.Background     = Border;
    style->DropShadow.Color      = DropShadow;
    style->Text.Color            = Text;
    style->Text.Decoration.Color = TextDecoration;
    style->Text.Shadow.Color     = TextShadow;
    style->Caret.Color           = Caret;
}

void color_theme::apply(std::shared_ptr<toggle::style> const& style) const
{
    style->Background        = Background;
    style->Border.Background = Border;
    style->DropShadow.Color  = DropShadow;
    style->Tick.Foreground   = Tick;
}

void color_theme::apply(std::shared_ptr<panel::style> const& style) const
{
    style->Background                      = Container;
    style->Border.Background               = Border;
    style->DropShadow.Color                = DropShadow;
    style->VScrollBar.Bar.HigherBackground = BarHigher;
    style->VScrollBar.Bar.LowerBackground  = BarLower;
    style->HScrollBar.Bar.HigherBackground = BarHigher;
    style->HScrollBar.Bar.LowerBackground  = BarLower;
}

void color_theme::apply(std::shared_ptr<tab_container::style> const& style) const
{
    style->Background        = Container;
    style->Border.Background = Border;
    style->DropShadow.Color  = DropShadow;
}

void color_theme::apply(std::shared_ptr<accordion::style> const& style) const
{
    style->Background        = Container;
    style->Border.Background = Border;
    style->DropShadow.Color  = DropShadow;
}

void color_theme::apply(std::shared_ptr<thumb_style> const& style) const
{
    style->Thumb.Background        = Thumb;
    style->Thumb.Border.Background = Border;
}

void color_theme::apply(std::shared_ptr<item_style> const& style) const
{
    style->Item.Background            = Item;
    style->Item.Border.Background     = Border;
    style->Item.Text.Color            = Text;
    style->Item.Text.Decoration.Color = TextDecoration;
    style->Item.Text.Shadow.Color     = TextShadow;
}

void color_theme::apply(std::shared_ptr<nav_arrows_style> const& style) const
{
    style->NavArrow.Foreground        = NavArrow;
    style->NavArrow.IncBackground     = NavArrowInc;
    style->NavArrow.DecBackground     = NavArrowDec;
    style->NavArrow.Border.Background = Border;
}

void create_styles(assets::group& resGrp, style_collection& styles)
{
    using namespace tcob::literals;
    using namespace std::chrono_literals;
    namespace element = element;

    color_theme normal;
    normal.Background     = colors::ForestGreen;
    normal.Border         = colors::MidnightBlue;
    normal.DropShadow     = color {0, 0, 0, 128};
    normal.Text           = colors::Black;
    normal.BarLower       = colors::Blue;
    normal.BarHigher      = colors::Red;
    normal.Caret          = colors::DarkGray;
    normal.Item           = colors::LightGray;
    normal.NavArrowInc    = colors::White;
    normal.NavArrowDec    = colors::White;
    normal.NavArrow       = colors::Blue;
    normal.TextShadow     = colors::Black;
    normal.TextDecoration = colors::Red;
    normal.Thumb          = colors::LightGreen;
    normal.Tick           = colors::Red;
    normal.Container      = colors::DarkSlateGray;

    color_theme hover;
    hover.Background     = normal.Background;
    hover.Border         = colors::IndianRed;
    hover.DropShadow     = color {0, 0, 0, 128};
    hover.Text           = colors::IndianRed;
    hover.BarLower       = colors::Blue;
    hover.BarHigher      = colors::Red;
    hover.Caret          = colors::DarkGray;
    hover.Item           = colors::LightBlue;
    hover.NavArrowInc    = colors::White;
    hover.NavArrowDec    = colors::White;
    hover.NavArrow       = colors::Blue;
    hover.TextShadow     = colors::Black;
    hover.TextDecoration = colors::Red;
    hover.Thumb          = colors::LightGreen;
    hover.Tick           = colors::Red;
    hover.Container      = normal.Container;

    color_theme active;
    active.Background     = hover.Border;
    active.Border         = hover.Background;
    active.DropShadow     = color {0, 0, 0, 128};
    active.Text           = colors::White;
    active.BarLower       = colors::Blue;
    active.BarHigher      = colors::Red;
    active.Caret          = colors::DarkGray;
    active.Item           = colors::Indigo;
    active.NavArrowInc    = colors::White;
    active.NavArrowDec    = colors::White;
    active.NavArrow       = colors::Blue;
    active.TextShadow     = colors::White;
    active.TextDecoration = colors::Red;
    active.Thumb          = colors::LightGreen;
    active.Tick           = colors::Red;
    active.Container      = normal.Container;

    {
        auto style {styles.create<panel>("panel", {})};
        style->Border.Size      = 3_px;
        style->Border.Radius    = 5_px;
        style->Margin           = {5_px};
        style->Padding          = {5_px};
        style->DropShadow.Color = color {0, 0, 0, 128};

        normal.apply(style);
    }
    {
        auto style {styles.create<button>("button", {})};
        style->Border.Type          = element::border::type::Solid;
        style->Border.Size          = 3_px;
        style->Border.Radius        = 5_px;
        style->Text.Style           = {false, gfx::font::weight::Normal};
        style->Text.Font            = resGrp.get<gfx::font_family>("Poppins");
        style->Text.Size            = 10_px;
        style->Text.Shadow.OffsetX  = 0_px;
        style->Text.Shadow.OffsetY  = 1_px;
        style->Text.Alignment       = {gfx::horizontal_alignment::Centered, gfx::vertical_alignment::Middle};
        style->Text.Decoration.Size = {3_px};
        style->Text.AutoSize        = element::text::auto_size_mode::Always;
        style->Margin               = {5_px};
        style->Padding              = {1_px};

        auto hoverStyle {styles.create<button>("button", {.Hover = true})};
        *hoverStyle = *style;

        auto activeStyle {styles.create<button>("button", {.Active = true})};
        *activeStyle = *style;

        normal.apply(style);
        hover.apply(hoverStyle);
        active.apply(activeStyle);
    }

    {
        auto style {styles.create<slider>("slider", {})};
        style->Margin            = {2_px};
        style->Padding           = {2_px};
        style->ThumbClass        = "slider_thumb";
        style->Bar.Type          = element::bar::type::Continuous;
        style->Bar.Size          = 25_pct;
        style->Bar.Delay         = 100ms;
        style->Bar.Border.Size   = 3_px;
        style->Bar.Border.Radius = 5_px;

        normal.apply(style);
    }
    {
        auto style {styles.create<label>("label", {})};
        style->Text.Style     = {false, gfx::font::weight::Normal};
        style->Text.Font      = resGrp.get<gfx::font_family>("Poppins");
        style->Text.Size      = 50_pct;
        style->Text.Alignment = {gfx::horizontal_alignment::Left, gfx::vertical_alignment::Middle};
        style->Text.AutoSize  = element::text::auto_size_mode::OnlyShrink;

        normal.apply(style);
    }
    {
        auto style {styles.create<label>("label-small", {})};
        style->Text.Style     = {false, gfx::font::weight::Normal};
        style->Text.Font      = resGrp.get<gfx::font_family>("Poppins");
        style->Text.Size      = 16_px;
        style->Text.Alignment = {gfx::horizontal_alignment::Centered, gfx::vertical_alignment::Middle};
        style->Text.AutoSize  = element::text::auto_size_mode::OnlyShrink;
        normal.apply(style);
        style->Text.Color = colors::White;
    }
    {
        auto style {styles.create<list_box>("list_box", {})};
        style->Border.Size                = 3_px;
        style->Border.Radius              = 5_px;
        style->Margin                     = {5_px};
        style->Padding                    = {5_px};
        style->DropShadow.Color           = color {0, 0, 0, 128};
        style->ItemHeight                 = 10_pct;
        style->ItemClass                  = "list_items";
        style->VScrollBar.ThumbClass      = "scrollbar_thumb";
        style->VScrollBar.Bar.Type        = element::bar::type::Continuous;
        style->VScrollBar.Bar.Size        = 20_pct;
        style->VScrollBar.Bar.Border.Size = 3_px;
        style->VScrollBar.Bar.Delay       = 250ms;

        auto hoverStyle {styles.create<list_box>("list_box", {.Hover = true})};
        *hoverStyle = *style;

        normal.apply(style);
        hover.apply(hoverStyle);
    }

    {
        auto style {styles.create<thumb_style>("slider_thumb", {}, {})};
        style->Thumb.Type          = element::thumb::type::Rect;
        style->Thumb.LongSide      = 25_pct;
        style->Thumb.ShortSide     = 80_pct;
        style->Thumb.Border.Size   = 3_px;
        style->Thumb.Border.Radius = 5_px;

        auto hoverStyle {styles.create<thumb_style>("slider_thumb", {.Hover = true})};
        hoverStyle->Thumb = style->Thumb;

        auto activeStyle {styles.create<thumb_style>("slider_thumb", {.Active = true})};
        activeStyle->Thumb = style->Thumb;

        normal.apply(style);
        hover.apply(hoverStyle);
        active.apply(activeStyle);
    }
    {
        auto style {styles.create<thumb_style>("scrollbar_thumb", {}, {})};
        style->Thumb.Type        = element::thumb::type::Rect;
        style->Thumb.LongSide    = 25_pct;
        style->Thumb.ShortSide   = 80_pct;
        style->Thumb.Border.Size = 3_px;

        auto hoverStyle {styles.create<thumb_style>("scrollbar_thumb", {.Hover = true})};
        hoverStyle->Thumb = style->Thumb;

        auto activeStyle {styles.create<thumb_style>("scrollbar_thumb", {.Active = true})};
        activeStyle->Thumb = style->Thumb;

        normal.apply(style);
        hover.apply(hoverStyle);
        active.apply(activeStyle);
    }
    {
        auto style {styles.create<item_style>("list_items", {}, {})};
        style->Item.Padding        = {2_px};
        style->Item.Text.Style     = {false, gfx::font::weight::Normal};
        style->Item.Text.Font      = resGrp.get<gfx::font_family>("Poppins");
        style->Item.Text.Size      = 50_pct;
        style->Item.Text.Alignment = {gfx::horizontal_alignment::Left, gfx::vertical_alignment::Middle};
        style->Item.Border.Size    = 3_px;

        auto hoverStyle {styles.create<item_style>("list_items", {.Hover = true})};
        hoverStyle->Item = style->Item;

        auto activeStyle {styles.create<item_style>("list_items", {.Active = true})};
        activeStyle->Item = style->Item;

        normal.apply(style);
        hover.apply(hoverStyle);
        active.apply(activeStyle);
    }
}
