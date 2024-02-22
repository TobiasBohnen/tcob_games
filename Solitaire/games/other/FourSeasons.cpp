// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "FourSeasons.hpp"

#include "Field.hpp"

namespace solitaire::games {

////////////////////////////////////////////////////////////

game_info four_seasons::Info = {.Name = "Four Seasons", .Type = type::SimplePacker, .DeckCount = 1, .CardDealCount = 1, .Redeals = 0};

four_seasons::four_seasons(field& f)
    : base_game {f, Info}
{
    Stock.Position = {0, 0};
    Stock.Initial  = initial::face_down(46);
    add_pile(&Stock);

    Waste.Position = {1, 0};
    add_pile(&Waste);

    create_piles(Foundation, 4, [&](auto& foundation, i32) {
        foundation.Rule = {.Build = build_type::UpInSuit, .Wrap = true, .Move = move_type::None, .Empty {empty::First(Foundation[0])}};
    });

    Foundation[0].Position = {0, 1};
    Foundation[0].Initial  = initial::face_up(1);
    Foundation[1].Position = {0, 3};
    Foundation[2].Position = {4, 1};
    Foundation[3].Position = {4, 3};

    create_piles(Tableau, 5, [](auto& tableau, i32) {
        tableau.Initial = initial::face_up(1);
        tableau.Rule    = {.Build = build_type::DownByRank, .Wrap = true, .Move = move_type::Top, .Empty {empty::Any()}};
        tableau.Layout  = layout_type::Squared;
    });

    Tableau[0].Position = {2, 1};
    Tableau[1].Position = {1, 2};
    Tableau[2].Position = {2, 2};
    Tableau[3].Position = {3, 2};
    Tableau[4].Position = {2, 3};
}

auto four_seasons::do_deal() -> bool
{
    if (Stock.empty()) { return false; }

    for (i32 i {0}; i < info().CardDealCount; ++i) {
        Stock.move_cards(Waste, std::ssize(Stock.Cards) - 1, 1, false);
    }
    Waste.flip_up_cards();

    return true;
}

}
