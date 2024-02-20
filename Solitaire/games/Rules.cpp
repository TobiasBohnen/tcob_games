// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Rules.hpp"

#include <utility>

#include "Cards.hpp"
#include "Games.hpp"
#include "Piles.hpp"

namespace solitaire {

auto static is_same_color(suit a, suit b) -> bool
{
    return get_suit_color(a) == get_suit_color(b);
}

////////////////////////////////////////////////////////////

empty::empty(empty::func func, bool canDropStack)
    : Accept {std::move(func)}
    , OnlySingleCard {canDropStack}
{
}

auto empty::Ace() -> func
{
    return [](card const& card) { return card.get_rank() == rank::Ace; };
}

auto empty::King() -> func
{
    return [](card const& card) { return card.get_rank() == rank::King; };
}

auto empty::None() -> func
{
    return [](card const&) { return false; };
}

auto empty::Any() -> func
{
    return [](card const&) { return true; };
}

auto empty::First(pile const& pile, i32 interval) -> func
{
    return [&pile, interval](card const& card) {
        std::optional<rank> startingRank {};
        if (!pile.empty()) {
            startingRank = pile.Cards[0].get_rank();
        }

        return startingRank && card.get_rank() == get_next_rank(*startingRank, interval, true);
    };
}

auto empty::Card(suit s, rank r) -> func
{
    return [=](card const& card) { return card.get_rank() == r && card.get_suit() == s; };
}

auto empty::Card(suit_color sc, rank r) -> func
{
    return [=](card const& card) { return card.get_rank() == r
                                       && get_suit_color(card.get_suit()) == sc; };
}

auto empty::Suits(std::set<suit> const& suits) -> func
{
    return [=](card const& card) { return suits.contains(card.get_suit()); };
}

auto empty::Ranks(std::set<rank> const& ranks) -> func
{
    return [=](card const& card) { return ranks.contains(card.get_rank()); };
}

////////////////////////////////////////////////////////////

namespace rules {

    auto build(pile const& targetPile, isize targetIndex, card const& drop, isize numCards) -> bool
    {
        if ((targetPile.Rule.Move == move_type::None || targetPile.Rule.Move == move_type::Top) && numCards > 1) { return false; }

        if (!limit_size(targetPile, numCards)) { return false; }

        if (fill(targetPile, drop, numCards)) { return true; }
        if (empty(targetPile) || targetIndex < 0) { return false; } // can't fill and is empty

        card const& target {targetPile.Cards[targetIndex]};
        if (target.is_face_down()) { return false; }

        i32 const  interval {targetPile.Rule.Interval};
        bool const wrap {targetPile.Rule.Wrap};

        switch (targetPile.Rule.Build) {
        case build_type::Any:
            return true;
        case build_type::NoBuilding:
            return false;

        case build_type::InRank:
            return in_rank(target, drop);
        case build_type::InRankOrDownByRank:
            return in_rank(target, drop) || build_down(target, drop, interval, wrap);
        case build_type::DownByRank:
            return build_down(target, drop, interval, wrap);
        case build_type::UpByRank:
            return build_up(target, drop, interval, wrap);
        case build_type::UpOrDownByRank:
            return build_up(target, drop, interval, wrap) || build_down(target, drop, interval, wrap);

        case build_type::DownInSuit:
            return in_suit(target, drop) && build_down(target, drop, interval, wrap);
        case build_type::UpInSuit:
            return in_suit(target, drop) && build_up(target, drop, interval, wrap);
        case build_type::UpOrDownInSuit:
            return in_suit(target, drop) && (build_up(target, drop, interval, wrap) || build_down(target, drop, interval, wrap));

        case build_type::DownInColor:
            return in_color(target, drop) && build_down(target, drop, interval, wrap);
        case build_type::UpInColor:
            return in_color(target, drop) && build_up(target, drop, interval, wrap);
        case build_type::UpOrDownInColor:
            return in_color(target, drop) && (build_up(target, drop, interval, wrap) || build_down(target, drop, interval, wrap));

        case build_type::DownAlternateColors:
            return alternate_color(target, drop) && build_down(target, drop, interval, wrap);
        case build_type::UpAlternateColors:
            return alternate_color(target, drop) && build_up(target, drop, interval, wrap);
        case build_type::UpOrDownAlternateColors:
            return alternate_color(target, drop) && (build_up(target, drop, interval, wrap) || build_down(target, drop, interval, wrap));

        case build_type::DownAnyButOwnSuit:
            return !in_suit(target, drop) && build_down(target, drop, interval, wrap);
        case build_type::UpAnyButOwnSuit:
            return !in_suit(target, drop) && build_up(target, drop, interval, wrap);
        case build_type::UpOrDownAnyButOwnSuit:
            return !in_suit(target, drop) && (build_up(target, drop, interval, wrap) || build_down(target, drop, interval, wrap));

        case build_type::RankPack: {
            if (std::ssize(targetPile.Cards) % 4 == 0) {
                return build_up(target, drop, interval, wrap);
            }
            return in_rank(target, drop);
        }
        }

        return false;
    }

    auto rank(card const& card0, enum rank r) -> bool
    {
        return card0.get_rank() == r;
    }

    auto rank_higher(card const& card0, card const& card1) -> bool
    {
        return static_cast<u8>(card0.get_rank()) > static_cast<u8>(card1.get_rank());
    }

    auto build_up(card const& card0, card const& card1, i32 interval, bool wrap) -> bool
    {
        auto const rank {get_next_rank(card0.get_rank(), interval, wrap)};
        return rank == card1.get_rank();
    }

    auto build_down(card const& card0, card const& card1, i32 interval, bool wrap) -> bool
    {
        auto const rank {get_next_rank(card0.get_rank(), -interval, wrap)};
        return rank == card1.get_rank();
    }

    auto in_suit(card const& card0, card const& card1) -> bool
    {
        return card0.get_suit() == card1.get_suit();
    }

    auto suit(card const& card0, enum suit s) -> bool
    {
        return card0.get_suit() == s;
    }

    auto in_color(card const& card0, card const& card1) -> bool
    {
        return is_same_color(card0.get_suit(), card1.get_suit());
    }

    auto alternate_color(card const& card0, card const& card1) -> bool
    {
        return !is_same_color(card0.get_suit(), card1.get_suit());
    }

    auto in_rank(card const& card0, card const& card1) -> bool
    {
        return card0.get_rank() == card1.get_rank();
    }

    auto fill(pile const& pile, card const& card0, isize numCards) -> bool
    {
        if (!empty(pile)) { return false; }
        if (pile.Rule.Empty.OnlySingleCard && numCards > 1) { return false; }
        return pile.Rule.Empty.Accept(card0);
    }

    auto empty(pile const& pile) -> bool
    {
        return pile.empty();
    }

    auto limit_size(pile const& pile, isize numCards) -> bool
    {
        if (pile.Rule.Limit < 0) { return true; }
        return std::ssize(pile.Cards) + numCards <= pile.Rule.Limit;
    }

    namespace stack {
        auto top(pile const& target, point_i pos) -> isize
        {
            if (!target.empty()) {
                if (target.Cards.back().Bounds.contains(pos)) {
                    return std::ssize(target.Cards) - 1;
                }
            }

            return INDEX_INVALID;
        }

        auto top_or_pile(pile const& target, point_i pos) -> isize
        {
            if (!target.empty()) {
                isize const cardCount {std::ssize(target.Cards)};

                // hover top
                isize const top {cardCount - 1};
                if (target.Cards[top].Bounds.contains(pos)) { return top; }
                if (cardCount == 1) { return INDEX_INVALID; }

                //  or whole pile
                if (target.Cards[0].Bounds.contains(pos)) { return 0; }
            }

            return INDEX_INVALID;
        }

        auto face_up(pile const& target, point_i pos) -> isize
        {
            if (!target.empty()) {
                isize const cardCount {std::ssize(target.Cards)};

                isize const top {cardCount - 1};
                for (isize i {top}; i >= 0; --i) {
                    if (i < top && target.Cards[i].is_face_down()) {
                        break;
                    }

                    if (target.Cards[i].Bounds.contains(pos)) {
                        return i;
                    }
                }
            }

            return INDEX_INVALID;
        }

        auto in_seq(games::base_game const* game, pile const& target, point_i pos) -> isize
        {
            if (!target.empty()) {
                isize const cardCount {std::ssize(target.Cards)};

                isize const top {cardCount - 1};
                for (isize i {top}; i >= 0; --i) {
                    auto const& card0 {target.Cards[i]};

                    if (i < top
                        && (card0.is_face_down() || !game->can_drop(target, i, target.Cards[i + 1], 1))) {
                        break;
                    }

                    if (card0.Bounds.contains(pos)) {
                        return i;
                    }
                }
            }

            return INDEX_INVALID;
        }

        auto in_seq_in_suit(games::base_game const* game, pile const& target, point_i pos) -> isize
        {
            if (!target.empty()) {
                isize const cardCount {std::ssize(target.Cards)};

                isize const top {cardCount - 1};
                for (isize i {top}; i >= 0; --i) {
                    auto const& card0 {target.Cards[i]};

                    if (i < top) {
                        auto const& card1 {target.Cards[i + 1]};
                        if (card0.get_suit() != card1.get_suit()
                            || card0.is_face_down()
                            || !game->can_drop(target, i, card1, 1)) {
                            break;
                        }
                    }

                    if (card0.Bounds.contains(pos)) {
                        return i;
                    }
                }
            }

            return INDEX_INVALID;
        }

        auto super_move(games::base_game const* game, pile const& target, point_i pos) -> isize
        {
            if (!game->piles().contains(pile_type::FreeCell)) {
                return top(target, pos);
            }

            if (!target.empty()) {
                isize const cardCount {std::ssize(target.Cards)};
                auto const& freeCells {game->piles().at(pile_type::FreeCell)};

                isize movableCards {std::ranges::count_if(freeCells, [](auto const* fc) { return fc->empty(); }) + 1};
                if (movableCards == 1 && target.Cards.back().Bounds.contains(pos)) {
                    return cardCount - 1;
                }

                isize idx {0};
                for (isize i {cardCount - 2}; i >= 0; --i) {
                    --movableCards;
                    if (movableCards == 0 || !game->can_drop(target, i, target.Cards[i + 1], 1)) {
                        idx = i + 1;
                        break;
                    }
                }

                for (isize i {cardCount - 1}; i >= idx; --i) {
                    if (target.Cards[i].Bounds.contains(pos)) {
                        return i;
                    }
                }
            }

            return INDEX_INVALID;
        }
    }
}

}
