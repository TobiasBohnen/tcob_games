// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep
#include "Games.hpp"

namespace solitaire {
////////////////////////////////////////////////////////////

class main_menu : public form {
public:
    main_menu(gfx::window* window, rect_f bounds);

    std::shared_ptr<button>   BtnStart;
    std::shared_ptr<list_box> LbxGames;
    std::shared_ptr<button>   BtnGames;
    std::shared_ptr<label>    LblInfo;
    std::shared_ptr<button>   BtnUndo;
    std::shared_ptr<button>   BtnQuit;
};

////////////////////////////////////////////////////////////

class game_list : public form {
public:
    game_list(gfx::window* window, rect_f bounds, std::vector<games::game_info> const& games);

    prop<std::string> SelectedGame;
};

}
