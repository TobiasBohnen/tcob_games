-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local layout = require 'base/layout'
local ops    = require 'base/ops'
local piles  = require 'base/piles'
require 'base/common'

local montana_base    = {
    redeal = function(game, ranks)
        local columns = #ranks + 1
        local tableau = game.Tableau
        local cards = {}

        for y = 1, 4 do
            local inOrder = true
            local suit
            for x = 1, columns do
                local tab = tableau[x + (y - 1) * columns]
                if not tab.IsEmpty then
                    if x == 1 then
                        suit = tab.Cards[1].Suit
                    elseif inOrder then
                        inOrder = tab.Cards[1].Suit == suit
                    end

                    inOrder = inOrder and (tab.Cards[1].Rank == ranks[x])
                    if not inOrder then
                        cards[#cards + 1] = tab.Cards[1]
                        tab:clear()
                    end
                end
            end
        end
        cards = game:shuffle_cards(cards)

        for y = 1, 4 do
            local wasEmpty = false
            for x = 1, columns do
                local tab = tableau[x + (y - 1) * columns]
                if tab.IsEmpty then
                    if wasEmpty then
                        game.PlaceTop(table.remove(cards), tab, true)
                    else
                        wasEmpty = true
                    end
                end
            end
        end

        return true
    end,

    check_drop = function(game, targetPile, drop, ranks, mode)
        local tableau = game.Tableau

        local check_left = function(leftTab)
            --check if left card is one rank higher
            if leftTab.IsEmpty then return false end
            local leftCard = leftTab.Cards[1]
            if leftCard.Suit == drop.Suit then
                for k, v in ipairs(ranks) do
                    if k == #ranks then return false end --don't wrap
                    if v == leftCard.Rank then return ranks[k + 1] == drop.Rank end
                end
            end
        end
        local check_right = function(rightTab)
            --check if right card is one rank lower
            if rightTab.IsEmpty then return false end
            local rightCard = rightTab.Cards[1]
            if rightCard.Suit == drop.Suit then
                for k, v in ipairs(ranks) do
                    if k == #ranks then return false end --don't wrap
                    if v == rightCard.Rank then return ranks[k - 1] == drop.Rank end
                end
            end
        end

        local columns = #ranks + 1
        if not targetPile.IsEmpty then return false end

        for i, tab in ipairs(tableau) do
            if tab == targetPile then
                if i % columns == 1 then return drop.Rank == ranks[1] end --leftmost column

                local result = false

                if string.find(mode, "l") then
                    result = check_left(tableau[i - 1])
                end
                if result then return true end

                if string.find(mode, "r") and i % columns ~= 0 then
                    result = check_right(tableau[i + 1])
                end
                if result then return true end
            end
        end

        return false
    end,

    check_state = function(game, ranks)
        local columns = #ranks + 1
        local tableau = game.Tableau

        for y = 1, 4 do
            local firstTab = tableau[1 + (y - 1) * columns]
            if firstTab.IsEmpty or firstTab.Cards[1].Rank ~= ranks[1] then return "Running" end
            local suit = firstTab.Cards[1].Suit

            for x = 2, #ranks do
                local tab = tableau[x + (y - 1) * columns]
                if tab.IsEmpty then return "Running" end
                local card = tab.Cards[1]
                if card.Rank ~= ranks[x] or card.Suit ~= suit then return "Running" end
            end
        end

        return "Success"
    end
}

------
local montana_ranks   = { table.unpack(Ranks, 2, 13) }

local montana         = {
    Info        = {
        Name          = "Montana",
        Type          = "OpenNonBuilder",
        Family        = "Montana",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 2
    },
    Stock       = {},
    Tableau     = {
        Size   = 52,
        create = {
            Position = {},
            Initial = piles.initial.face_up(1),
            Layout = "Squared",
            Rule = { Build = "NoBuilding", Move = "Top", Empty = "None" }
        }
    },
    on_shuffle  = function(_, card, _)
        return card.Rank == "Ace"
    end,
    on_created  = function(game) layout.montana(game, 13) end,
    on_redeal   = function(game)
        return montana_base.redeal(game, montana_ranks)
    end,
    check_state = function(game)
        return montana_base.check_state(game, montana_ranks)
    end,
    check_drop  = function(game, targetPile, _, drop, _)
        return montana_base.check_drop(game, targetPile, drop, montana_ranks, "l")
    end
}

------

local moonlight       = Copy(montana)
moonlight.Info.Name   = "Moonlight"
moonlight.check_drop  = function(game, targetPile, _, drop, _)
    return montana_base.check_drop(game, targetPile, drop, montana_ranks, "rl")
end

------
local blue_moon_ranks = Ranks

local blue_moon       = {
    Info        = {
        Name          = "Blue Moon",
        Type          = "OpenNonBuilder",
        Family        = "Montana",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 2
    },
    Stock       = {},
    Tableau     = {
        Size   = 56,
        create = function(i)
            return {
                Initial = piles.initial.face_up(i % 14 == 0 and 0 or 1),
                Layout = "Squared",
                Rule = { Build = "NoBuilding", Move = "Top", Empty = "None" }
            }
        end
    },
    on_shuffle  = function(game, card, _)
        if card.Rank == "Ace" then
            return game.PlaceTop(card, game.Tableau, 1, 1, true)
                or game.PlaceTop(card, game.Tableau, 15, 1, true)
                or game.PlaceTop(card, game.Tableau, 29, 1, true)
                or game.PlaceTop(card, game.Tableau, 43, 1, true)
        end

        return false
    end,
    check_drop  = function(game, targetPile, _, drop, _)
        return montana_base.check_drop(game, targetPile, drop, blue_moon_ranks, "l")
    end,
    on_redeal   = function(game)
        return montana_base.redeal(game, blue_moon_ranks)
    end,
    check_state = function(game)
        return montana_base.check_state(game, blue_moon_ranks)
    end,
    on_created  = function(game) layout.montana(game, 14) end
}

------

local galary          = Copy(blue_moon)
galary.Info.Name      = "Galary"
galary.Tableau.create = function(i)
    return {
        Initial = piles.initial.face_up((i % 14 == 0 or i % 14 == 1) and 0 or 1),
        Layout = "Squared",
        Rule = { Build = "NoBuilding", Move = "Top", Empty = "None" }
    }
end
galary.on_before_shuffle = function(game, card)
    return blue_moon.on_shuffle(game, card)
end
galary.on_shuffle     = nil
galary.check_drop     = function(game, targetPile, _, drop, _)
    return montana_base.check_drop(game, targetPile, drop, blue_moon_ranks, "rl")
end

------
local paganini_ranks  = { "Ace", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King" }

local paganini        = {
    Info           = {
        Name          = "Paganini",
        Type          = "OpenNonBuilder",
        Family        = "Montana",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 1
    },
    Stock          = {},
    Tableau        = {
        Size   = 40,
        create = function(i)
            return {
                Initial = piles.initial.face_up(i % 10 == 0 and 0 or 1),
                Layout = "Squared",
                Rule = { Build = "NoBuilding", Move = "Top", Empty = "None" }
            }
        end
    },
    on_before_shuffle = function(_, card)
        local rank = card.Rank
        return rank == "Two" or rank == "Three" or rank == "Four" or rank == "Five"
    end,
    on_shuffle     = function(game, card, _)
        if card.Rank == "Ace" then
            return game.PlaceTop(card, game.Tableau, 1, 1, true)
                or game.PlaceTop(card, game.Tableau, 11, 1, true)
                or game.PlaceTop(card, game.Tableau, 21, 1, true)
                or game.PlaceTop(card, game.Tableau, 31, 1, true)
        end

        return false
    end,
    check_drop     = function(game, targetPile, _, drop, _)
        return montana_base.check_drop(game, targetPile, drop, paganini_ranks, "l")
    end,
    on_redeal      = function(game)
        return montana_base.redeal(game, paganini_ranks)
    end,
    check_state    = function(game)
        return montana_base.check_state(game, paganini_ranks)
    end,
    on_created     = function(game) layout.montana(game, 10) end
}

------

local spoilt          = {
    Info           = {
        Name          = "Spoilt",
        Type          = "OpenNonBuilder",
        Family        = "Montana",
        DeckCount     = 1,
        CardDealCount = 1,
        Redeals       = 0
    },
    Stock          = {
        Position = { x = 0, y = 1 },
        Initial = piles.initial.face_down(3)
    },
    Waste          = {
        Position = { x = 0, y = 2 },
        Initial = piles.initial.face_up(1)
    },
    Tableau        = {
        Size   = 32,
        create = function(i)
            return {
                Position = { x = i % 8 + 1, y = i // 8 },
                Initial = piles.initial.face_down(i % 8 == 0 and 0 or 1),
                Layout = "Squared",
                Rule = { Build = "NoBuilding", Move = "None", Empty = "None" }
            }
        end
    },
    on_before_shuffle = function(_, card)
        local rank = card.Rank
        return rank == "Two" or rank == "Three" or rank == "Four" or rank == "Five" or rank == "Six"
    end,
    on_change      = function(game)
        local sevenCount = 0
        for i, tableau in ipairs(game.Tableau) do
            if tableau.CardCount == 2 then
                tableau:move_cards(game.Waste[1], 1, 1, false)
                game.Waste[1]:flip_up_top_card()
                return
            elseif (i - 1) % 8 == 0 and not tableau.IsEmpty then
                sevenCount = sevenCount + 1
            end
        end

        if sevenCount == 4 then
            for _, tableau in ipairs(game.Tableau) do
                tableau:flip_up_top_card()
            end
        else
            local stock = game.Stock[1]
            if not stock.IsEmpty and sevenCount > 0 then
                if stock.CardCount > 3 - sevenCount then
                    ops.deal.stock_to_waste(game)
                    return
                end
            end
        end
    end,
    check_drop     = function(game, targetPile, _, drop, _)
        local ranks = { "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King", "Ace" }
        local tableau = game.Tableau

        --get suits per row
        local rowEmpty = { true, true, true, true }
        local suitRows = {}
        for y = 1, 4 do
            for x = 1, 8 do
                local tab = tableau[x + (y - 1) * 8]
                if not tab.IsEmpty and tab.Cards[1].IsFaceUp then
                    rowEmpty[y] = false
                    suitRows[tab.Cards[1].Suit] = y
                    break
                end
            end
        end

        --check rank and suit
        for i, tab in ipairs(tableau) do
            if tab == targetPile then
                local x = (i - 1) % 8 + 1
                if drop.Rank == ranks[x] then                                 --rank fits
                    local y = (i - 1) // 8 + 1
                    if suitRows[drop.Suit] == y then return true end          --suit fits row
                    return suitRows[drop.Suit] == nil and rowEmpty[y] == true --suit not used elsewhere, and row is empty
                end
            end
        end

        return false
    end
}

------------------------

RegisterGame(montana)
RegisterGame(blue_moon)
RegisterGame(galary)
RegisterGame(paganini)
RegisterGame(moonlight)
RegisterGame(spoilt)
