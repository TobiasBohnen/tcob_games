// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Piles.hpp"

namespace solitaire {

auto initial::top_face_up(usize size) -> std::vector<bool>
{
    std::vector<bool> retValue(size, false);
    retValue[size - 1] = true;
    return retValue;
}

auto initial::face_up(usize size) -> std::vector<bool>
{
    return std::vector<bool>(size, true);
}

auto initial::face_down(usize size) -> std::vector<bool>
{
    return std::vector<bool>(size, false);
}

////////////////////////////////////////////////////////////

void pile::set_active(bool b, isize idx)
{
    _isActive = b;
    if (!b) {
        remove_tint();
    } else {
        tint_cards(COLOR_HOVER, idx);
    }
}

auto pile::is_active() const -> bool
{
    return _isActive;
}

auto pile::is_playable() const -> bool
{
    return Rule.Move != move_type::None
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

auto static get_valid_cards(empty::func const& f) -> std::multimap<rank, suit>
{
    std::multimap<rank, suit> retValue;

    for (i32 i {0}; i < 52; ++i) {
        suit s {static_cast<suit>(i / 13)};
        rank r {static_cast<rank>(i % 13 + 1)};
        if (f({s, r, 0})) {
            retValue.emplace(r, s);
        }
    }

    return retValue;
}

auto static get_rank_name(empty::func const& s) -> std::string
{
    std::array<bool, 26>                     ranks {};
    static std::array<std::string, 13> const rankNames = {"Ace", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King"};

    for (i32 i {0}; i < 26; ++i) {
        ranks[i] = s({i < 13 ? suit::Clubs : suit::Diamonds, static_cast<rank>(i % 13 + 1), 0});
    }

    isize const countTrue {std::ranges::count(ranks, true)};
    if (countTrue == 26) { return "Any"; }
    if (countTrue == 0) { return "None"; }

    std::string retValue;
    for (i32 i {0}; i < 13; ++i) {
        if (ranks[i] && ranks[i + 13]) {
            retValue += rankNames[i];
        } else if (ranks[i]) {
            retValue += "Black " + rankNames[i];
        } else if (ranks[i + 13]) {
            retValue += "Red " + rankNames[i];
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
        std::string redeals {remainingRedeals < 0 ? "unlimited" : std::to_string(remainingRedeals)};
        retValue.Rule = "Redeals: " + redeals;
    } break;
    case pile_type::Foundation:
    case pile_type::Tableau: {
        retValue.Pile      = get_pile_type_name(Type);
        retValue.Rule      = get_building_hint_text(Rule.Build) + "\nFirst: " + get_rank_name(Rule.Empty.Accept);
        retValue.CardCount = std::to_string(cardCount);
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
        auto const valid {get_valid_cards(Rule.Empty.Accept)};
        if (valid.size() == 52) {
            return "card_base_gen"; // Any
        }
        if (valid.count(rank::Ace) == 4) {
            return "card_base_ace";
        }
        if (valid.count(rank::King) == 4) {
            return "card_base_king";
        }
        return "card_base_gen";
    }

    return "card_empty";
}

void pile::move_cards(pile& to, isize srcOffset, isize numCards, bool reverse)
{
    if (srcOffset < 0) {
        numCards += srcOffset;
        srcOffset = 0;
    }

    for (isize i {0}; i < numCards; ++i) {
        auto& card {Cards[i + srcOffset]};
        if (reverse) {
            to.Cards.emplace_front(card);
        } else {
            to.Cards.emplace_back(card);
        }
    }

    Cards.erase(Cards.begin() + srcOffset, Cards.begin() + srcOffset + numCards);

    if (Type != pile_type::Stock) { flip_up_top_card(); }
}

////////////////////////////////////////////////////////////

stock::stock()
{
    Type      = pile_type::Stock;
    Rule.Move = move_type::None;
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
