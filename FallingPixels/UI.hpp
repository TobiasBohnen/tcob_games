// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Element.hpp"
#include <tcob/tcob.hpp>

using namespace tcob;

////////////////////////////////////////////////////////////

class elements_form : public gfx::ui::form {
public:
    elements_form(gfx::window* window, rect_f const& bounds);

    signal<element_type> LeftButtonElement;
    signal<element_type> MiddleButtonElement;
    signal<element_type> RightButtonElement;

private:
    void gen_styles();

    assets::manual_asset_ptr<gfx::font_family> _font;
};