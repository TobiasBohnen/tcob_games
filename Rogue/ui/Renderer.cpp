// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Renderer.hpp"

namespace Rogue {

renderer::renderer(ui::terminal& term)
    : _term {term}
{
}

void renderer::begin()
{
    _term.clear();
}

auto renderer::current_cell() const -> point_i
{
    return _term.get_xy();
}

void renderer::set_color(color foreground, color background)
{
    _term.color_set(foreground, background);
}

void renderer::draw_box(rect_i const& rect)
{
    _term.rectangle(rect);
}

void renderer::draw_cell(point_i pos, utf8_string_view str)
{
    _term.add_str(pos, str);
}

void renderer::draw_cell(utf8_string_view str)
{
    _term.add_str(str);
}

}
