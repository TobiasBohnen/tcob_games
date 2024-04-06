-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local rules = require 'base/rules'


local chessboard       = {
    Info             = {
        Name          = "Chessboard",
        Family        = "BeleagueredCastle",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 0
    },
    Foundation       = {
        Size = 4,
        Pile = { Rule = rules.ff_upsuit_none_l13 }
    },
    Tableau          = {
        Size = 10,
        Pile = function(i)
            return {
                Initial = ops.Initial.face_up(i < 2 and 6 or 5),
                Layout = "Column",
                Rule = { Base = rules.Base.Any(), Build = rules.Build.UpOrDownInSuit(true), Move = rules.Move.Top() }
            }
        end
    },
    check_playable   = function(game, targetPile, targetCardIndex, card, numCards)
        local foundation1 = game.Foundation[1]
        if foundation1.IsEmpty and targetPile == foundation1 then -- allow any card on first foundation
            return true
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end,
    on_piles_created = Sol.Layout.canister
}

------

local lasker           = Sol.copy(chessboard)
lasker.Info.Name       = "Lasker"
lasker.Foundation.Pile = { Rule = rules.ff_upsuit_top_l13 }
lasker.Tableau.Pile    = function(i)
    return {
        Initial = ops.Initial.face_up(i < 2 and 6 or 5),
        Layout = "Column",
        Rule = { Base = rules.Base.Any(), Build = rules.Build.UpOrDownInSuit(true), Move = rules.Move.InSeq() }
    }
end

------

local morphy           = Sol.copy(chessboard)
morphy.Info.Name       = "Morphy"
morphy.Tableau.Pile    = function(i)
    return {
        Initial = ops.Initial.face_up(i < 2 and 6 or 5),
        Layout = "Column",
        Rule = { Base = rules.Base.Any(), Build = rules.Build.UpOrDownAlternateColors(true), Move = rules.Move.Top() }
    }
end

------

------------

Sol.register_game(chessboard)
Sol.register_game(lasker)
Sol.register_game(morphy)
