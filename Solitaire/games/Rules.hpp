// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Cards.hpp"
#include "Common.hpp" // IWYU pragma: keep

namespace solitaire {

//////////////////
enum class build_type {
    NoBuilding,

    InRank,
    RankPack,

    UpOrDownByRank,
    DownByRank,
    UpByRank,
    InRankOrDownByRank,

    UpOrDownAnyButOwnSuit,
    DownAnyButOwnSuit,
    UpAnyButOwnSuit,

    UpOrDownInSuit,
    DownInSuit,
    UpInSuit,

    UpOrDownInColor,
    DownInColor,
    UpInColor,

    UpOrDownAlternateColors,
    DownAlternateColors,
    UpAlternateColors,

    Any
};

enum class move_type {
    None, // non-playable
    Top,
    TopOrPile,
    FaceUp,
    InSequence,
    InSequenceInSuit,
    SuperMove,
    Other
};

class empty {
public:
    using func = std::function<bool(card const&)>;

    empty() = default;
    empty(empty::func func, bool canDropStack = false);

    empty::func Accept {empty::None()};
    bool        OnlySingleCard {false};

    auto static Ace() -> func;
    auto static King() -> func;
    auto static None() -> func;
    auto static Any() -> func;
    auto static First(pile const& pile, i32 interval = 0) -> func;
    auto static Card(suit s, rank r) -> func;
    auto static Card(suit_color sc, rank r) -> func;
    auto static Suits(std::set<suit> const& suits) -> func;
    auto static Ranks(std::set<rank> const& ranks) -> func;
};

struct rule {
    build_type Build {build_type::NoBuilding};
    i32        Interval {1};
    bool       Wrap {false};
    move_type  Move {move_type::Top};
    empty      Empty {};
    i32        Limit {-1};
};

//////////////////////////////////////////

namespace rules {
    auto build(pile const& targetPile, isize targetIndex, card const& drop, isize numCards) -> bool;

    auto in_suit(card const& card0, card const& card1) -> bool;
    auto suit(card const& card0, suit s) -> bool;

    auto in_color(card const& card0, card const& card1) -> bool;
    auto alternate_color(card const& card0, card const& card1) -> bool;

    auto in_rank(card const& card0, card const& card1) -> bool;
    auto rank(card const& card0, rank r) -> bool;
    auto rank_higher(card const& card0, card const& card1) -> bool;
    auto build_up(card const& card0, card const& card1, i32 interval, bool wrap) -> bool;
    auto build_down(card const& card0, card const& card1, i32 interval, bool wrap) -> bool;

    auto fill(pile const& pile, card const& card0, isize numCards) -> bool;
    auto empty(pile const& pile) -> bool;
    auto limit_size(pile const& pile, isize numCards) -> bool;

    namespace stack {
        auto top(pile const& target, point_i pos) -> isize;
        auto top_or_pile(pile const& target, point_i pos) -> isize;
        auto face_up(pile const& target, point_i pos) -> isize;
        auto in_seq(games::base_game const* game, pile const& target, point_i pos) -> isize;
        auto in_seq_in_suit(games::base_game const* game, pile const& target, point_i pos) -> isize;
        auto super_move(games::base_game const* game, pile const& target, point_i pos) -> isize;
    }
}

}
