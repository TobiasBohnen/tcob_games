// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

////////////////////////////////////////////////////////////

enum class value_category : u8 {
    None,
    OnePair,
    TwoPair,
    ThreeOfAKind,
    FullHouse,
    Straight,
    FourOfAKind,
    FiveOfAKind,
};
enum class color_category : u8 {
    None,
    Flush,
    Rainbow,
};

////////////////////////////////////////////////////////////

struct hand {
    value_category Value {value_category::None};
    color_category Color {color_category::None};
};

static constexpr auto to_string(value_category v) -> std::string_view
{
    switch (v) {
    case value_category::None:         return "None";
    case value_category::OnePair:      return "One Pair";
    case value_category::TwoPair:      return "Two Pair";
    case value_category::ThreeOfAKind: return "Three of a Kind";
    case value_category::FourOfAKind:  return "Four of a Kind";
    case value_category::FiveOfAKind:  return "Five of a Kind";
    case value_category::FullHouse:    return "Full House";
    case value_category::Straight:     return "Straight";
    }
    return "Unknown";
}

static constexpr auto to_string(color_category v) -> std::string_view
{
    switch (v) {
    case color_category::None:    return "None";
    case color_category::Flush:   return "Flush";
    case color_category::Rainbow: return "Rainbow";
    }
    return "Unknown";
}
