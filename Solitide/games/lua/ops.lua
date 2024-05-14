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
    -- shuffle
    play_to_foundation = function(game, card, pile)
        if pile.Type ~= "Tableau" then return false end

        local foundation = game.Foundation
        for _, v in ipairs(foundation) do
            if game:play_card(card, v) then return true end
        end

        return false
    end,
    -- after shuffle
    kings_to_bottom = function(game)
        for _, v in ipairs(game.Tableau) do
            v:shift_rank_to_bottom("King")
        end
    end
}

local function redeal_func(fromPile, toPile)
    if toPile.IsEmpty and not fromPile.IsEmpty then
        fromPile:move_cards(toPile, 1, #fromPile.Cards, true)
        toPile:flip_down_cards()
        return true
    end

    return false
end

local redeal = {
    to_pile = redeal_func,
    waste_to_stock = function(game) return redeal_func(game.Waste[1], game.Stock[1]) end
}

local function deal_func(fromPile, toPile, count)
    if fromPile.IsEmpty then return false end
    for _ = 1, count do
        fromPile:move_cards(toPile, #fromPile.Cards, 1, false)
    end
    toPile:flip_up_cards()
    return true
end

local function deal_group_func(fromPile, toGroup, emptyMode)
    emptyMode = emptyMode or "Always"

    if fromPile.IsEmpty then return false end
    for _, toPile in ipairs(toGroup) do
        if fromPile.IsEmpty then break end
        local check = true
        if emptyMode == "IfEmpty" then
            check = toPile.IsEmpty
        elseif emptyMode == "IfNotEmpty" then
            check = not toPile.IsEmpty
        end

        if check then
            fromPile:move_cards(toPile, #fromPile.Cards, 1, false)
            toPile:flip_up_top_card()
        end
    end
    return true
end

local deal = {
    to_pile = deal_func,
    to_group = deal_group_func,

    stock_to_waste = function(game) return deal_func(game.Stock[1], game.Waste[1], 1) end,
    stock_to_waste_by_3 = function(game) return deal_func(game.Stock[1], game.Waste[1], 3) end,
    stock_to_waste_by_redeals_left = function(game) return deal_func(game.Stock[1], game.Waste[1], game.RedealsLeft + 1) end,
    stock_to_tableau = function(game) return deal_group_func(game.Stock[1], game.Tableau) end,
    waste_or_stock_to_empty_tableau = function(game) return deal_group_func(game.Waste[1], game.Tableau, "IfEmpty") or deal_group_func(game.Stock[1], game.Tableau, "IfEmpty") end
}

return {
    Shuffle = shuffle,
    Redeal = redeal,
    Deal = deal
}
