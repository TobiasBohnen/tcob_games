// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../_common/Common.hpp" // IWYU pragma: keep

namespace Mines {

class main_menu : public form<dock_layout> {
public:
    main_menu(gfx::window* window, assets::group const& resGrp, rect_f bounds);

    std::shared_ptr<button> BtnStart;
    std::shared_ptr<button> BtnQuit;
    std::shared_ptr<slider> SldWidth;
    std::shared_ptr<slider> SldHeight;
    std::shared_ptr<slider> SldMines;

protected:
private:
};

}
