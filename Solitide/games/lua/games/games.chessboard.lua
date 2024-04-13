-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local chessboard       = {
    Info           = {
        Name      = "Chessboard",
        Family    = "BeleagueredCastle",
        DeckCount = 1,
        Redeals   = 0
    },
    Foundation     = {
        Size = 4,
        Pile = { Rule = Sol.Rules.ff_upsuit_none_l13 }
    },
    Tableau        = {
        Size = 10,
        Pile = function(i)
            return {
                Initial = Sol.Initial.face_up(i < 2 and 6 or 5),
                Layout = "Column",
                Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.UpOrDownInSuit(true), Move = Sol.Rules.Move.Top() }
            }
        end
    },
    check_playable = function(game, targetPile, targetCardIndex, card, numCards)
        local foundation1 = game.Foundation[1]
        if foundation1.IsEmpty and targetPile == foundation1 then -- allow any card on first foundation
            return true
        end

        return game:can_play(targetPile, targetCardIndex, card, numCards)
    end,
    on_init        = Sol.Layout.canister
}

------

local lasker           = Sol.copy(chessboard)
lasker.Info.Name       = "Lasker"
lasker.Foundation.Pile = { Rule = Sol.Rules.ff_upsuit_top_l13 }
lasker.Tableau.Pile    = function(i)
    return {
        Initial = Sol.Initial.face_up(i < 2 and 6 or 5),
        Layout = "Column",
        Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.UpOrDownInSuit(true), Move = Sol.Rules.Move.InSeq() }
    }
end

------

local morphy           = Sol.copy(chessboard)
morphy.Info.Name       = "Morphy"
morphy.Tableau.Pile    = function(i)
    return {
        Initial = Sol.Initial.face_up(i < 2 and 6 or 5),
        Layout = "Column",
        Rule = { Base = Sol.Rules.Base.Any(), Build = Sol.Rules.Build.UpOrDownAlternateColors(true), Move = Sol.Rules.Move.Top() }
    }
end

------

------------

Sol.register_game(chessboard)
Sol.register_game(lasker)
Sol.register_game(morphy)
