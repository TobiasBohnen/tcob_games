-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ops   = require 'base/ops'
local rules = require 'base/rules'


local spider             = {
    Info       = {
        Name          = "Spider",
        Type          = "Spider",
        Family        = "Spider",
        DeckCount     = 2,
        CardDealCount = 10,
        Redeals       = 0
    },
    Stock      = {
        Position = { x = 0, y = 0 },
        Initial = ops.Initial.face_down(50)
    },
    Foundation = {
        Size   = 4,
        Create = function(i)
            return {
                Position = { x = i + 2, y = 0 },
                Rule = rules.none_none_none
            }
        end
    },
    Tableau    = {
        Size   = 10,
        Create = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial  = ops.Initial.top_face_up(i < 4 and 6 or 5),
                Layout   = "Column",
                Rule     = { Base = rules.Base.Any, Build = rules.Build.DownByRank, Move = rules.Move.InSeqInSuit }
            }
        end
    },
    on_change  = function(game)
        for _, tableau in ipairs(game.Tableau) do
            if tableau.CardCount >= 13 then
                -- look for completed stack
                local targetSuit = tableau.Cards[tableau.CardCount].Suit
                local found = true
                for i = 0, 12 do
                    local card = tableau.Cards[tableau.CardCount - i]
                    if card.IsFaceDown or card.Suit ~= targetSuit or card.Rank ~= Sol.Ranks[i + 1] then
                        found = false
                        break
                    end
                end

                -- move to foundation
                if found then
                    for _, foundation in ipairs(game.Foundation) do
                        if foundation.IsEmpty then
                            tableau:move_cards(foundation, tableau.CardCount - 12, 13, false)
                            tableau:flip_up_top_card()
                            break
                        end
                    end
                end
            end
        end
    end,
    on_deal    = function(game)
        for _, tableau in ipairs(game.Tableau) do
            if tableau.IsEmpty then return false end
        end

        return game.Stock[1]:deal_to_group(game.Tableau, false)
    end
}

------

local arachnida          = Sol.copy(spider)
arachnida.Info.Name      = "Arachnida"
arachnida.Foundation     = {
    Position = { x = 11, y = 0 },
    Rule = rules.none_none_none
}
arachnida.Tableau.Create = function(i)
    return {
        Position = { x = i + 1, y = 0 },
        Initial  = ops.Initial.top_face_up(i < 4 and 6 or 5),
        Layout   = "Column",
        Rule     = { Base = rules.Base.Any, Build = rules.Build.InRankOrDownByRank, Move = rules.Move.InSeqInSuitOrSameRank }
    }
end

------------------------

Sol.register_game(spider)
Sol.register_game(arachnida)
