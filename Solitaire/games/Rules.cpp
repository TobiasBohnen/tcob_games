// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Rules.hpp"

#include "Cards.hpp"
#include "Games.hpp"
#include "Piles.hpp"

namespace solitaire {

auto static is_same_color(suit a, suit b) -> bool
{
    return get_suit_color(a) == get_suit_color(b);
}

////////////////////////////////////////////////////////////

namespace rules {
    static auto build_up(card const& card0, card const& card1, i32 interval, bool wrap) -> bool
    {
        auto const rank {get_next_rank(card0.get_rank(), interval, wrap)};
        return rank == card1.get_rank();
    }

    static auto build_down(card const& card0, card const& card1, i32 interval, bool wrap) -> bool
    {
        auto const rank {get_next_rank(card0.get_rank(), -interval, wrap)};
        return rank == card1.get_rank();
    }

    static auto in_suit(card const& card0, card const& card1) -> bool
    {
        return card0.get_suit() == card1.get_suit();
    }

    static auto in_color(card const& card0, card const& card1) -> bool
    {
        return is_same_color(card0.get_suit(), card1.get_suit());
    }

    static auto alternate_color(card const& card0, card const& card1) -> bool
    {
        return !is_same_color(card0.get_suit(), card1.get_suit());
    }

    static auto in_rank(card const& card0, card const& card1) -> bool
    {
        return card0.get_rank() == card1.get_rank();
    }

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
}

namespace stack {
    auto top(pile const& target, isize idx) -> bool
    {
        return idx == std::ssize(target.Cards) - 1;
    }

    auto top_or_pile(pile const& target, isize idx) -> bool
    {
        return idx == std::ssize(target.Cards) - 1 || idx == 0;
    }

    auto face_up(pile const& target, isize idx) -> bool
    {
        return !target.Cards[idx].is_face_down();
    }

    auto in_seq(games::base_game const* game, pile const& target, isize idx) -> bool
    {
        if (target.Cards[idx].is_face_down()) { return false; }

        for (isize i {idx}; i < std::ssize(target.Cards) - 1; ++i) {
            if (!game->can_drop(target, i, target.Cards[i + 1], 1)) {
                return false;
            }
        }

        return true;
    }

    auto in_seq_in_suit(games::base_game const* game, pile const& target, isize idx) -> bool
    {
        if (target.Cards[idx].is_face_down()) { return false; }

        auto const targetSuit {target.Cards[idx].get_suit()};

        for (isize i {idx}; i < std::ssize(target.Cards) - 1; ++i) {
            if (target.Cards[i + 1].get_suit() != targetSuit
                || !game->can_drop(target, i, target.Cards[i + 1], 1)) {
                return false;
            }
        }

        return true;
    }

    auto in_seq_in_suit_same_rank(games::base_game const* game, pile const& target, isize idx) -> bool
    {
        if (target.Cards[idx].is_face_down()) { return false; }

        if (in_seq_in_suit(game, target, idx)) { return true; }

        auto const targetRank {target.Cards.back().get_rank()};
        for (isize i {idx}; i < std::ssize(target.Cards) - 1; ++i) {
            if (target.Cards[i + 1].get_rank() != targetRank) {
                return false;
            }
        }

        return true;
    }

    auto super_move(games::base_game const* game, pile const& target, isize idx) -> bool
    {
        if (target.Cards[idx].is_face_down()) { return false; }

        if (!game->piles().contains(pile_type::FreeCell)) {
            return top(target, idx);
        }

        auto const& freeCells {game->piles().at(pile_type::FreeCell)};
        isize       movableCards {std::ranges::count_if(freeCells, [](auto const* fc) { return fc->empty(); }) + 1};

        if (idx + movableCards < std::ssize(target.Cards)) { return false; }

        return in_seq(game, target, idx);
    }
}

}
