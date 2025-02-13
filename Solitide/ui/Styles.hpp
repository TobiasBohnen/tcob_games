// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "Themes.hpp"

namespace solitaire {
////////////////////////////////////////////////////////////

class styles {
public:
    styles(assets::group& resGrp);

    auto load(color_themes const& theme) -> style_collection;

private:
    template <typename T>
    auto create(data::config::object const& obj, style_collection& styles, std::string const& name, style_flags flags);

    void parse(data::config::object const& obj, accordion::style* style);
    void parse(data::config::object const& obj, button::style* style);
    void parse(data::config::object const& obj, checkbox::style* style);
    void parse(data::config::object const& obj, cycle_button::style* style);
    void parse(data::config::object const& obj, drop_down_list::style* style);
    void parse(data::config::object const& obj, grid_view::style* style);
    void parse(data::config::object const& obj, image_box::style* style);
    void parse(data::config::object const& obj, label::style* style);
    void parse(data::config::object const& obj, list_box::style* style);
    void parse(data::config::object const& obj, panel::style* style);
    void parse(data::config::object const& obj, progress_bar::style* style);
    void parse(data::config::object const& obj, radio_button::style* style);
    void parse(data::config::object const& obj, slider::style* style);
    void parse(data::config::object const& obj, spinner::style* style);
    void parse(data::config::object const& obj, tab_container::style* style);
    void parse(data::config::object const& obj, text_box::style* style);
    void parse(data::config::object const& obj, toggle::style* style);
    void parse(data::config::object const& obj, nav_arrows_style* style);
    void parse(data::config::object const& obj, thumb_style* style);
    void parse(data::config::object const& obj, item_style* style);

    void parse_element(data::config::object const& obj, element::border* border);
    void parse_element(data::config::object const& obj, element::shadow* shadow);
    void parse_element(data::config::object const& obj, element::text* text);
    void parse_element(data::config::object const& obj, element::tick* tick);
    void parse_element(data::config::object const& obj, element::scrollbar* scrollbar);
    void parse_element(data::config::object const& obj, element::bar* bar);
    void parse_element(data::config::object const& obj, element::caret* caret);

    void parse_style(data::config::object const& obj, widget_style* style);
    void parse_widget_style(data::config::object const& obj, widget_style* style);

    assets::group& _resGrp;
};

template <typename T>
inline auto styles::create(data::config::object const& obj, style_collection& styles, std::string const& name, style_flags flags)
{
    auto style {styles.create<T>(name, flags)};
    *style = *dynamic_cast<T*>(styles.get({name, {}, {}}));
    parse(obj, style.get());
    return style;
}

}
