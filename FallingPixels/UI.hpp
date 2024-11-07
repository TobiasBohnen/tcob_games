// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

////////////////////////////////////////////////////////////

class elements_form : public gfx::ui::form {
public:
    elements_form(gfx::window* window, rect_f const& bounds, script_element_vec const& elements);

    signal<i32> LeftButtonElement;
    signal<i32> MiddleButtonElement;
    signal<i32> RightButtonElement;

private:
    void gen_styles();

    assets::manual_asset_ptr<gfx::font_family> _font;
};