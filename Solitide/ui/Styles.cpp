#include "Styles.hpp"

static auto parse_unit_value(tcob::utf8_string_view input) -> std::pair<std::string, std::string>
{
    std::string value;
    std::string unit;
    bool        parsingValue {true};
    bool        decimalFound {false};

    for (char c : input) {
        if (std::isspace(c)) { continue; }

        if (std::isdigit(c) || (c == '.' && !decimalFound)) {
            if (parsingValue) {
                value += c;
            } else {
                unit += c;
            }
            if (c == '.') { decimalFound = true; }
        } else {
            parsingValue = false;
            unit += c;
        }
    }

    return std::make_pair(value, unit);
}

auto static get_length_values(tcob::data::config::cfg_value const& config) -> std::vector<tcob::gfx::ui::length>
{
    using namespace tcob;
    std::vector<gfx::ui::length> l;

    auto const strings {helper::split(std::get<utf8_string>(config), ',')};
    for (auto str : strings) {
        auto const [valueStr, unit] {parse_unit_value(str)};
        f32 const valueF {std::stof(valueStr)};
        if (unit == "px") {
            l.emplace_back(valueF, gfx::ui::length::type::Absolute);
        } else if (unit == "pct") {
            l.emplace_back(valueF / 100.0f, gfx::ui::length::type::Relative);
        } else {
            return {};
        }
    }

    return l;
}

namespace tcob::data::config {
template <>
struct converter<gfx::ui::thickness> {
    auto static IsType(cfg_value const& config) -> bool
    {
        return std::holds_alternative<utf8_string>(config);
    }

    auto static From(cfg_value const& config, gfx::ui::thickness& value) -> bool
    {
        if (std::holds_alternative<utf8_string>(config)) {
            auto l {get_length_values(config)};
            switch (l.size()) {
            case 1:
                value = {l[0]};
                return true;
            case 2:
                value = {l[0], l[1]};
                return true;
            case 4:
                value = {l[0], l[1], l[2], l[3]};
                return true;
            default:
                return false;
            }
        }

        return false;
    }
};

template <>
struct converter<gfx::ui::dimensions> {
    auto static IsType(cfg_value const& config) -> bool
    {
        return std::holds_alternative<utf8_string>(config);
    }

    auto static From(cfg_value const& config, gfx::ui::dimensions& value) -> bool
    {
        if (std::holds_alternative<utf8_string>(config)) {
            auto l {get_length_values(config)};
            switch (l.size()) {
            case 1:
                value = {l[0], l[0]};
                return true;
            case 2:
                value = {l[0], l[1]};
                return true;
            default:
                return false;
            }
        }

        return false;
    }
};

template <>
struct converter<gfx::ui::length> {
    auto static IsType(cfg_value const& config) -> bool
    {
        return std::holds_alternative<utf8_string>(config);
    }

    auto static From(cfg_value const& config, gfx::ui::length& value) -> bool
    {
        if (std::holds_alternative<utf8_string>(config)) {
            auto l {get_length_values(config)};
            switch (l.size()) {
            case 1:
                value = l[0];
                return true;
            default:
                return false;
            }
        }

        return false;
    }
};

template <>
struct converter<gfx::ui::ui_paint> {
    auto static IsType(cfg_value const& config) -> bool
    {
        return std::holds_alternative<utf8_string>(config);
    }

    auto static From(cfg_value const& config, gfx::ui::ui_paint& value) -> bool
    {
        if (std::holds_alternative<utf8_string>(config)) {
            auto const str {std::get<utf8_string>(config)};
            value = color::FromString(str);
            return true;
        }

        return false;
    }
};

}

using namespace tcob::data::config;

namespace solitaire {

styles::styles(assets::group& resGrp)
    : _resGrp {resGrp}
{
}

auto styles::load(color_themes const& theme) -> style_collection
{
    using namespace tcob::literals;
    using namespace std::chrono_literals;

    namespace element = element;

    style_collection retValue;

    object styleFile {};
    if (styleFile.load("styles.json") != load_status::Ok) { return retValue; }
    std::unordered_map<std::string, std::string> typeMap;

    for (auto const& entry : styleFile) {
        object styleObj;
        if (!entry.second.try_get(styleObj)) { continue; } // ERROR
        if (entry.first.empty()) { continue; }             // ERROR

        auto typeSplit {helper::split(entry.first, '#')};

        auto        names {helper::split(typeSplit[0], ':')};
        style_flags flags {};
        if (names.size() > 1) {
            if (std::find(names.begin() + 1, names.end(), "hover") != names.end()) { flags.Hover = true; }
            if (std::find(names.begin() + 1, names.end(), "active") != names.end()) { flags.Active = true; }
        }

        auto const   name {std::string {names[0]}};
        std::string& type {typeMap[name]};
        if (type.empty()) {
            type = typeSplit.size() > 1 ? typeSplit[1] : typeSplit[0];
        }

        auto applyTheme([&](auto&& style) {
            if (flags.Active) {
                theme.Active.apply(style);
            } else if (flags.Hover) {
                theme.Hover.apply(style);
            } else {
                theme.Normal.apply(style);
            }
        });

        std::unordered_map<std::string, std::function<void()>> const styleCreators {
            {"accordion", [&]() { applyTheme(create<accordion::style>(styleObj, retValue, name, flags)); }},
            {"button", [&]() { applyTheme(create<button::style>(styleObj, retValue, name, flags)); }},
            {"checkbox", [&]() { applyTheme(create<checkbox::style>(styleObj, retValue, name, flags)); }},
            {"cycle_button", [&]() { applyTheme(create<cycle_button::style>(styleObj, retValue, name, flags)); }},
            {"drop_down_list", [&]() { applyTheme(create<drop_down_list::style>(styleObj, retValue, name, flags)); }},
            {"grid_view", [&]() { applyTheme(create<grid_view::style>(styleObj, retValue, name, flags)); }},
            {"image_box", [&]() { applyTheme(create<image_box::style>(styleObj, retValue, name, flags)); }},
            {"label", [&]() { applyTheme(create<label::style>(styleObj, retValue, name, flags)); }},
            {"list_box", [&]() { applyTheme(create<list_box::style>(styleObj, retValue, name, flags)); }},
            {"panel", [&]() { applyTheme(create<panel::style>(styleObj, retValue, name, flags)); }},
            {"progress_bar", [&]() { applyTheme(create<progress_bar::style>(styleObj, retValue, name, flags)); }},
            {"radio_button", [&]() { applyTheme(create<radio_button::style>(styleObj, retValue, name, flags)); }},
            {"slider", [&]() { applyTheme(create<slider::style>(styleObj, retValue, name, flags)); }},
            {"spinner", [&]() { applyTheme(create<spinner::style>(styleObj, retValue, name, flags)); }},
            {"tab_container", [&]() { applyTheme(create<tab_container::style>(styleObj, retValue, name, flags)); }},
            {"text_box", [&]() { applyTheme(create<text_box::style>(styleObj, retValue, name, flags)); }},
            {"toggle", [&]() { applyTheme(create<toggle::style>(styleObj, retValue, name, flags)); }},
            {"tooltip", [&]() { applyTheme(create<tooltip::style>(styleObj, retValue, name, flags)); }},
            {"thumb", [&]() { applyTheme(create<thumb_style>(styleObj, retValue, name, flags)); }},
            {"nav_arrow", [&]() { applyTheme(create<nav_arrows_style>(styleObj, retValue, name, flags)); }},
            {"item", [&]() { applyTheme(create<item_style>(styleObj, retValue, name, flags)); }}};
        auto it {styleCreators.find(type)};
        if (it != styleCreators.end()) {
            it->second();
        } else {
            // ERROR
        }
    }

    return retValue;
}

void styles::parse(object const& obj, accordion::style* style)
{
    parse_widget_style(obj, style);
    obj.try_get(style->SectionBarHeight, "section_bar_height");
    obj.try_get(style->SectionItemClass, "section_item_class");
}

void styles::parse(object const& obj, button::style* style)
{
    parse_widget_style(obj, style);
    if (object el; obj.try_get(el, "text")) { parse_element(el, &style->Text); }
}

void styles::parse(object const& obj, checkbox::style* style)
{
    parse_widget_style(obj, style);
    if (object el; obj.try_get(el, "tick")) { parse_element(el, &style->Tick); }
}

void styles::parse(object const& obj, cycle_button::style* style)
{
    parse_widget_style(obj, style);
    if (object el; obj.try_get(el, "text")) { parse_element(el, &style->Text); }
}

void styles::parse(object const& obj, drop_down_list::style* style)
{
    parse_widget_style(obj, style);
    if (object el; obj.try_get(el, "text")) { parse_element(el, &style->Text); }
    obj.try_get(style->NavArrowClass, "arrow_class");
    obj.try_get(style->ItemClass, "item_class");
    obj.try_get(style->ItemHeight, "item_height");
    obj.try_get(style->VisibleItemCount, "visible_item_count");
    if (object el; obj.try_get(el, "v_scroll_bar")) { parse_element(el, &style->VScrollBar); }
}

void styles::parse(data::config::object const& obj, grid_view::style* style)
{
    parse_widget_style(obj, style);
    if (object el; obj.try_get(el, "v_scroll_bar")) { parse_element(el, &style->VScrollBar); }
    obj.try_get(style->AutoSizeColumns, "auto_size_columns");
    obj.try_get(style->HeaderClass, "header_class");
    obj.try_get(style->RowClass, "row_class");
    obj.try_get(style->RowHeight, "row_height");
}

void styles::parse(data::config::object const& obj, image_box::style* style)
{
    parse_widget_style(obj, style);
    obj.try_get(style->Fit, "fit");
    obj.try_get(style->Alignment, "alignment");
}

void styles::parse(data::config::object const& obj, label::style* style)
{
    parse_widget_style(obj, style);
    if (object el; obj.try_get(el, "text")) { parse_element(el, &style->Text); }
}

void styles::parse(data::config::object const& obj, list_box::style* style)
{
    parse_widget_style(obj, style);
    if (object el; obj.try_get(el, "v_scroll_bar")) { parse_element(el, &style->VScrollBar); }
    obj.try_get(style->ItemClass, "item_class");
    obj.try_get(style->ItemHeight, "item_height");
}

void styles::parse(data::config::object const& obj, panel::style* style)
{
    parse_widget_style(obj, style);
    if (object el; obj.try_get(el, "v_scroll_bar")) { parse_element(el, &style->VScrollBar); }
    if (object el; obj.try_get(el, "h_scroll_bar")) { parse_element(el, &style->HScrollBar); }
}

void styles::parse(data::config::object const& obj, progress_bar::style* style)
{
    parse_widget_style(obj, style);
    if (object el; obj.try_get(el, "bar")) { parse_element(el, &style->Bar); }
}

void styles::parse(data::config::object const& obj, radio_button::style* style)
{
    parse_widget_style(obj, style);
    if (object el; obj.try_get(el, "tick")) { parse_element(el, &style->Tick); }
}

void styles::parse(data::config::object const& obj, slider::style* style)
{
    parse_style(obj, style);
    if (object el; obj.try_get(el, "bar")) { parse_element(el, &style->Bar); }
    obj.try_get(style->ThumbClass, "thumb_class");
}

void styles::parse(data::config::object const& obj, spinner::style* style)
{
    parse_widget_style(obj, style);
    if (object el; obj.try_get(el, "text")) { parse_element(el, &style->Text); }
    obj.try_get(style->NavArrowClass, "nav_arrow_class");
}

void styles::parse(data::config::object const& obj, tab_container::style* style)
{
    parse_widget_style(obj, style);
    obj.try_get(style->TabItemClass, "tab_item_class");
    obj.try_get(style->TabBarPosition, "tab_bar_position");
    obj.try_get(style->TabBarHeight, "tab_bar_height");
    obj.try_get(style->MaxTabsPerRow, "max_tabs_per_row");
}

void styles::parse(data::config::object const& obj, text_box::style* style)
{
    parse_widget_style(obj, style);
    if (object el; obj.try_get(el, "text")) { parse_element(el, &style->Text); }
    if (object el; obj.try_get(el, "caret")) { parse_element(el, &style->Caret); }
}

void styles::parse(data::config::object const& obj, toggle::style* style)
{
    parse_widget_style(obj, style);
    if (object el; obj.try_get(el, "tick")) { parse_element(el, &style->Tick); }
    obj.try_get(style->Delay, "delay");
}

void styles::parse(data::config::object const& obj, tooltip::style* style)
{
    parse_widget_style(obj, style);
    if (object el; obj.try_get(el, "v_scroll_bar")) { parse_element(el, &style->VScrollBar); }
    if (object el; obj.try_get(el, "h_scroll_bar")) { parse_element(el, &style->HScrollBar); }
    obj.try_get(style->Delay, "delay");
    obj.try_get(style->FadeIn, "fade_in");
}

void styles::parse(data::config::object const& obj, nav_arrows_style* style)
{
    obj.try_get(style->NavArrow.Type, "type");
    obj.try_get(style->NavArrow.IncBackground, "inc_background");
    obj.try_get(style->NavArrow.DecBackground, "dec_background");
    obj.try_get(style->NavArrow.Foreground, "foreground");
    obj.try_get(style->NavArrow.Size, "size");
    obj.try_get(style->NavArrow.Padding, "padding");
    if (object el; obj.try_get(el, "border")) { parse_element(el, &style->NavArrow.Border); }
}

void styles::parse(data::config::object const& obj, thumb_style* style)
{
    obj.try_get(style->Thumb.Type, "type");
    obj.try_get(style->Thumb.Background, "background");
    obj.try_get(style->Thumb.LongSide, "long_side");
    obj.try_get(style->Thumb.ShortSide, "short_side");
    if (object el; obj.try_get(el, "border")) { parse_element(el, &style->Thumb.Border); }
}

void styles::parse(data::config::object const& obj, item_style* style)
{
    if (object el; obj.try_get(el, "text")) { parse_element(el, &style->Item.Text); }
    obj.try_get(style->Item.Background, "background");
    if (object el; obj.try_get(el, "border")) { parse_element(el, &style->Item.Border); }
    obj.try_get(style->Item.Padding, "padding");
}

void styles::parse_element(object const& obj, element::border* border)
{
    obj.try_get(border->Radius, "radius");
    obj.try_get(border->Background, "background");
    obj.try_get(border->Size, "size");
    obj.try_get(border->Type, "type");
}

void styles::parse_element(object const& obj, element::shadow* shadow)
{
    obj.try_get(shadow->Color, "color");
    obj.try_get(shadow->OffsetX, "offset_x");
    obj.try_get(shadow->OffsetY, "offset_y");
}

void styles::parse_element(object const& obj, element::text* text)
{
    obj.try_get(text->Color, "color");
    obj.try_get(text->Alignment, "alignment");
    obj.try_get(text->AutoSize, "auto_size");
    obj.try_get(text->Size, "size");
    if (std::string fontName; obj.try_get(fontName, "font")) { text->Font = _resGrp.get<gfx::font_family>(fontName); }
    // obj.try_get(text->Decoration, "decoration");
    obj.try_get(text->Style, "style");
    if (object el; obj.try_get(el, "shadow")) { parse_element(el, &text->Shadow); }
    obj.try_get(text->Transform, "transform");
}

void styles::parse_element(object const& obj, element::tick* tick)
{
    obj.try_get(tick->Type, "type");
    obj.try_get(tick->Foreground, "foreground");
    obj.try_get(tick->Size, "size");
}

void styles::parse_element(object const& obj, element::scrollbar* scrollbar)
{
    if (object el; obj.try_get(el, "bar")) { parse_element(el, &scrollbar->Bar); }
    obj.try_get(scrollbar->ThumbClass, "thumb_class");
}

void styles::parse_element(data::config::object const& obj, element::bar* bar)
{
    obj.try_get(bar->Type, "type");
    obj.try_get(bar->LowerBackground, "lower_background");
    obj.try_get(bar->HigherBackground, "higher_background");
    obj.try_get(bar->Size, "size");
    if (object el; obj.try_get(el, "border")) { parse_element(el, &bar->Border); }
    obj.try_get(bar->Delay, "delay");
}

void styles::parse_element(data::config::object const& obj, element::caret* caret)
{
    obj.try_get(caret->Color, "color");
    obj.try_get(caret->Width, "width");
    obj.try_get(caret->BlinkRate, "blink_rate");
}

void styles::parse_style(object const& obj, widget_style* style)
{
    obj.try_get(style->Cursor, "cursor");
    obj.try_get(style->Margin, "margin");
    obj.try_get(style->Padding, "padding");
}

void styles::parse_widget_style(object const& obj, widget_style* style)
{
    parse_style(obj, style);
    obj.try_get(style->Background, "background");
    if (object el; obj.try_get(el, "border")) { parse_element(el, &style->Border); }
    if (object el; obj.try_get(el, "drop_shadow")) { parse_element(el, &style->DropShadow); }
}

} // namespace solitaire
