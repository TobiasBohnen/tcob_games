-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local easthaven                  = {
    Info       = {
        Name      = "Easthaven",
        Family    = "Gypsy",
        DeckCount = 1
    },
    Stock      = {
        Initial = Sol.Initial.face_down(31)
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 7,
        Pile = {
            Initial = Sol.Initial.top_face_up(3),
            Layout = "Column",
            Rule = Sol.Rules.any_downac_inseq
        }
    },
    on_init    = Sol.Layout.klondike,
    do_deal    = function(game) return Sol.Ops.Deal.to_group(game.Stock[1], game.Tableau, false) end
}

------

local double_easthaven           = Sol.copy(easthaven)
double_easthaven.Info.Name       = "Double Easthaven"
double_easthaven.Info.DeckCount  = 2
double_easthaven.Stock.Initial   = Sol.Initial.face_down(80)
double_easthaven.Foundation.Size = 8
double_easthaven.Tableau.Size    = 8

------

local triple_easthaven           = Sol.copy(easthaven)
triple_easthaven.Info.Name       = "Triple Easthaven"
triple_easthaven.Info.DeckCount  = 3
triple_easthaven.Stock.Initial   = Sol.Initial.face_down(120)
triple_easthaven.Foundation.Size = 12
triple_easthaven.Tableau.Size    = 12

------

local westhaven                  = Sol.copy(easthaven)
westhaven.Info.Name              = "Westhaven"
westhaven.Stock.Initial          = Sol.Initial.face_down(22)
westhaven.Foundation.Pile.Rule   = Sol.Rules.ace_upsuit_none
westhaven.Tableau.Size           = 10

------

------------------------

Sol.register_game(easthaven)
Sol.register_game(double_easthaven)
Sol.register_game(triple_easthaven)
Sol.register_game(westhaven)
