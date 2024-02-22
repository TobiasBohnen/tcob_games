// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Spider.hpp"

#include <algorithm>
#include <utility>

#include "Field.hpp"

namespace solitaire::games {

////////////////////////////////////////////////////////////

spider::spider(field& f, game_info info)
    : base_game {f, std::move(info)}
{
}

game_info spider::Info = {.Name = "Spider", .Type = type::Spider, .Family = family::Spider, .DeckCount = 2, .CardDealCount = 10, .Redeals = 0};

spider::spider(field& f)
    : spider {f, Info}
{
    Stock.Position = {0, 0};
    Stock.Initial  = initial::face_down(50);
    add_pile(&Stock);

    create_piles(Foundation, 8, [](auto& foundation, i32 i) {
        foundation.Position = {i + 2.f, 0};
        foundation.Rule     = {.Build = build_type::NoBuilding, .Move = move_type::None};
    });

    create_piles(Tableau, 10, [](auto& tableau, i32 i) {
        tableau.Position = {i + 0.0f, 1.f};
        tableau.Initial  = initial::top_face_up(i < 4 ? 6 : 5);
        tableau.Layout   = layout_type::Column;
        tableau.Rule     = {.Build = build_type::DownByRank, .Move = move_type::InSequenceInSuit, .Empty {empty::Any()}};
    });
}

void spider::before_layout()
{
    for (auto& tableau : Tableau) {
        if (tableau.Cards.size() < 13) { continue; }

        // look for completed stack
        suit const targetSuit {tableau.Cards.back().get_suit()};
        bool       found {true};
        for (isize i {1}; i <= 13; ++i) {
            auto const& card {tableau.Cards[std::ssize(tableau.Cards) - i]};
            if (card.is_face_down() || card.get_suit() != targetSuit || card.get_rank() != static_cast<rank>(i)) {
                found = false;
                break;
            }
        }
        // move to foundation
        if (found) {
            for (auto& foundation : Foundation) {
                if (foundation.empty()) {
                    std::move(tableau.Cards.end() - 13, tableau.Cards.end(), std::back_inserter(foundation.Cards));
                    tableau.Cards.erase(tableau.Cards.end() - 13, tableau.Cards.end());
                    tableau.flip_up_top_card();
                    break;
                }
            }
        }
    }
}

auto spider::do_deal() -> bool
{
    if (!std::ranges::any_of(Tableau, [](auto const& tableau) { return tableau.empty(); })) {
        if (Stock.Cards.empty()) { return false; }

        for (auto& toPile : Tableau) {

            if (!Stock.Cards.empty()) {
                Stock.move_cards(toPile, std::ssize(Stock.Cards) - 1, 1, false);
            } else {
                break;
            }
            toPile.flip_up_top_card();
        }

        return true;
    }

    return false;
}

////////////////////////////////////////////////////////////

game_info arachnida::Info = {.Name = "Arachnida", .Type = type::Spider, .Family = family::Spider, .DeckCount = 2, .CardDealCount = 10, .Redeals = 0};

arachnida::arachnida(field& f)
    : spider {f, Info}
{
    Stock.Position = {0, 0};
    Stock.Initial  = initial::face_down(50);
    add_pile(&Stock);

    create_piles(Foundation, 1, [](auto& foundation, i32) {
        foundation.Position = {11.0f, 0.0f};
        foundation.Rule     = {.Build = build_type::NoBuilding, .Move = move_type::None};
    });

    create_piles(Tableau, 10, [](auto& tableau, i32 i) {
        tableau.Position = {i + 1.0f, 0.f};
        tableau.Initial  = initial::top_face_up(i < 4 ? 6 : 5);
        tableau.Layout   = layout_type::Column;
        tableau.Rule     = {.Build = build_type::InRankOrDownByRank, .Move = move_type::Other, .Empty {empty::Any()}};
    });
}

auto arachnida::stack_index(pile const& targetPile, point_i pos) const -> isize
{
    if (targetPile.Rule.Move == move_type::Other) {
        if (!targetPile.empty()) {
            isize const top {std::ssize(targetPile.Cards) - 1};
            rank const  targetRank {targetPile.Cards.back().get_rank()};
            suit const  targetSuit {targetPile.Cards.back().get_suit()};
            for (isize i {top}; i >= 0; --i) {
                auto const& card0 {targetPile.Cards[i]};

                if (i < top) {
                    auto const& card1 {targetPile.Cards[i + 1]};
                    if ((card0.get_suit() != targetSuit && card0.get_rank() != targetRank)
                        || card0.is_face_down()
                        || !can_drop(targetPile, i, card1, 1)) {
                        break;
                    }
                }

                if (card0.Bounds.contains(pos)) {
                    return i;
                }
            }
        }

        return INDEX_INVALID;
    }

    return spider::stack_index(targetPile, pos);
}

}
