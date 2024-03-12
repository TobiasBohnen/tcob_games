// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Cards.hpp"
#include "Common.hpp" // IWYU pragma: keep

namespace solitaire {

//////////////////

/*
auto static get_building_hint_text(build_type h) -> std::string
{
    switch (h) {
    case build_type::Any: return "Any card.";
    case build_type::NoBuilding: return "No building.";

    case build_type::DownByRank: return "Build down by rank.";
    case build_type::UpByRank: return "Build up by rank.";
    case build_type::UpOrDownByRank: return "Build up or down by rank.";

    case build_type::DownInSuit: return "Build down by suit.";
    case build_type::UpInSuit: return "Build up by suit.";
    case build_type::UpOrDownInSuit: return "Build up or down by suit.";

    case build_type::DownAlternateColors: return "Build down by alternate color.";
    case build_type::UpAlternateColors: return "Build up by alternate color.";
    case build_type::UpOrDownAlternateColors: return "Build up or down by alternate color.";

    case build_type::DownAnyButOwnSuit: return "Build down by any suit but own.";
    case build_type::UpAnyButOwnSuit: return "Build up by any suit but own.";
    case build_type::UpOrDownAnyButOwnSuit: return "Build up or down by any suit but own.";

    case build_type::DownInColor: return "Build down by color.";
    case build_type::UpInColor: return "Build up by color.";
    case build_type::UpOrDownInColor: return "Build up or down by color.";

    case build_type::InRank: return "Build by same rank.";
    case build_type::InRankOrDownByRank: return "Build down by rank or by same rank.";
    case build_type::RankPack: return "Build by same rank, then build up by rank.";
    }

    return "";
}
*/

using build_func = std::function<bool(card const& target, card const& drop, i32 interval, bool wrap)>;
using move_func  = std::function<bool(games::base_game const* game, pile const* target, isize idx)>;
using empty_func = std::function<bool(card const&, isize)>;

namespace move {
    auto top(games::base_game const*, pile const* target, isize idx) -> bool;
}

struct rule {
    std::string BuildHint;
    build_func  Build {[](auto, auto, auto, auto) { return false; }};

    bool      IsPlayable {true};
    bool      IsSequence {false};
    move_func Move {move::top};

    empty_func Empty {[](auto, auto) { return false; }};

    i32  Interval {1};
    bool Wrap {false};
    i32  Limit {-1};
};

//////////////////////////////////////////

namespace rules {
    auto build(pile const& targetPile, isize targetIndex, card const& drop, isize numCards) -> bool;
}

}
