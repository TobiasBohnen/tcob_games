-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

Sol.Layout = require "layout"
Sol.Rules = require "rules"
Sol.Ops = require "ops"

function Sol.copy(obj, seen)
    if type(obj) ~= 'table' then return obj end
    if seen and seen[obj] then return seen[obj] end
    local s = seen or {}
    local res = setmetatable({}, getmetatable(obj))
    s[obj] = res
    for k, v in pairs(obj) do res[Sol.copy(k, s)] = Sol.copy(v, s) end
    return res
end

Sol.RankValues = {
    Ace = 1,
    Two = 2,
    Three = 3,
    Four = 4,
    Five = 5,
    Six = 6,
    Seven = 7,
    Eight = 8,
    Nine = 9,
    Ten = 10,
    Jack = 11,
    Queen = 12,
    King = 13
}

Sol.Ranks = { "Ace", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King" }
Sol.SuitColors = { Hearts = "Red", Diamonds = "Red", Spades = "Black", Clubs = "Black" }

function Sol.get_rank(r, interval, wrap)
    local target = Sol.RankValues[r] + interval

    if wrap then
        target = ((target - 1) % 13 + 13) % 13 + 1
    end

    if target == 0 or target > 13 then
        return nil
    end

    return Sol.Ranks[target]
end

function Sol.shuffle_piles(game, pileGroups)
    local cards = {}

    for _, piles in ipairs(pileGroups) do
        for _, pile in ipairs(piles) do
            local pileCards = pile.Cards
            for j = 1, #pileCards do
                cards[#cards + 1] = pileCards[j]
            end
            pile:clear()
        end
    end

    return game:shuffle_cards(cards)
end

Sol.Initial = {
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
