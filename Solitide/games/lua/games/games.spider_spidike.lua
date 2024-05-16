-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local spidike = {
    Info       = {
        Name      = "Spidike",
        Family    = "Spider",
        DeckCount = 1
    },
    Stock      = {
        Initial = Sol.Initial.face_down(24)
    },
    Foundation = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_upsuit_top }
    },
    Tableau    = {
        Size = 7,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i + 1),
                Layout  = Sol.Pile.Layout.Column,
                Rule    = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.DownByRank(), Move = Sol.Rules.Move.InSeqInSuit() }
            }
        end
    },
    deal       = Sol.Ops.Deal.stock_to_tableau,
    on_init    = Sol.Layout.klondike
}


------

local incompatibility           = Sol.copy(spidike)
incompatibility.Info.Name       = "Incompatibility"
incompatibility.Info.DeckCount  = 2
incompatibility.Stock.Initial   = Sol.Initial.face_down(54)
incompatibility.Foundation.Size = 8
incompatibility.Tableau         = {
    Size = 10,
    Pile = {
        Initial = Sol.Initial.face_up(5),
        Layout  = Sol.Pile.Layout.Column,
        Rule    = { Base = Sol.Rules.Base.King(), Build = Sol.Rules.Build.DownByRank(), Move = Sol.Rules.Move.InSeqInSuit() }
    }
}
incompatibility.deal            = function(game)
    return Sol.Ops.Deal.to_group(game.Stock[1], game.Tableau, Sol.DealMode.IfNotEmpty)
end


------

------------------------

Sol.register_game(spidike)
Sol.register_game(incompatibility)
