// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

class renderer {
public:
    explicit renderer(ui::terminal& term);

    void begin();
    auto current_cell() const -> point_i;

    void set_color(color foreground, color background);

    void draw_box(rect_i const& rect);

    void draw_cell(point_i pos, utf8_string_view str);
    void draw_cell(utf8_string_view str);

private:
    ui::terminal& _term;
};

}