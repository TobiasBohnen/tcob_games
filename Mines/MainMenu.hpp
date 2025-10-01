// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../_common/Common.hpp" // IWYU pragma: keep

namespace Mines {

class main_menu : public form<dock_layout> {
public:
    main_menu(assets::group const& resGrp, rect_i const& bounds);

    button* BtnStart {nullptr};
    button* BtnQuit {nullptr};
    slider* SldWidth {nullptr};
    slider* SldHeight {nullptr};
    slider* SldMines {nullptr};

protected:
private:
};

}
