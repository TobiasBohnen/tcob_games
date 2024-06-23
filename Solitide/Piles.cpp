// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Piles.hpp"

namespace solitaire {

auto build_none(card const&, card const&) -> bool
{
    return false;
}

auto move_top(pile const* target, isize idx) -> bool
{
    return idx == target->size() - 1;
}

auto base_none(card const&, isize) -> bool
{
    return false;
}

auto pile::is_playable() const -> bool
{
    return !empty()
        && !Cards.back().is_face_down()
        && Rule.IsPlayable();
}

auto pile::empty() const -> bool
{
    return Cards.empty();
}

auto pile::size() const -> isize
{
    return std::ssize(Cards);
}

void pile::remove_tint()
{
    constexpr color COLOR_DEFAULT {colors::White};
    constexpr color COLOR_COMPLETE {colors::Gray};
    color           c {COLOR_DEFAULT};
    if (Rule.Limit > 0 && std::ssize(Cards) >= Rule.Limit) { c = COLOR_COMPLETE; }

    if (Marker) { Marker->Color = c; }
    for (auto& card : Cards) { card.Color = c; }
}

void pile::tint_cards(color color, isize startIdx)
{
    if (startIdx == INDEX_MARKER) {
        Marker->Color = color;
    } else {
        for (; startIdx < size(); ++startIdx) { Cards[startIdx].Color = color; }
    }
}

void pile::flip_cards(std::vector<bool> const& val)
{
    for (isize i {0}; i < std::ssize(val); ++i) {
        if (i >= size()) { return; }
        if (val[i]) {
            Cards[i].flip_face_up();
        } else {
            Cards[i].flip_face_down();
        }
    }
}

void pile::flip_up_cards()
{
    for (auto& card : Cards) { card.flip_face_up(); }
}

void pile::flip_up_top_card()
{
    if (!empty()) { Cards.back().flip_face_up(); }
}

void pile::flip_down_cards()
{
    for (auto& card : Cards) { card.flip_face_down(); }
}

void pile::flip_down_top_card()
{
    if (!empty()) { Cards.back().flip_face_down(); }
}

auto static get_base_cards(base_func const& func) -> std::multimap<rank, suit>
{
    std::multimap<rank, suit> retValue;

    for (i32 i {0}; i < 52; ++i) {
        suit s {static_cast<suit>(i / 13)};
        rank r {static_cast<rank>(i % 13 + 1)};
        if (func({s, r, 0}, 1)) {
            retValue.emplace(r, s);
        }
    }

    return retValue;
}

auto get_pile_type_name(pile_type pt) -> std::string
{
    switch (pt) {
    case pile_type::Stock: return "Stock";
    case pile_type::Waste: return "Waste";
    case pile_type::Foundation: return "Foundation";
    case pile_type::Tableau: return "Tableau";
    case pile_type::Reserve: return "Reserve";
    case pile_type::FreeCell: return "FreeCell";
    }

    return "";
}

auto pile::get_marker_texture_name() const -> std::string
{
    // TODO:
    // redeal for Stock
    if (Type == pile_type::Foundation || Type == pile_type::Tableau) {
        auto const valid {get_base_cards(Rule.Base)};
        if (valid.size() == 52) {
            return "card_base_gen"; // Any
        }
        for (u8 cr {static_cast<u8>(rank::Ace)}; cr <= static_cast<u8>(rank::King); ++cr) {
            if (valid.count(static_cast<rank>(cr)) == 4) {
                return "card_base_" + helper::to_lower(get_rank_name(static_cast<rank>(cr)));
            }
        }
        return "card_base_gen";
    }

    return "card_empty";
}

void pile::move_cards(pile& to, isize startIndex, isize numCards, bool reverse)
{
    if (startIndex < 0) {
        numCards += startIndex;
        startIndex = 0;
    }

    if (numCards == 0) { return; }

    for (isize i {0}; i < numCards; ++i) {
        auto& card {Cards[i + startIndex]};
        if (reverse) {
            to.Cards.emplace_front(card);
        } else {
            to.Cards.emplace_back(card);
        }
    }

    Cards.erase(Cards.begin() + startIndex, Cards.begin() + startIndex + numCards);

    if (Type != pile_type::Stock) { flip_up_top_card(); }
    remove_tint();
    to.remove_tint();
}

static auto fill(pile const& pile, card const& card0, isize numCards) -> bool
{
    if (!pile.empty()) { return false; }
    return pile.Rule.Base(card0, numCards);
}

static auto limit_size(pile const& pile, isize numCards) -> bool
{
    if (pile.Rule.Limit == UNLIMITED) { return true; }
    return pile.size() + numCards <= pile.Rule.Limit;
}

auto pile::build(isize targetIndex, card const& card, isize numCards) const -> bool
{
    if ((!Rule.IsSequence) && numCards > 1) { return false; }

    if (!limit_size(*this, numCards)) { return false; }

    if (fill(*this, card, numCards)) { return true; }
    if (empty() || targetIndex < 0) { return false; } // can't fill and is empty

    auto const& target {Cards[targetIndex]};
    if (target.is_face_down()) { return false; }

    return Rule.Build(target, card);
}

void pile::reset()
{
    Cards.clear();
    IsHovering = false;
    IsDragging = false;
    remove_tint();
}

////////////////////////////////////////////////////////////

stock::stock()
{
    Type            = pile_type::Stock;
    Rule.IsPlayable = [] { return false; };
}

waste::waste()
{
    Type = pile_type::Waste;
}

tableau::tableau()
{
    Type = pile_type::Tableau;
}

foundation::foundation()
{
    Type       = pile_type::Foundation;
    Rule.Limit = AUTO_LIMIT;
}

reserve::reserve()
{
    Type = pile_type::Reserve;
}

freecell::freecell()
{
    Type = pile_type::FreeCell;
}
}
