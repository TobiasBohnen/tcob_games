// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Piles.hpp"

#include "Games.hpp"

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

void pile::set_hovering(bool b, isize idx, color color)
{
    _isHovering = b;
    if (!b) {
        remove_tint();
    } else {
        tint_cards(color, idx);
    }
}

auto pile::is_hovering() const -> bool
{
    return _isHovering;
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

void pile::flip_cards(std::vector<bool> const& val)
{
    for (usize i {0}; i < val.size(); ++i) {
        if (i >= Cards.size()) { return; }
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

auto static get_base_cards(empty_func const& func) -> std::multimap<rank, suit>
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

    // TODO: translate
    return "";
}

auto pile::get_description(games::base_game const& game) const -> pile_description
{
    auto const cardCount {Cards.size()};

    pile_description retValue;
    retValue.Pile      = get_pile_type_name(Type);
    retValue.CardCount = std::to_string(cardCount);

    switch (Type) {
    case pile_type::Stock: {
        retValue.Description      = game.info().RemainingRedeals < 0 ? "∞" : std::to_string(game.info().RemainingRedeals);
        retValue.DescriptionLabel = "Redeals";
    } break;
    case pile_type::Waste:
    case pile_type::Reserve:
    case pile_type::FreeCell:
    case pile_type::Foundation:
    case pile_type::Tableau: {
        retValue.Description      = Rule.BuildHint;
        retValue.DescriptionLabel = "Build";
        retValue.Move             = Rule.MoveHint;
        retValue.MoveLabel        = "Move";
        retValue.Base             = Rule.BaseHint;
        retValue.BaseLabel        = "Base";
        break;
    }
    }

    // TODO: translate
    return retValue;
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
