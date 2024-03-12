-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local shuffle = {
    -- before shuffle
    ace_to_foundation = function(game, card)
        if card.Rank == "Ace" then
            return game.PlaceTop(card, game.Foundation, true)
        end

        return false
    end,
    king_to_foundation = function(game, card)
        if card.Rank == "King" then
            return game.PlaceTop(card, game.Foundation, true)
        end

        return false
    end,
    -- after shuffle
    kings_to_bottom = function(game)
        for _, v in ipairs(game.Tableau) do
            v:move_rank_to_bottom("King")
        end
    end
}

local redeal = {
    waste_to_stock = function(game) return game.Waste[1]:redeal(game.Stock[1]) end
}

local deal = {
    stock_to_waste = function(game) return game.Stock[1]:deal(game.Waste[1], game.CardDealCount) end,
    stock_to_waste_by_redeals_left = function(game) return game.Stock[1]:deal(game.Waste[1], game.RedealsLeft + 1) end
}

local initial = {
    top_face_up = function(size)
        local retValue = {}
        for i = 1, size - 1 do
            retValue[i] = false
        end
        retValue[size] = true
        return retValue
    end,
    face_up = function(size)
        local retValue = {}
        for i = 1, size do
            retValue[i] = true
        end
        return retValue
    end,
    face_down = function(size)
        local retValue = {}
        for i = 1, size do
            retValue[i] = false
        end
        return retValue
    end,
    alternate = function(size, first)
        local retValue = {}
        for i = 1, size do
            retValue[i] = first
            first = not first
        end
        return retValue
    end
}

return {
    Initial = initial,
    Shuffle = shuffle,
    Redeal = redeal,
    Deal = deal
}
