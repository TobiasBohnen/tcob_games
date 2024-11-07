-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local robert = {
    Info       = {
        Name      = "Robert",
        Family    = "Golf",
        DeckCount = 1,
        Redeals   = 2
    },
    Stock      = {
        Position = { x = 0, y = 1 },
        Initial = Sol.Initial.face_down(51)
    },
    Waste      = {
        Position = { x = 1, y = 1 }
    },
    Foundation = {
        Position = { x = 0.5, y = 0 },
        Initial  = Sol.Initial.face_up(1),
        Layout   = Sol.Pile.Layout.Squared,
        Rule     = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.UpOrDownByRank(true), Move = Sol.Rules.Move.None() }
    },
    deal       = Sol.Ops.Deal.stock_to_waste,
    redeal     = Sol.Ops.Redeal.waste_to_stock
}


------

local wasatch        = Sol.copy(robert)
wasatch.Info.Name    = "Wasatch"
wasatch.Info.Redeals = -1
wasatch.deal         = Sol.Ops.Deal.stock_to_waste_by_3


------

local bobby      = Sol.copy(robert)
bobby.Info.Name  = "Bobby"
bobby.Foundation = {
    Size = 2,
    Pile = function(i)
        return {
            Position = { x = i, y = 0 },
            Initial  = Sol.Initial.face_up(1 - i),
            Layout   = Sol.Pile.Layout.Squared,
            Rule     = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.UpOrDownByRank(true), Move = Sol.Rules.Move.None() }
        }
    end
}


------

------------------------

Sol.register_game(robert)
Sol.register_game(bobby)
Sol.register_game(wasatch)
