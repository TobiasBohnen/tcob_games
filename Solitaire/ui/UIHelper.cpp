#include "UIHelper.hpp"

namespace solitaire {

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
    style->Background            = LabelBackground;
    style->Border.Background     = Border;
    style->DropShadow.Color      = DropShadow;
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

void create_styles(color_themes const& theme, assets::group& resGrp, style_collection& styles)
{
    using namespace tcob::literals;
    using namespace std::chrono_literals;
    namespace element = element;

    {
        auto style {styles.create<panel>("panel", {})};
        style->Border.Size      = 2_px;
        style->Border.Radius    = 5_px;
        style->Margin           = {5_px};
        style->Padding          = {5_px};
        style->DropShadow.Color = color {0, 0, 0, 128};

        theme.Normal.apply(style);
    }
    {
        auto style {styles.create<button>("button", {})};
        style->Border.Type          = element::border::type::Solid;
        style->Border.Size          = 2_px;
        style->Border.Radius        = 2_px;
        style->Text.Style           = {false, gfx::font::weight::Normal};
        style->Text.Font            = resGrp.get<gfx::font_family>("Poppins");
        style->Text.Size            = 10_px;
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
        auto style {styles.create<label>("label", {})};
        style->Text.Style     = {false, gfx::font::weight::Normal};
        style->Text.Font      = resGrp.get<gfx::font_family>("Poppins");
        style->Text.Size      = 50_pct;
        style->Text.Alignment = {gfx::horizontal_alignment::Left, gfx::vertical_alignment::Middle};
        style->Text.AutoSize  = element::text::auto_size_mode::OnlyShrink;

        theme.Normal.apply(style);
        style->Text.Shadow.Color = colors::Transparent;
    }
    {
        auto style {styles.create<label>("label-small", {})};
        style->Text.Style     = {false, gfx::font::weight::Normal};
        style->Text.Font      = resGrp.get<gfx::font_family>("Poppins");
        style->Text.Size      = 16_px;
        style->Text.Alignment = {gfx::horizontal_alignment::Centered, gfx::vertical_alignment::Middle};
        style->Text.AutoSize  = element::text::auto_size_mode::Always;
        theme.Normal.apply(style);
        style->Text.Color       = colors::White;
        style->DropShadow.Color = colors::Transparent;
        style->Margin           = {5_px};
    }
    {
        auto style {styles.create<list_box>("list_box", {})};
        style->Border.Size                = 2_px;
        style->Border.Radius              = 5_px;
        style->Margin                     = {5_px};
        style->Padding                    = {5_px};
        style->DropShadow.Color           = color {0, 0, 0, 128};
        style->ItemHeight                 = 10_pct;
        style->ItemClass                  = "list_items";
        style->VScrollBar.ThumbClass      = "scrollbar_thumb";
        style->VScrollBar.Bar.Type        = element::bar::type::Continuous;
        style->VScrollBar.Bar.Size        = 15_pct;
        style->VScrollBar.Bar.Border.Size = 2_px;
        style->VScrollBar.Bar.Delay       = 250ms;

        auto hoverStyle {styles.create<list_box>("list_box", {.Hover = true})};
        *hoverStyle = *style;

        theme.Normal.apply(style);
        theme.Hover.apply(hoverStyle);
    }
    {
        auto style {styles.create<list_box>("list_box_games", {})};
        style->Border.Size                = 2_px;
        style->Border.Radius              = 5_px;
        style->Margin                     = {5_px};
        style->Padding                    = {5_px};
        style->DropShadow.Color           = color {0, 0, 0, 128};
        style->ItemHeight                 = 5_pct;
        style->ItemClass                  = "list_items";
        style->VScrollBar.ThumbClass      = "scrollbar_thumb";
        style->VScrollBar.Bar.Type        = element::bar::type::Continuous;
        style->VScrollBar.Bar.Size        = 3_pct;
        style->VScrollBar.Bar.Border.Size = 2_px;
        style->VScrollBar.Bar.Delay       = 250ms;

        auto hoverStyle {styles.create<list_box>("list_box_games", {.Hover = true})};
        *hoverStyle = *style;

        theme.Normal.apply(style);
        theme.Hover.apply(hoverStyle);
    }
    {
        auto style {styles.create<tab_container>("tab_container", {})};
        style->Border.Size      = 2_px;
        style->Border.Radius    = 5_px;
        style->Margin           = {5_px};
        style->Padding          = {5_px};
        style->DropShadow.Color = color {0, 0, 0, 128};
        style->TabBarHeight     = 5_pct;
        style->TabItemClass     = "tab_items";
        style->MaxTabs          = 8;
        theme.Normal.apply(style);
    }

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
        style->Item.Text.Font      = resGrp.get<gfx::font_family>("Poppins");
        style->Item.Text.Size      = 50_pct;
        style->Item.Text.Alignment = {gfx::horizontal_alignment::Left, gfx::vertical_alignment::Middle};
        style->Item.Border.Size    = 2_px;

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
        style->Item.Padding        = {5_px};
        style->Item.Text.Style     = {false, gfx::font::weight::Normal};
        style->Item.Text.Font      = resGrp.get<gfx::font_family>("Poppins");
        style->Item.Text.Size      = 25_pct;
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
}

auto load_themes() -> std::map<std::string, color_themes>
{
    std::map<std::string, color_themes> retValue;

    if (io::is_file("theme.ini")) {
        data::config::object themes {};
        themes.load("theme.ini");

        for (auto const& [k, v] : themes) {
            color_theme normal;
            normal.Background      = colors::LightSkyBlue;
            normal.Border          = colors::MidnightBlue;
            normal.DropShadow      = color {0, 0, 0, 128};
            normal.Text            = colors::Black;
            normal.BarLower        = colors::SlateGray;
            normal.BarHigher       = normal.BarHigher;
            normal.Caret           = colors::DarkGray;
            normal.Item            = colors::LightGray;
            normal.Thumb           = colors::DodgerBlue;
            normal.Container       = colors::LavenderBlush;
            normal.LabelBackground = colors::DimGray;

            color_theme hover;
            hover.Background = normal.Background;
            hover.Border     = colors::IndianRed;
            hover.DropShadow = color {0, 0, 0, 128};
            hover.Text       = hover.Border;
            hover.BarLower   = normal.BarLower;
            hover.BarHigher  = normal.BarHigher;
            hover.Caret      = normal.Caret;
            hover.Item       = colors::LightBlue;
            hover.Thumb      = normal.Thumb;
            hover.Container  = normal.Container;

            color_theme active;
            active.Background = hover.Border;
            active.Border     = colors::ForestGreen;
            active.DropShadow = color {0, 0, 0, 128};
            active.Text       = colors::White;
            active.BarLower   = normal.BarLower;
            active.BarHigher  = normal.BarHigher;
            active.Caret      = normal.Caret;
            active.Item       = normal.Background;
            active.Thumb      = normal.Thumb;
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
            getColor(normal.DropShadow, "normal", "DropShadow");
            getColor(normal.Text, "normal", "Text");
            getColor(normal.Caret, "normal", "Text");
            getColor(normal.BarLower, "normal", "BarLower");
            getColor(normal.BarHigher, "normal", "BarHigher");
            getColor(normal.Tick, "normal", "Tick");
            getColor(normal.Item, "normal", "Item");
            getColor(normal.Thumb, "normal", "Thumb");
            getColor(normal.Container, "normal", "Container");
            getColor(normal.LabelBackground, "normal", "LabelBackground");

            getColor(hover.Background, "hover", "Background");
            getColor(hover.Border, "hover", "Border");
            getColor(hover.DropShadow, "hover", "DropShadow");
            getColor(hover.Text, "hover", "Text");
            getColor(hover.Caret, "hover", "Text");
            getColor(hover.BarLower, "hover", "BarLower");
            getColor(hover.BarHigher, "hover", "BarHigher");
            getColor(hover.Tick, "hover", "Tick");
            getColor(hover.Item, "hover", "Item");
            getColor(hover.Thumb, "hover", "Thumb");
            getColor(hover.Container, "hover", "Container");

            getColor(active.Background, "active", "Background");
            getColor(active.Border, "active", "Border");
            getColor(active.DropShadow, "active", "DropShadow");
            getColor(active.Text, "active", "Text");
            getColor(active.Caret, "active", "Text");
            getColor(active.BarLower, "active", "BarLower");
            getColor(active.BarHigher, "active", "BarHigher");
            getColor(active.Tick, "active", "Tick");
            getColor(active.Item, "active", "Item");
            getColor(active.Thumb, "active", "Thumb");
            getColor(active.Container, "active", "Container");

            retValue[k] = {.Normal = normal, .Hover = hover, .Active = active};
        }
    }

    return retValue;
}

}
