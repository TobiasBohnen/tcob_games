-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

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
    local rank = Sol.RankValues[r]
    local target = rank + interval

    if wrap then
        target = ((target - 1) % 13 + 13) % 13 + 1
    end

    if target == 0 or target > 13 then
        return nil
    end

    return Sol.Ranks[target]
end

function Sol.shuffle_tableau(game)
    local cards = {}

    local tableau = game.Tableau
    for _, tab in ipairs(tableau) do
        for j = 1, #tab.Cards do
            cards[#cards + 1] = tab.Cards[j]
        end
        tab:clear()
    end

    return game:shuffle_cards(cards)
end
