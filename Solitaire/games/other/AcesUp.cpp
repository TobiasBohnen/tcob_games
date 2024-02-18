// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "AcesUp.hpp"

#include "Field.hpp"

#include <algorithm>

namespace solitaire::games {

////////////////////////////////////////////////////////////

game_info aces_up::Info = {.Name = "Aces Up", .Type = type::ClosedNonBuilder, .DeckCount = 1, .CardDealCount = 4, .Redeals = 0};

aces_up::aces_up(field& f)
    : base_game {f, Info}
{
    Stock.Position = {0, 0};
    Stock.Initial  = initial::face_down(48);
    add_pile(&Stock);

    create_piles(Foundation, 1, [](auto& foundation, i32) {
        foundation.Position  = {1, 0};
        foundation.Rule.Move = {move_type::None};
    });

    create_piles(Tableau, 4, [](auto& tableau, i32 i) {
        tableau.Position = {i + 0.0f, 1};
        tableau.Initial  = initial::face_up(1);
        tableau.Layout   = layout_type::Column;
        tableau.Rule     = {.Build = build_type::NoBuilding, .Move = move_type::Top, .Empty {empty::Any()}};
    });
}

auto aces_up::can_drop(pile const& targetPile, isize targetIndex, card const& drop, isize numCards) const -> bool
{
    if (numCards != 1) { return false; }

    switch (targetPile.Type) {
    case pile_type::Foundation:
        if (rules::rank(drop, rank::Ace)) { return false; }
        return std::ranges::any_of(Tableau, [&](auto const& pile) {
            if (pile.empty()) { return false; }
            auto const& card {pile.Cards.back()};
            return (rules::in_suit(card, drop)
                    && (rules::rank(card, rank::Ace) || rules::rank_higher(card, drop)));
        });
    case pile_type::Tableau:
        return rules::build(targetPile, targetIndex, drop, numCards);
    default:
        return false;
    }
}

auto aces_up::check_state() const -> game_state
{
    if (Foundation[0].Cards.size() == 48) {
        return game_state::Success;
    }

    if (Stock.empty()) {
        std::unordered_set<suit> suits;
        for (auto const& tab : Tableau) {
            if (!tab.empty()) {
                suits.insert(tab.Cards.back().get_suit());
            }
        }
        if (suits.size() == 4) {
            return game_state::Failure;
        }
    }

    return game_state::Running;
}

auto aces_up::do_deal() -> bool
{
    return Stock.deal_group(Tableau, false);
}
}
