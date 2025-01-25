// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../_common/Common.hpp" // IWYU pragma: keep

namespace BrickOut {

class main_menu : public form {
public:
    main_menu(gfx::window* window, assets::group const& resGrp, rect_f bounds);

    std::shared_ptr<button> BtnStart;
    std::shared_ptr<button> BtnQuit;
    std::shared_ptr<label>  LblScore;

protected:
private:
};

}
