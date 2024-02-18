// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep
#include "Games.hpp"

namespace solitaire::games {
////////////////////////////////////////////////////////////

class spider : public base_game {
public:
    spider(field& f);

    static game_info Info;

protected:
    spider(field& f, game_info info);

    void before_layout() override;
    auto do_deal() -> bool override;
};

////////////////////////////////////////////////////////////

class arachnida : public spider {
public:
    arachnida(field& f);

    static game_info Info;

protected:
    auto stack_index(pile const& targetPile, point_i pos) const -> isize override;
};
}
