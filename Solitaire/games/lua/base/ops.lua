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

local function redeal_func(from, to)
    if to.IsEmpty and not from.IsEmpty then
        from:move_cards(to, 1, #from.Cards, true)
        to:flip_down_cards()
        return true
    end

    return false
end

local redeal = {
    to_pile = redeal_func,
    waste_to_stock = function(game) return redeal_func(game.Waste[1], game.Stock[1]) end
}

local function deal_func(from, to, count)
    if from.IsEmpty then return false end
    for _ = 1, count, 1 do
        from:move_cards(to, #from.Cards, 1, false)
    end
    to:flip_up_cards()
    return true
end

local function deal_group_func(from, to, ifEmpty)
    if from.IsEmpty then return false end
    for _, toPile in ipairs(to) do
        if from.IsEmpty then break end
        if ifEmpty == toPile.IsEmpty then
            from:move_cards(toPile, #from.Cards, 1, false)
            toPile:flip_up_top_card()
        end
    end
    return true
end

local deal = {
    to_pile = deal_func,
    to_group = deal_group_func,
    stock_to_waste = function(game) return deal_func(game.Stock[1], game.Waste[1], game.CardDealCount) end,
    stock_to_waste_by_redeals_left = function(game) return deal_func(game.Stock[1], game.Waste[1], game.RedealsLeft + 1) end
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
