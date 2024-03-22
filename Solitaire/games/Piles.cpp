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
    return idx == std::ssize(target->Cards) - 1;
}

auto empty_none(card const&, isize) -> bool
{
    return false;
}

void pile::set_active(bool b, isize idx, color color)
{
    _isActive = b;
    if (!b) {
        remove_tint();
    } else {
        tint_cards(color, idx);
    }
}

auto pile::is_active() const -> bool
{
    return _isActive;
}

auto pile::is_playable() const -> bool
{
    return Rule.IsPlayable
        && !empty()
        && !Cards.back().is_face_down();
}

auto pile::empty() const -> bool
{
    return Cards.empty();
}

void pile::remove_tint()
{
    constexpr color COLOR_DEFAULT {colors::White};

    if (Marker) {
        Marker->Color = COLOR_DEFAULT;
    }
    for (auto& card : Cards) {
        card.Color = COLOR_DEFAULT;
    }
}

void pile::tint_cards(color color, isize idx)
{
    if (idx == INDEX_MARKER) {
        Marker->Color = color;
        ++idx;
    } else {
        for (; idx < std::ssize(Cards); ++idx) {
            Cards[idx].Color = color;
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

auto static get_valid_cards(empty_func const& func) -> std::multimap<rank, suit>
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

auto static get_empty_ranks(empty_func const& func) -> std::string
{
    std::array<bool, 26> ranks {};

    for (i32 i {0}; i < 26; ++i) {
        ranks[i] = func({i < 13 ? suit::Clubs : suit::Diamonds, static_cast<rank>(i % 13 + 1), 0}, 1);
    }

    isize const countTrue {std::ranges::count(ranks, true)};
    if (countTrue == 26) { return "Any"; }
    if (countTrue == 0) { return "None"; }

    std::string retValue;
    for (u8 cr {static_cast<u8>(rank::Ace)}; cr <= static_cast<u8>(rank::King); ++cr) {
        auto const r {get_rank_name(static_cast<rank>(cr + 1))};
        if (ranks[cr] && ranks[cr + 13]) {
            retValue += r;
        } else if (ranks[cr]) {
            retValue += "Black " + r;
        } else if (ranks[cr + 13]) {
            retValue += "Red " + r;
        }
    }
    return retValue;
}

auto get_pile_type_name(pile_type s) -> std::string
{
    switch (s) {
    case pile_type::Stock: return "Stock";
    case pile_type::Waste: return "Waste";
    case pile_type::Foundation: return "Foundation";
    case pile_type::Tableau: return "Tableau";
    case pile_type::Reserve: return "Reserve";
    case pile_type::FreeCell: return "FreeCell";
    }

    return "";
}

auto pile::get_description(i32 remainingRedeals) const -> hover_info
{
    auto const cardCount {Cards.size()};

    hover_info retValue;
    retValue.Pile      = get_pile_type_name(Type);
    retValue.CardCount = std::to_string(cardCount);

    switch (Type) {
    case pile_type::Waste:
    case pile_type::Reserve:
    case pile_type::FreeCell:
        break;
    case pile_type::Stock: {
        std::string redeals {remainingRedeals < 0 ? "∞" : std::to_string(remainingRedeals)};
        retValue.Rule = "Redeals: " + redeals;
    } break;
    case pile_type::Foundation:
    case pile_type::Tableau: {
        retValue.Rule = Rule.BuildHint + "\nBase: " + get_empty_ranks(Rule.Base);
        break;
    }
    }

    // TODO: translate
    return retValue;
}

auto pile::get_marker_texture_name() const -> std::string
{
    // TODO:
    // empty for Waste/Reserve/FreeCell
    // redeal for Stock
    // rank for Foundation/Tableau
    if (Type == pile_type::Foundation || Type == pile_type::Tableau) {
        auto const valid {get_valid_cards(Rule.Base)};
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
}

static auto fill(pile const& pile, card const& card0, isize numCards) -> bool
{
    if (!pile.empty()) { return false; }
    return pile.Rule.Base(card0, numCards);
}

static auto limit_size(pile const& pile, isize numCards) -> bool
{
    if (pile.Rule.Limit < 0) { return true; }
    return std::ssize(pile.Cards) + numCards <= pile.Rule.Limit;
}

auto pile::build(isize targetIndex, card const& drop, isize numCards) const -> bool
{
    if ((!Rule.IsSequence) && numCards > 1) { return false; }

    if (!limit_size(*this, numCards)) { return false; }

    if (fill(*this, drop, numCards)) { return true; }
    if (empty() || targetIndex < 0) { return false; } // can't fill and is empty

    card const& target {Cards[targetIndex]};
    if (target.is_face_down()) { return false; }

    return Rule.Build(target, drop);
}

////////////////////////////////////////////////////////////

stock::stock()
{
    Type            = pile_type::Stock;
    Rule.IsPlayable = false;
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
    Type = pile_type::Foundation;
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
