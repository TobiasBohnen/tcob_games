-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local fortunes_favor = {
    Info            = {
        Name      = "Fortune's Favor",
        Family    = "FortyThieves",
        DeckCount = 1
    },
    Stock           = {
        Position = { x = 0, y = 0 },
        Initial = Sol.Initial.face_down(36)
    },
    Waste           = { Position = { x = 0, y = 1 } },
    Foundation      = {
        Size = 4,
        Pile = function(i)
            return {
                Position = { x = i + 2, y = 0 },
                Rule = Sol.Rules.ace_upsuit_top,
            }
        end
    },
    Tableau         = {
        Size = 12,
        Pile = function(i)
            return {
                Position = { x = i % 4 + 2, y = i // 4 + 1 },
                Initial  = Sol.Initial.face_up(1),
                Layout   = Sol.Pile.Layout.Squared,
                Rule     = Sol.Rules.any_downsuit_top
            }
        end
    },
    on_before_setup = Sol.Ops.Shuffle.ace_to_foundation,
    deal            = Sol.Ops.Deal.stock_to_waste,
    on_end_turn     = Sol.Ops.Deal.waste_or_stock_to_empty_tableau
}


------

local preference         = Sol.copy(fortunes_favor)
preference.Info.Name     = "Preference"
preference.Stock.Initial = Sol.Initial.face_down(40)
preference.Tableau.Size  = 8


------

------------------------

Sol.register_game(fortunes_favor)
Sol.register_game(preference)
