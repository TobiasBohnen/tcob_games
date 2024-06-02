-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local grandee = {
    Info = {
        Name      = "Grandee",
        Family    = "Other",
        DeckCount = 2
    },
    Stock = {
        Position = { x = 7, y = 4 },
        Initial = Sol.Initial.face_down(90)
    },
    Foundation = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i, y = 1 },
                Rule     = Sol.Rules.ace_upsuit_top
            }
        end
    },
    Tableau = {
        Size = 14,
        Pile = function(i)
            return {
                Position = { x = i % 7 + 0.5, y = i // 7 + 2 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Squared,
                Rule     = Sol.Rules.any_downsuit_top
            }
        end
    },
    deal = Sol.Ops.Deal.stock_to_tableau
}


------

------------------------

Sol.register_game(grandee)
