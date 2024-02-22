-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local layout = require 'base/layout'
local ops    = require 'base/ops'
local piles  = require 'base/piles'
require 'base/common'

local montana_base = {
    redeal = function(game, ranks)
        local columns = #ranks + 1
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
                        game.PlaceTop(table.remove(cards), tab, true)
                    else
                        wasEmpty = true
                    end
                end
            end
        end

        return true
    end,

    can_drop = function(game, targetPile, drop, ranks)
        local columns = #ranks + 1
        if not targetPile.Empty then return false end

        local tableau = game.Tableau
        for i = 1, #tableau do
            if tableau[i] == targetPile then
                if (i - 1) % columns == 0 then return drop.Rank == ranks[1] end --leftmost column

                --check if left card is one rank higher
                local leftTab = tableau[i - 1]
                if leftTab.Empty then return false end
                local leftCard = leftTab.Cards[1]
                if leftCard.Suit == drop.Suit then
                    for k, v in ipairs(ranks) do
                        if k == #ranks then return false end --don't wrap
                        if v == leftCard.Rank then return ranks[k + 1] == drop.Rank end
                    end
                end
            end
        end

        return false
    end,

    check_state = function(game, ranks)
        local columns = #ranks + 1
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
}

------
local montana_ranks = { table.unpack(Ranks, 2, 13) }

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
    shuffle     = function(_, card, _)
        return card.Rank == "Ace"
    end,
    can_drop    = function(game, targetPile, _, drop, _)
        return montana_base.can_drop(game, targetPile, drop, montana_ranks)
    end,
    redeal      = function(game)
        return montana_base.redeal(game, montana_ranks)
    end,
    check_state = function(game)
        return montana_base.check_state(game, montana_ranks)
    end
}

------
local blue_moon_ranks = Ranks

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
    shuffle     = function(game, card, _)
        if card.Rank == "Ace" then
            return game.PlaceTop(card, game.Tableau, 1, 1, true)
                or game.PlaceTop(card, game.Tableau, 15, 1, true)
                or game.PlaceTop(card, game.Tableau, 29, 1, true)
                or game.PlaceTop(card, game.Tableau, 43, 1, true)
        end

        return false
    end,
    can_drop    = function(game, targetPile, _, drop, _)
        return montana_base.can_drop(game, targetPile, drop, blue_moon_ranks)
    end,
    redeal      = function(game)
        return montana_base.redeal(game, blue_moon_ranks)
    end,
    check_state = function(game)
        return montana_base.check_state(game, blue_moon_ranks)
    end
}

------
local paganini_ranks = { "Ace", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King" }

local paganini = {
    Info           = {
        Name          = "Paganini",
        Type          = "OpenNonBuilder",
        Family        = "Montana",
        DeckCount     = 1,
        CardDealCount = 0,
        Redeals       = 1
    },
    Stock          = { Position = { x = 4.5, y = 5 } },
    Tableau        = {
        Size   = 40,
        create = function(i)
            local init = 1
            if i % 10 == 0 then init = 0 end
            return {
                Position = { x = i % 10, y = i // 10 },
                Initial = piles.initial.face_up(init),
                Layout = "Squared",
                Rule = { Build = "NoBuilding", Move = "Top", Empty = "None" }
            }
        end
    },
    before_shuffle = function(_, card)
        local rank = card.Rank
        return rank == "Two" or rank == "Three" or rank == "Four" or rank == "Five"
    end,
    shuffle        = function(game, card, _)
        if card.Rank == "Ace" then
            return game.PlaceTop(card, game.Tableau, 1, 1, true)
                or game.PlaceTop(card, game.Tableau, 11, 1, true)
                or game.PlaceTop(card, game.Tableau, 21, 1, true)
                or game.PlaceTop(card, game.Tableau, 31, 1, true)
        end

        return false
    end,
    can_drop       = function(game, targetPile, _, drop, _)
        return montana_base.can_drop(game, targetPile, drop, paganini_ranks)
    end,
    redeal         = function(game)
        return montana_base.redeal(game, paganini_ranks)
    end,
    check_state    = function(game)
        return montana_base.check_state(game, paganini_ranks)
    end
}

------------------------

RegisterGame(montana)
RegisterGame(blue_moon)
RegisterGame(paganini)
