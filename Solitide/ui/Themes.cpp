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
    style->Background                       = Background;
    style->Border.Background                = Border;
    style->DropShadow.Color                 = Shadow;
    style->Text.Color                       = Foreground;
    style->VScrollBar.Bar.HigherBackground  = Foreground;
    style->VScrollBar.Bar.LowerBackground   = Foreground;
    style->VScrollBar.Bar.Border.Background = Border;
}

void color_theme::apply(std::shared_ptr<grid_view::style> const& style) const
{
    style->Background                       = Background;
    style->Border.Background                = Border;
    style->DropShadow.Color                 = Shadow;
    style->VScrollBar.Bar.HigherBackground  = Foreground;
    style->VScrollBar.Bar.LowerBackground   = Foreground;
    style->VScrollBar.Bar.Border.Background = Border;
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
    style->Background                       = Background;
    style->Border.Background                = Border;
    style->VScrollBar.Bar.HigherBackground  = Foreground;
    style->VScrollBar.Bar.LowerBackground   = Foreground;
    style->VScrollBar.Bar.Border.Background = Border;
}

void color_theme::apply(std::shared_ptr<progress_bar::style> const& style) const
{
    style->Background            = Background;
    style->Border.Background     = Border;
    style->DropShadow.Color      = Shadow;
    style->Bar.HigherBackground  = Background;
    style->Bar.LowerBackground   = Background;
    style->Bar.Border.Background = Border;
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
    style->Bar.HigherBackground  = Background;
    style->Bar.LowerBackground   = Background;
    style->Bar.Border.Background = Border;
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
    style->Background                       = Container;
    style->Border.Background                = Border;
    style->VScrollBar.Bar.HigherBackground  = Foreground;
    style->VScrollBar.Bar.LowerBackground   = Foreground;
    style->VScrollBar.Bar.Border.Background = Border;
    style->HScrollBar.Bar.HigherBackground  = Foreground;
    style->HScrollBar.Bar.LowerBackground   = Foreground;
    style->HScrollBar.Bar.Border.Background = Border;
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

void color_theme::apply(std::shared_ptr<thumb_style> const& style) const
{
    style->Thumb.Background        = Background;
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
    style->NavArrow.UpBackground      = Background;
    style->NavArrow.DownBackground    = Background;
    style->NavArrow.Border.Background = Border;
}

void load_themes(std::map<std::string, color_themes>& themeMap)
{
    themeMap.clear();
    using namespace tcob::data::config;

    object themes {};
    if (themes.load("themes.json") != load_status::Ok) { return; }

    object     customThemes {};
    auto const files {io::enumerate("/", {"themes.*.json", false}, true)};
    for (auto const& file : files) {
        if (customThemes.load(file) == load_status::Ok) { themes.merge(customThemes, true); }
    }

    auto const getColor {[](object const& theme, color& target, std::string const& group, std::string const& name) {
        if (!theme.has(group, name)) { return; }
        target = theme[group][name].is<std::string>()
            ? color::FromString(theme[group][name].as<std::string>())
            : theme[group][name].as<color>();
    }};

    for (auto const& [k, v] : themes) {
        if (!v.is<object>()) { continue; }

        color_theme normal;
        normal.Background = colors::LightSkyBlue;
        normal.Border     = colors::DodgerBlue;
        normal.Shadow     = color {0, 0, 0, 128};
        normal.Foreground = colors::MidnightBlue;
        normal.List       = colors::LightGray;
        normal.Container  = colors::LavenderBlush;
        normal.Label      = colors::LightSlateGray;

        color_theme hover;
        hover.Background = normal.Background;
        hover.Border     = colors::IndianRed;
        hover.Shadow     = color {0, 0, 0, 128};
        hover.Foreground = hover.Border;
        hover.List       = colors::LightBlue;
        hover.Container  = normal.Container;

        color_theme active;
        active.Background = hover.Border;
        active.Border     = hover.Background;
        active.Shadow     = color {0, 0, 0, 128};
        active.Foreground = colors::White;
        active.List       = hover.Border;
        active.Container  = normal.Container;

        object theme {v.as<object>()};

        getColor(theme, normal.Background, "normal", "background");
        getColor(theme, normal.Border, "normal", "border");
        getColor(theme, normal.Shadow, "normal", "shadow");
        getColor(theme, normal.Foreground, "normal", "foreground");
        getColor(theme, normal.List, "normal", "list");
        getColor(theme, normal.Container, "normal", "container");
        getColor(theme, normal.Label, "normal", "label");

        getColor(theme, hover.Background, "hover", "background");
        getColor(theme, hover.Border, "hover", "border");
        getColor(theme, hover.Shadow, "hover", "shadow");
        getColor(theme, hover.Foreground, "hover", "foreground");
        getColor(theme, hover.List, "hover", "list");
        getColor(theme, hover.Container, "hover", "container");

        getColor(theme, active.Background, "active", "background");
        getColor(theme, active.Border, "active", "border");
        getColor(theme, active.Shadow, "active", "shadow");
        getColor(theme, active.Foreground, "active", "foreground");
        getColor(theme, active.List, "active", "list");
        getColor(theme, active.Container, "active", "container");

        color_themes ct;
        ct.TableBackgroundA = colors::OliveDrab;
        ct.TableBackgroundB = colors::DarkGreen;

        getColor(theme, ct.TableBackgroundA, "table", "A");
        getColor(theme, ct.TableBackgroundB, "table", "B");

        ct.Normal   = normal;
        ct.Hover    = hover;
        ct.Active   = active;
        themeMap[k] = ct;
    }
}
}
