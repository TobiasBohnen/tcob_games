-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local setup = {
    -- before setup
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
    ace_and_king_to_foundation = function(game, card)
        local foundation = game.Foundation
        if card.Rank == "Ace" then
            for i = 1, 4 do
                if not foundation[i].IsEmpty and foundation[i].Cards[1].Suit == card.Suit then return false end
            end
            return game.PlaceTop(card, foundation, 1, 4, true)
        elseif card.Rank == "King" then
            for i = 5, 8 do
                if not foundation[i].IsEmpty and foundation[i].Cards[1].Suit == card.Suit then return false end
            end
            return game.PlaceTop(card, foundation, 5, 4, true)
        end

        return false
    end,
    -- setup
    play_to_foundation = function(game, card, pile)
        if pile.Type ~= Sol.Pile.Type.Tableau then return false end

        local foundation = game.Foundation
        for _, v in ipairs(foundation) do
            if game:play_card(card, v) then return true end
        end

        return false
    end,
    -- after setup
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
    to_pile        = redeal_func,
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

local function deal_group_func(fromPile, toGroup, mode)
    mode = mode or Sol.DealMode.Always
    local ret = false

    if fromPile.IsEmpty then return false end

    for _, toPile in ipairs(toGroup) do
        if fromPile.IsEmpty then break end

        local check = true
        if mode == Sol.DealMode.IfEmpty then
            check = toPile.IsEmpty
        elseif mode == Sol.DealMode.IfNotEmpty then
            check = not toPile.IsEmpty
        end

        if check then
            fromPile:move_cards(toPile, #fromPile.Cards, 1, false)
            toPile:flip_up_top_card()
            ret = true
        end
    end
    return ret
end

local deal = {
    to_pile                         = deal_func,
    to_group                        = deal_group_func,

    stock_to_waste                  = function(game) return deal_func(game.Stock[1], game.Waste[1], 1) end,
    stock_to_waste_by_3             = function(game) return deal_func(game.Stock[1], game.Waste[1], 3) end,
    stock_to_waste_by_redeals_left  = function(game) return deal_func(game.Stock[1], game.Waste[1], game.RedealsLeft + 1) end,
    stock_to_tableau                = function(game) return deal_group_func(game.Stock[1], game.Tableau) end,
    waste_or_stock_to_empty_tableau = function(game)
        return
            deal_group_func(game.Waste[1], game.Tableau, Sol.DealMode.IfEmpty)
            or deal_group_func(game.Stock[1], game.Tableau, Sol.DealMode.IfEmpty)
    end
}

local function get_py_cells(n) return (n * (n + 1)) / 2 end
local function get_py_row(n) return (-1 + math.sqrt((8 * n) + 1)) / 2 end
local pyramid = {
    pile = function(size, base, offset, i)
        local dummyCells <const>     = get_py_cells(base - 1)
        local dummyCellRows <const>  = get_py_row(dummyCells)
        local totalRows <const>      = get_py_row(size + dummyCells)
        local lastRowStart <const>   = get_py_cells(totalRows - 1) - dummyCells
        local lastRowSize <const>    = size - lastRowStart

        local idx <const>            = i + 1
        local currentRow <const>     = math.ceil(get_py_row(dummyCells + idx)) - dummyCellRows
        local currentRowSize <const> = currentRow + base - 1
        local currentColumn <const>  = i - (get_py_cells(currentRow - 1 + dummyCellRows) - dummyCells)

        return {
            HasMarker = idx < base,
            Layout    = Sol.Pile.Layout.Column,
            Initial   = idx <= lastRowStart and Sol.Initial.face_down(1) or Sol.Initial.face_up(1),
            Position  = {
                x = (lastRowSize - currentRowSize) / 2 + currentColumn + offset.x,
                y = (currentRow * 0.5) + offset.y
            }
        }
    end,
    flip = function(size, base, tableau)
        local dummyCells <const> = get_py_cells(base - 1)
        local totalRows <const>  = get_py_row(size + dummyCells)
        local last <const>       = get_py_cells(totalRows - 1) - dummyCells

        for tabIdx = 1, last do
            local pile = tableau[tabIdx]
            if pile.IsEmpty then goto continue end

            local rowSize <const> = math.ceil(get_py_row(dummyCells + tabIdx))
            if tableau[tabIdx + rowSize + 0].IsEmpty and tableau[tabIdx + rowSize + 1].IsEmpty then
                pile:flip_up_top_card()
            end

            ::continue::
        end
    end,
    face_up_pile = function(size, base, offset, i)
        local dummyCells <const> = get_py_cells(base - 1)
        local totalRows <const>  = get_py_row(size + dummyCells)

        local pile               = Sol.Ops.Pyramid.pile(size, base, offset, i)
        --all cards visible
        pile.Initial             = Sol.Initial.face_up(1)
        --only bottom row playable
        local move               = Sol.Rules.Move.Top()

        local last <const>       = get_py_cells(totalRows - 1) - dummyCells
        local rowSize <const>    = math.ceil((-1 + math.sqrt((8 * i) + 1)) / 2)
        move.IsPlayable          = function(game)
            if i >= last then return true end
            local tableau = game.Tableau
            return tableau[i + rowSize + 1].IsEmpty and tableau[i + rowSize + 2].IsEmpty
        end

        pile.Rule                = { Base = Sol.Rules.Base.None(), Build = Sol.Rules.Build.None(), Move = move }

        return pile
    end
}

return {
    Setup   = setup,
    Redeal  = redeal,
    Deal    = deal,
    Pyramid = pyramid
}
