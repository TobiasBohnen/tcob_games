// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep
#include "Games.hpp"

namespace solitaire::games {
////////////////////////////////////////////////////////////

class aces_up : public base_game {
public:
    aces_up(field& f);

    static game_info Info;

    auto can_drop(pile const& targetPile, isize targetIndex, card const& drop, isize numCards) const -> bool override;

protected:
    auto do_deal() -> bool override;
    auto check_state() const -> game_state override;

private:
};
}
