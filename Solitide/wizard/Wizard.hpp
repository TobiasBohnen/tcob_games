// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

namespace solitaire {
////////////////////////////////////////////////////////////

class form_wizard : public form {
public:
    form_wizard(gfx::window* window, assets::group& resGrp);

    std::shared_ptr<button> BtnGenerate;
    std::shared_ptr<button> BtnBack;

    void set_log_messages(std::vector<std::string> const& messages);

private:
    std::shared_ptr<list_box> _lbxLog;
};

}
