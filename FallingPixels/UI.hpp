// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "ElementSystem.hpp"

////////////////////////////////////////////////////////////

class elements_form : public ui::form<ui::dock_layout> {
public:
    elements_form(rect_i const& bounds, std::vector<element_def> const& elements);

    signal<i32> SelectedElement;

private:
    void gen_styles();

    asset_owner_ptr<gfx::font_family> _font;
};