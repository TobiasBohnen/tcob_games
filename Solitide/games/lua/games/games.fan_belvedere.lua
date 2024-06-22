-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local belvedere = {
    Info            = {
        Name      = "Belvedere",
        Family    = "Fan",
        DeckCount = 1
    },
    Stock           = {
        Initial = Sol.Initial.face_down(27)
    },
    Waste           = {
        Size = 3,
        Pile = {
            Layout = Sol.Pile.Layout.Squared,
            Rule   = Sol.Rules.none_none_top
        }
    },
    Foundation      = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ace_uprank_none }
    },
    Tableau         = {
        Size = 8,
        Pile = {
            Initial = Sol.Initial.face_up(3),
            Layout  = Sol.Pile.Layout.Row,
            Rule    = Sol.Rules.none_downrank_top
        }
    },
    on_before_setup = function(game, card)
        if card.Rank == "Ace" then
            return game.PlaceTop(card, game.Foundation[1], true)
        end

        return false
    end,
    on_init         = function(game) Sol.Layout.fan(game, 4) end,
    deal            = function(game) return Sol.Ops.Deal.to_group(game.Stock[1], game.Waste) end
}


------

local bristol           = Sol.copy(belvedere)
bristol.Info.Name       = "Bristol"
bristol.Stock.Initial   = Sol.Initial.face_down(28)
bristol.on_before_setup = nil
bristol.on_after_setup  = Sol.Ops.Setup.kings_to_bottom


------

local vincent                = Sol.copy(bristol)
vincent.Info.Name            = "Vincent"
vincent.Foundation.Pile.Rule = Sol.Rules.ace_upsuit_top
vincent.Tableau.Pile.Rule    = Sol.Rules.king_downsuit_inseq


------

local dover          = Sol.copy(bristol)
dover.Info.Name      = "Dover"
dover.Info.DeckCount = 2
dover.Stock.Initial  = Sol.Initial.face_down(80)


------

------------------------

Sol.register_game(belvedere)
Sol.register_game(bristol)
Sol.register_game(dover)
Sol.register_game(vincent)
