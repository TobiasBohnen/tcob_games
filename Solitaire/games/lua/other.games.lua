-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local layout = require 'base/layout'
local ops    = require 'base/ops'
local piles  = require 'base/piles'
require 'base/common'

local aces_up = {
    Info        = {
        Name          = "Aces Up",
        Type          = "ClosedNonBuilder",
        Family        = "Other",
        DeckCount     = 1,
        CardDealCount = 4,
        Redeals       = 0
    },
    Stock       = {
        Position = { x = 0, y = 0 },
        Initial = piles.initial.face_down(48)
    },
    Foundation  = {
        Position = { x = 1, y = 0 },
        Rule = { Move = "None" }
    },
    Tableau     = {
        Size   = 4,
        create = function(i)
            return {
                Position = { x = i, y = 1 },
                Initial  = piles.initial.face_up(1),
                Layout   = "Column",
                Rule     = { Build = "NoBuilding", Move = "Top", Empty = "Any" }
            }
        end
    },
    can_drop    = function(game, targetPile, _, drop, numCards)
        if numCards ~= 1 then return false end

        if targetPile.Type == "Foundation" then
            if drop.Rank == "Ace" then return false end
            local dropRank = RankValues[drop.Rank]
            for _, tab in ipairs(game.Tableau) do
                if not tab.Empty then
                    local tabCard = tab.Cards[tab.CardCount]
                    if tabCard.Suit == drop.Suit and (tabCard.Rank == "Ace" or RankValues[tabCard.Rank] > dropRank) then
                        return true
                    end
                end
            end
        elseif targetPile.Type == "Tableau" then
            return targetPile.Empty
        end

        return false
    end,
    check_state = function(game)
        if game.Foundation[1].CardCount == 48 then return "Success" end

        if game.Stock[1].Empty then
            local suits = {}
            for _, tab in ipairs(game.Tableau) do
                if tab.Empty or suits[tab.Cards[tab.CardCount].Suit] ~= nil then return "Running" end
                suits[tab.Cards[tab.CardCount].Suit] = true
            end
            return "Failure"
        end

        return "Running"
    end,
    deal        = function(game) return game.Stock[1]:deal_to_group(game.Tableau, false) end
}

------
local four_seasons_fou_pos = {
    { 0, 1 }, { 0, 3 }, { 4, 1 }, { 4, 3 }
}
local four_seasons_tab_pos = {
    { 2, 1 }, { 1, 2 }, { 2, 2 }, { 3, 2 }, { 2, 3 }
}

local four_seasons = {
    Info       = {
        Name          = "Four Seasons",
        Type          = "SimplePacker",
        Family        = "Other",
        DeckCount     = 1,
        CardDealCount = 1,
        Redeals       = 0
    },
    Stock      = {
        Position = { x = 0, y = 0 },
        Initial = piles.initial.face_down(46)
    },
    Waste      = { Position = { x = 1, y = 0 } },
    Foundation = {
        Size   = 4,
        create = function(i)
            return {
                Position = { x = four_seasons_fou_pos[i + 1][1], y = four_seasons_fou_pos[i + 1][2] },
                Initial  = piles.initial.face_up(i == 0 and 1 or 0),
                Rule     = { Build = "UpInSuit", Wrap = true, Move = "None", Empty = "FirstFoundation" }
            }
        end
    },
    Tableau    = {
        Size   = 5,
        create = function(i)
            return {
                Position = { x = four_seasons_tab_pos[i + 1][1], y = four_seasons_tab_pos[i + 1][2] },
                Initial  = piles.initial.face_up(1),
                Layout   = "Squared",
                Rule     = { Build = "DownByRank", Wrap = true, Move = "Top", Empty = "Any" }
            }
        end
    },
    can_drop   = function(game, targetPile, _, drop, numCards)
        if numCards ~= 1 then return false end

        if targetPile.Type == "Foundation" then
            if drop.Rank == "Ace" then return false end
            local dropRank = RankValues[drop.Rank]
            for _, tab in ipairs(game.Tableau) do
                if not tab.Empty then
                    local tabCard = tab.Cards[tab.CardCount]
                    if tabCard.Suit == drop.Suit and (tabCard.Rank == "Ace" or RankValues[tabCard.Rank] > dropRank) then
                        return true
                    end
                end
            end
        elseif targetPile.Type == "Tableau" then
            return targetPile.Empty
        end

        return false
    end,
    deal       = ops.deal.stock_to_waste
}

------------------------

RegisterGame(aces_up)
RegisterGame(four_seasons)
