#include "Themes.hpp"

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

auto load_themes() -> std::map<std::string, color_themes>
{
    std::map<std::string, color_themes> retValue;

    data::config::object themes {};
    if (themes.load("themes.json") != load_status::Ok) { return retValue; }

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

        getColor(normal.Background, "normal", "background");
        getColor(normal.Border, "normal", "border");
        getColor(normal.Shadow, "normal", "shadow");
        getColor(normal.Foreground, "normal", "foreground");
        getColor(normal.List, "normal", "list");
        getColor(normal.Container, "normal", "container");
        getColor(normal.Label, "normal", "label");

        getColor(hover.Background, "hover", "background");
        getColor(hover.Border, "hover", "border");
        getColor(hover.Shadow, "hover", "shadow");
        getColor(hover.Foreground, "hover", "foreground");
        getColor(hover.List, "hover", "list");
        getColor(hover.Container, "hover", "container");

        getColor(active.Background, "active", "background");
        getColor(active.Border, "active", "border");
        getColor(active.Shadow, "active", "shadow");
        getColor(active.Foreground, "active", "foreground");
        getColor(active.List, "active", "list");
        getColor(active.Container, "active", "container");

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

    return retValue;
}
}
