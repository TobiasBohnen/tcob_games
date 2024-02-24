-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local layout = require 'base/layout'
local ops    = require 'base/ops'
local piles  = require 'base/piles'
require 'base/common'

local spider             = {
    Info          = {
        Name          = "Spider",
        Type          = "Spider",
        Family        = "Spider",
        DeckCount     = 2,
        CardDealCount = 10,
        Redeals       = 0
    },
    Stock         = {
        Position = { x = 0, y = 0 },
        Initial = piles.initial.face_down(50)
    },
    Foundation    = {
        Size   = 4,
        create = function(i)
            return {
                Position = { x = i + 2, y = 0 },
                Rule = { Build = "NoBuilding", Move = "None" }
            }
        end
    },
    Tableau       = {
        Size   = 10,
        create = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial  = piles.initial.top_face_up(i < 4 and 6 or 5),
                Layout   = "Column",
                Rule     = { Build = "DownByRank", Move = "InSequenceInSuit", Empty = "Any" }
            }
        end
    },
    before_layout = function(game)
        for _, tableau in ipairs(game.Tableau) do
            if tableau.CardCount >= 13 then
                -- look for completed stack
                local targetSuit = tableau.Cards[tableau.CardCount].Suit
                local found = true
                for i = 0, 12 do
                    local card = tableau.Cards[tableau.CardCount - i]
                    if card.FaceDown or card.Suit ~= targetSuit or card.Rank ~= Ranks[i + 1] then
                        found = false
                        break
                    end
                end

                -- move to foundation
                if found then
                    for _, foundation in ipairs(game.Foundation) do
                        if foundation.Empty then
                            tableau:move_cards(foundation, tableau.CardCount - 12, 13, false)
                            tableau:flip_up_top_card()
                            break
                        end
                    end
                end
            end
        end
    end,
    deal          = function(game)
        for _, tableau in ipairs(game.Tableau) do
            if tableau.Empty then return false end
        end

        return game.Stock[1]:deal_to_group(game.Tableau, false)
    end
}

------
local arachnida          = Copy(spider)
arachnida.Info.Name      = "Arachnida"
arachnida.Foundation     = {
    Position = { x = 11, y = 0 },
    Rule = { Build = "NoBuilding", Move = "None" }
}
arachnida.Tableau.create = function(i)
    return {
        Position = { x = i + 1, y = 0 },
        Initial  = piles.initial.top_face_up(i < 4 and 6 or 5),
        Layout   = "Column",
        Rule     = { Build = "InRankOrDownByRank", Move = "InSequenceInSuitOrSameRank", Empty = "Any" }
    }
end

------------------------

RegisterGame(spider)
RegisterGame(arachnida)
