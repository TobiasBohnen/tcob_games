#include "Cards.hpp"

namespace solitaire {

card::card(suit suit, rank rank, u8 deck)
    : _suit {suit}
    , _rank {rank}
    , _deck {deck}
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
    std::string retValue;

    switch (_suit) {
    case suit::Hearts: retValue += "hearts_"; break;
    case suit::Diamonds: retValue += "diamonds_"; break;
    case suit::Clubs: retValue += "clubs_"; break;
    case suit::Spades: retValue += "spades_"; break;
    }

    switch (_rank) {
    case rank::Ace: retValue += "A"; break;
    case rank::Two: retValue += "02"; break;
    case rank::Three: retValue += "03"; break;
    case rank::Four: retValue += "04"; break;
    case rank::Five: retValue += "05"; break;
    case rank::Six: retValue += "06"; break;
    case rank::Seven: retValue += "07"; break;
    case rank::Eight: retValue += "08"; break;
    case rank::Nine: retValue += "09"; break;
    case rank::Ten: retValue += "10"; break;
    case rank::Jack: retValue += "J"; break;
    case rank::Queen: retValue += "Q"; break;
    case rank::King: retValue += "K"; break;
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
    suit s {static_cast<suit>((value >> 12) & 0b0001111)};
    rank r {static_cast<rank>((value >> 8) & 0b00001111)};
    u8   deck {static_cast<u8>((value >> 1) & 0b01111111)};

    card retValue {s, r, deck};
    retValue._faceDown = (value & 1) != 0;

    return retValue;
}

////////////////////////////////////////////////////////////

deck::deck(u8 num)
{
    Cards.reserve(52);
    for (i32 s {static_cast<i32>(suit::Hearts)}; s <= static_cast<i32>(suit::Spades); ++s) {
        for (i32 r {static_cast<i32>(rank::Ace)}; r <= static_cast<i32>(rank::King); ++r) {
            Cards.emplace_back(static_cast<suit>(s), static_cast<rank>(r), num);
        }
    }
}

////////////////////////////////////////////////////////////

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

} // namespace solitaire
