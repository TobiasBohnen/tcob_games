// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Cards.hpp"

namespace solitaire {

card::card(suit suit, rank rank, u8 deck, bool faceDown)
    : _suit {suit}
    , _rank {rank}
    , _deck {deck}
    , _faceDown {faceDown}
{
}

auto card::get_suit() const -> suit
{
    return _suit;
}

auto card::get_rank() const -> rank
{
    return _rank;
}

void card::flip_face_up()
{
    _faceDown = false;
}

void card::flip_face_down()
{
    _faceDown = true;
}

auto card::is_face_down() const -> bool
{
    return _faceDown;
}

auto card::get_name() const -> std::string
{
    std::string retValue {std::format("{:02}", static_cast<u8>(_rank))};

    switch (_suit) {
    case suit::Hearts: retValue += "h"; break;
    case suit::Diamonds: retValue += "d"; break;
    case suit::Clubs: retValue += "c"; break;
    case suit::Spades: retValue += "s"; break;
    }

    return retValue;
}

auto card::get_texture_name() const -> std::string
{
    if (_faceDown) { return "card_back"; }

    std::string retValue {"card_"};
    retValue += get_name();
    return retValue;
}

auto card::to_value() const -> u16
{
    return static_cast<u16>(
        (static_cast<u32>(_suit) << 12) | (static_cast<u32>(_rank) << 8) | (static_cast<u32>(_deck) << 1) | (_faceDown ? 1 : 0));
}

auto card::FromValue(u16 value) -> card
{
    suit const s {static_cast<suit>((value >> 12) & 0b0001111)};
    rank const r {static_cast<rank>((value >> 8) & 0b00001111)};
    u8 const   deck {static_cast<u8>((value >> 1) & 0b01111111)};
    bool const faceDown {(value & 1) != 0};

    return {s, r, deck, faceDown};
}

////////////////////////////////////////////////////////////

deck::deck(u8 num, std::unordered_set<suit> const& suits, std::unordered_set<rank> const& ranks)
{
    Cards.reserve(52);
    for (u8 s {static_cast<u8>(suit::Hearts)}; s <= static_cast<u8>(suit::Spades); ++s) {
        if (suits.contains(static_cast<suit>(s))) {
            for (u8 r {static_cast<u8>(rank::Ace)}; r <= static_cast<u8>(rank::King); ++r) {
                if (ranks.contains(static_cast<rank>(r))) {
                    Cards.emplace_back(static_cast<suit>(s), static_cast<rank>(r), num);
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////

auto get_next_rank(rank r, i32 interval, bool wrap) -> std::optional<rank>
{
    i32 target {static_cast<i32>(r) + interval};
    if (wrap) {
        target = ((target - 1) % 13 + 13) % 13 + 1;
    }

    if (target == 0 || target > 13) {
        return std::nullopt;
    }

    return static_cast<rank>(target);
}

auto get_suit_color(suit s) -> suit_color
{
    switch (s) {
    case suit::Hearts:
    case suit::Diamonds:
        return suit_color::Red;
    case suit::Clubs:
    case suit::Spades:
        return suit_color::Black;
    }
}

auto get_rank_symbol(rank r) -> std::string
{
    switch (r) {
    case rank::Ace: return "A";
    case rank::Two: return "2";
    case rank::Three: return "3";
    case rank::Four: return "4";
    case rank::Five: return "5";
    case rank::Six: return "6";
    case rank::Seven: return "7";
    case rank::Eight: return "8";
    case rank::Nine: return "9";
    case rank::Ten: return "10";
    case rank::Jack: return "J";
    case rank::Queen: return "Q";
    case rank::King: return "K";
    }

    return "";
}

auto get_rank_name(rank r) -> std::string
{
    switch (r) {
    case rank::Ace: return "Ace";
    case rank::Two: return "Two";
    case rank::Three: return "Three";
    case rank::Four: return "Four";
    case rank::Five: return "Five";
    case rank::Six: return "Six";
    case rank::Seven: return "Seven";
    case rank::Eight: return "Eight";
    case rank::Nine: return "Nine";
    case rank::Ten: return "Ten";
    case rank::Jack: return "Jack";
    case rank::Queen: return "Queen";
    case rank::King: return "King";
    }

    return "";
}

} // namespace solitaire
