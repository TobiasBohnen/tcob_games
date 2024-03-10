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
    InSequenceInSuitOrSameRank,
    SuperMove
};

using empty_func = std::function<bool(card const&, isize)>;

struct rule {
    build_type Build {build_type::NoBuilding};
    i32        Interval {1};
    bool       Wrap {false};
    move_type  Move {move_type::Top};
    empty_func Empty {[](auto, auto) { return false; }};
    i32        Limit {-1};
};

//////////////////////////////////////////

namespace rules {
    auto build(pile const& targetPile, isize targetIndex, card const& drop, isize numCards) -> bool;
}

namespace stack {
    auto top(pile const& target, isize idx) -> bool;
    auto top_or_pile(pile const& target, isize idx) -> bool;
    auto face_up(pile const& target, isize idx) -> bool;
    auto in_seq(games::base_game const* game, pile const& target, isize idx) -> bool;
    auto in_seq_in_suit(games::base_game const* game, pile const& target, isize idx) -> bool;
    auto in_seq_in_suit_same_rank(games::base_game const* game, pile const& target, isize idx) -> bool;
    auto super_move(games::base_game const* game, pile const& target, isize idx) -> bool;
}

}
