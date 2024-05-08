// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include <map>

namespace solitaire {

struct color_theme {
    color Background {colors::Transparent};
    color Foreground {colors::Transparent};
    color Container {colors::Transparent};
    color Shadow {colors::Transparent};
    color Border {colors::Transparent};
    color List {colors::Transparent};
    color Label {colors::Transparent};

    void apply(std::shared_ptr<button::style> const& style) const;
    void apply(std::shared_ptr<checkbox::style> const& style) const;
    void apply(std::shared_ptr<cycle_button::style> const& style) const;
    void apply(std::shared_ptr<drop_down_list::style> const& style) const;
    void apply(std::shared_ptr<grid_view::style> const& style) const;
    void apply(std::shared_ptr<image_box::style> const& style) const;
    void apply(std::shared_ptr<label::style> const& style) const;
    void apply(std::shared_ptr<list_box::style> const& style) const;
    void apply(std::shared_ptr<progress_bar::style> const& style) const;
    void apply(std::shared_ptr<radio_button::style> const& style) const;
    void apply(std::shared_ptr<slider::style> const& style) const;
    void apply(std::shared_ptr<spinner::style> const& style) const;
    void apply(std::shared_ptr<text_box::style> const& style) const;
    void apply(std::shared_ptr<toggle::style> const& style) const;

    void apply(std::shared_ptr<panel::style> const& style) const;
    void apply(std::shared_ptr<tab_container::style> const& style) const;
    void apply(std::shared_ptr<accordion::style> const& style) const;
    void apply(std::shared_ptr<tooltip::style> const& style) const;

    void apply(std::shared_ptr<thumb_style> const& style) const;
    void apply(std::shared_ptr<item_style> const& style) const;
    void apply(std::shared_ptr<nav_arrows_style> const& style) const;
};

struct color_themes {
    color TableBackgroundA {colors::Transparent};
    color TableBackgroundB {colors::Transparent};

    color_theme Normal;
    color_theme Hover;
    color_theme Active;
};

auto load_themes() -> std::map<std::string, color_themes>;

}
