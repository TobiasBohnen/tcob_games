// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep
#include "Games.hpp"

namespace solitaire::games {
////////////////////////////////////////////////////////////

class four_seasons : public base_game {
public:
    four_seasons(field& f);

    static game_info Info;

protected:
    auto do_deal() -> bool override;
};
}
