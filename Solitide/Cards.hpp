// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

namespace solitaire {

////////////////////////////////////////////////////////////

enum class suit : u8 {
    Hearts,
    Diamonds,
    Clubs,
    Spades
};

enum class suit_color : u8 {
    Red,
    Black
};

auto get_suit_color(suit s) -> suit_color;

enum class rank : u8 {
    Ace   = 1,
    Two   = 2,
    Three = 3,
    Four  = 4,
    Five  = 5,
    Six   = 6,
    Seven = 7,
    Eight = 8,
    Nine  = 9,
    Ten   = 10,
    Jack  = 11,
    Queen = 12,
    King  = 13
};

auto get_rank_symbol(rank r) -> std::string;
auto get_rank_name(rank r) -> std::string;

////////////////////////////////////////////////////////////

class card {
public:
    card() = default;
    card(suit suit, rank rank, u8 deck, bool faceDown = true);

    rect_f Bounds;
    color  Color {colors::White};

    auto get_suit() const -> suit;
    auto get_rank() const -> rank;

    void flip_face_up();
    void flip_face_down();
    auto is_face_down() const -> bool;

    auto get_texture_name() const -> std::string;

    auto to_value() const -> u16;
    auto static FromValue(u16 value) -> card;

    auto static constexpr Members()
    {
        return std::tuple {
            member<&card::_deck> {"Deck"},
            member<&card::_suit> {"Suit"},
            member<&card::_rank> {"Rank"},
            member<&card::_faceDown> {"IsFaceDown"},
            computed_member<
                [](auto&& val) { return get_suit_color(val._suit); },
                [](auto&&, auto&&) {}> {"Color"},
            computed_member<
                [](auto&& val) { return !val._faceDown; },
                [](auto&&, auto&&) {}> {"IsFaceUp"},
        };
    }

    auto operator==(card const& other) const -> bool
    {
        return _deck == other._deck
            && _suit == other._suit
            && _rank == other._rank;
    }

private:
    auto name() const -> std::string;

    suit _suit {};
    rank _rank {};
    u8   _deck {};
    bool _faceDown {true};
};

////////////////////////////////////////////////////////////

struct deck {
    deck(u8 num, std::unordered_set<suit> const& suits, std::unordered_set<rank> const& ranks);

    std::vector<card> Cards;

    auto static GetShuffled(auto&& rand, i32 num, std::unordered_set<suit> const& suits, std::unordered_set<rank> const& ranks) -> std::vector<card>
    {
        std::vector<card> retValue;
        for (u8 i {0}; i < num; ++i) {
            deck deck {i, suits, ranks};
            retValue.insert(retValue.end(), deck.Cards.begin(), deck.Cards.end());
        }

        rand(retValue);
        return retValue;
    }
};

}
