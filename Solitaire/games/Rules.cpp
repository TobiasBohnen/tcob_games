// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Rules.hpp"

#include "Cards.hpp"
#include "Games.hpp"
#include "Piles.hpp"

namespace solitaire {

////////////////////////////////////////////////////////////

namespace rules {
    static auto empty(pile const& pile) -> bool
    {
        return pile.empty();
    }

    static auto fill(pile const& pile, card const& card0, isize numCards) -> bool
    {
        if (!empty(pile)) { return false; }
        return pile.Rule.Empty(card0, numCards);
    }

    static auto limit_size(pile const& pile, isize numCards) -> bool
    {
        if (pile.Rule.Limit < 0) { return true; }
        return std::ssize(pile.Cards) + numCards <= pile.Rule.Limit;
    }

    auto build(pile const& targetPile, isize targetIndex, card const& drop, isize numCards) -> bool
    {
        if ((!targetPile.Rule.IsSequence) && numCards > 1) { return false; }

        if (!limit_size(targetPile, numCards)) { return false; }

        if (fill(targetPile, drop, numCards)) { return true; }
        if (empty(targetPile) || targetIndex < 0) { return false; } // can't fill and is empty

        card const& target {targetPile.Cards[targetIndex]};
        if (target.is_face_down()) { return false; }

        i32 const  interval {targetPile.Rule.Interval};
        bool const wrap {targetPile.Rule.Wrap};

        return targetPile.Rule.Build(target, drop, interval, wrap);
    }
}

auto move::top(games::base_game const*, pile const* target, isize idx) -> bool
{
    return idx == std::ssize(target->Cards) - 1;
}

}
