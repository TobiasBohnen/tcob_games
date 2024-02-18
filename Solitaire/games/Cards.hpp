// Copyright (c) 2024 Tobias Bohnen
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

////////////////////////////////////////////////////////////

class card {
public:
    card() = default;
    card(suit suit, rank rank, u8 deck);

    rect_f Bounds;
    color  Color {colors::White};
    auto   get_texture_name() const -> std::string;

    auto get_suit() const -> suit;
    auto get_rank() const -> rank;

    void flip_face_up();
    void flip_face_down();
    auto is_face_down() const -> bool;

    auto get_name() const -> std::string;

    auto to_value() const -> u16;
    auto static FromValue(u16 value) -> card;

    void static Serialize(card const& v, auto&& s)
    {
        s["Deck"]  = v._deck;
        s["Suit"]  = v._suit;
        s["Color"] = get_suit_color(v._suit);
        s["Rank"]  = v._rank;
    }
    auto static Deserialize(card& v, auto&& s) -> bool
    {
        return s.try_get(v._deck, "Deck") && s.try_get(v._suit, "Suit") && s.try_get(v._rank, "Rank");
    }

    auto operator==(card const& other) const -> bool
    {
        return _deck == other._deck
            && _suit == other._suit
            && _rank == other._rank;
    }

private:
    suit _suit {};
    rank _rank {};
    u8   _deck {};
    bool _faceDown {true};
};

////////////////////////////////////////////////////////////

struct deck {
    deck(u8 num);

    std::vector<card> Cards;

    auto static GetShuffled(auto&& rand, u8 num) -> deck
    {
        deck deck {num};
        rand.template shuffle<card>(deck.Cards);
        return deck;
    }
};

}
