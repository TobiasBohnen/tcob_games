-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local rules = require 'base/rules'


local snake                 = {
    Info              = {
        Name          = "Snake",
        Family        = "FreeCell",
        DeckCount     = 2,
        CardDealCount = 0,
        Redeals       = 0
    },
    FreeCell          = {
        Size = 7,
        Pile = function(i)
            return {
                Position = { x = i + 1, y = 0 },
                Rule     = rules.any_none_top
            }
        end
    },
    Foundation        = {
        Size = 8,
        Pile = function(i)
            return {
                Position = { x = i % 2 + 9, y = i // 2 },
                Rule = rules.ace_upsuit_top
            }
        end
    },
    Tableau           = {
        Size = 9,
        Pile = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial = ops.Initial.face_up(i == 0 and 96 or 0),
                Layout = "Column",
                Rule = { Base = rules.Base.None(), Build = rules.Build.DownAlternateColors(), Move = rules.Move.SuperMove() }
            }
        end
    },
    on_before_shuffle = ops.Shuffle.ace_to_foundation,
    on_after_shuffle  = function(game)
        -- kings start new pile
        local tableau = game.Tableau
        local tabIdx = 2
        local tableau1 = tableau[1]
        for cardIdx = tableau1.CardCount, 2, -1 do
            local card = tableau1.Cards[cardIdx]
            if card.Rank == "King" then
                tableau1:move_cards(tableau[tabIdx], cardIdx, tableau1.CardCount - cardIdx + 1, false)
                tabIdx = tabIdx + 1
            end
        end
    end
}

------

local cats_tail             = Sol.copy(snake)
cats_tail.Info.Name         = "Cat's Tail"
cats_tail.Tableau.Pile      = function(i)
    return {
        Position = { x = i, y = 1 },
        Initial = ops.Initial.face_up(i == 0 and 104 or 0),
        Layout = "Column",
        Rule = { Base = rules.Base.None(), Build = rules.Build.DownAlternateColors(), Move = rules.Move.InSeq() }
    }
end
cats_tail.on_before_shuffle = nil

------

------------------------

Sol.register_game(snake)
Sol.register_game(cats_tail)
