-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local penguin = {
    Info              = {
        Name      = "Penguin",
        Family    = "FreeCell",
        DeckCount = 1
    },
    FreeCell          = {
        Size = 7,
        Pile = { Rule = Sol.Rules.any_none_top }
    },
    Foundation        = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ff_upsuit_none_l13 }
    },
    Tableau           = {
        Size = 7,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i > 0 and 7 or 6),
                Layout  = "Column",
                Rule    = { Base = Sol.Rules.Base.FirstFoundation(-1), Build = Sol.Rules.Build.DownInSuit(true), Move = Sol.Rules.Move.InSeq() }
            }
        end
    },
    on_before_shuffle = function(game, card)
        local tableau1 = game.Tableau[1]
        if tableau1.IsEmpty then
            return game.PlaceTop(card, game.Tableau, true)
        else
            if card.Rank == tableau1.Cards[1].Rank then
                return game.PlaceTop(card, game.Foundation, true)
            end
        end
        return false
    end,
    on_init           = Sol.Layout.flipper
}


------

local opus         = Sol.copy(penguin)
opus.Info.Name     = "Opus"
opus.FreeCell.Size = 5


------

local tuxedo             = Sol.copy(penguin)
tuxedo.Info.Name         = "Tuxedo"
tuxedo.Foundation.Pile   = { Rule = Sol.Rules.ace_upsuit_none }
tuxedo.Tableau.Pile      = function(i)
    return {
        Initial = Sol.Initial.face_up(i % 3 == 0 and 8 or 7),
        Layout  = "Column",
        Rule    = Sol.Rules.any_downsuit_inseq
    }
end
tuxedo.on_before_shuffle = nil


------

------------------------

Sol.register_game(penguin)
Sol.register_game(opus)
Sol.register_game(tuxedo)
