// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

class main_menu : public ui::form<ui::dock_layout> {
public:
    main_menu(assets::group const& resGrp, rect_i const& bounds);

    std::shared_ptr<ui::terminal> Terminal;
};

}
