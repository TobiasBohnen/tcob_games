// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../_common/Common.hpp" // IWYU pragma: keep

namespace Snake {

class main_menu : public form<dock_layout> {
public:
    main_menu(assets::group const& resGrp, rect_i const& bounds);

    signal<>             Start;
    signal<>             Quit;
    signal<string const> Score;

protected:
private:
    void gen_styles();

    asset_owner_ptr<gfx::font_family> _font;
};

}