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

using move_func  = std::function<bool(games::base_game const* game, pile const* target, isize idx)>;
using empty_func = std::function<bool(card const&, isize)>;

namespace move {
    auto top(games::base_game const*, pile const* target, isize idx) -> bool;
}

struct rule {
    build_type Build {build_type::NoBuilding};
    i32        Interval {1};
    bool       Wrap {false};
    bool       IsPlayable {true};
    bool       IsSequence {false};
    move_func  Move {move::top};
    empty_func Empty {[](auto, auto) { return false; }};
    i32        Limit {-1};
};

//////////////////////////////////////////

namespace rules {
    auto build(pile const& targetPile, isize targetIndex, card const& drop, isize numCards) -> bool;
}

}
