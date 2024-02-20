-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local layout = require 'base/layout'
local ops    = require 'base/ops'
local piles  = require 'base/piles'
require 'base/common'

local function montana_redeal(game, ranks, columns)
    local tableau = game.Tableau
    local cards = {}

    for y = 1, 4 do
        local inOrder = true
        local suit
        for x = 1, columns do
            local tab = tableau[x + (y - 1) * columns]
            if not tab.Empty then
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
            if tab.Empty then
                if wasEmpty then
                    game.PutBack(table.remove(cards), tab, true)
                else
                    wasEmpty = true
                end
            end
        end
    end

    return true
end

local function montana_can_drop(game, targetPile, drop, columns)
    if targetPile.Empty then
        local tableau = game.Tableau
        for i = 1, #tableau do
            if tableau[i] == targetPile then
                if (i - 1) % columns == 0 then --leftmost column
                    return drop.Rank == "Two"
                end
                local leftTab = tableau[i - 1]
                if not leftTab.Empty then --check if left card is one rank higher
                    local leftCard = leftTab.Cards[1]
                    if leftCard.Suit == drop.Suit then
                        return game.GetRank(leftCard.Rank, 1, false) == drop.Rank
                    end
                end

                return false
            end
        end
        return false
    end
    return false
end

local function montana_check_state(game, ranks, columns)
    local tableau = game.Tableau

    for y = 1, 4 do
        local firstTab = tableau[1 + (y - 1) * columns]
        if firstTab.Empty or firstTab.Cards[1].Rank ~= ranks[1] then return "Running" end
        local suit = firstTab.Cards[1].Suit

        for x = 2, #ranks do
            local tab = tableau[x + (y - 1) * columns]
            if tab.Empty then return "Running" end
            local card = tab.Cards[1]
            if card.Rank ~= ranks[x] or card.Suit ~= suit then return "Running" end
        end
    end

    return "Success"
end

------

local montana = {
    Info        = {
        Name          = "Montana",
        Type          = "OpenNonBuilder",
        Family        = "Montana",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 2
    },
    Stock       = { Position = { x = 6, y = 5 } },
    Tableau     = {
        Size   = 52,
        create = function(i)
            return {
                Position = { x = i % 13, y = i // 13 },
                Initial = piles.initial.face_up(1),
                Layout = "Squared",
                Rule = { Build = "NoBuilding", Move = "Top", Empty = "None" }
            }
        end
    },
    can_drop    = function(game, targetPile, _, drop, _)
        return montana_can_drop(game, targetPile, drop, 13)
    end,
    shuffle     = function(_, card, _)
        return card.Rank == "Ace"
    end,
    redeal      = function(game)
        local ranks = { "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King" }
        return montana_redeal(game, ranks, 13)
    end,
    check_state = function(game)
        local ranks = { "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King" }
        return montana_check_state(game, ranks, 13)
    end
}

------

local blue_moon = {
    Info        = {
        Name          = "Blue Moon",
        Type          = "OpenNonBuilder",
        Family        = "Montana",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 2
    },
    Stock       = { Position = { x = 6, y = 5 } },
    Tableau     = {
        Size   = 56,
        create = function(i)
            local init = 1
            if i % 14 == 0 then init = 0 end
            return {
                Position = { x = i % 14, y = i // 14 },
                Initial = piles.initial.face_up(init),
                Layout = "Squared",
                Rule = { Build = "NoBuilding", Move = "Top", Empty = "None" }
            }
        end
    },
    can_drop    = function(game, targetPile, _, drop, _)
        return montana_can_drop(game, targetPile, drop, 14)
    end,
    shuffle     = function(game, card, _)
        if card.Rank == "Ace" then
            return game.PutBack(card, game.Tableau, 1, 1, true)
                or game.PutBack(card, game.Tableau, 15, 1, true)
                or game.PutBack(card, game.Tableau, 29, 1, true)
                or game.PutBack(card, game.Tableau, 43, 1, true)
        end

        return false
    end,
    redeal      = function(game)
        local ranks = { "Ace", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King" }
        return montana_redeal(game, ranks, 14)
    end,
    check_state = function(game)
        local ranks = { "Ace", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King" }
        return montana_check_state(game, ranks, 14)
    end
}


------------------------

register_game(montana)
register_game(blue_moon)
