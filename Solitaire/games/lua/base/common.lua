-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

function Copy(obj, seen)
    if type(obj) ~= 'table' then return obj end
    if seen and seen[obj] then return seen[obj] end
    local s = seen or {}
    local res = setmetatable({}, getmetatable(obj))
    s[obj] = res
    for k, v in pairs(obj) do res[Copy(k, s)] = Copy(v, s) end
    return res
end

RankValues = {
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

Ranks = { "Ace", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King" }

function GetRank(r, interval, wrap)
    local rank = RankValues[r]
    local target = rank + interval

    if wrap then
        target = ((target - 1) % 13 + 13) % 13 + 1
    end

    if target == 0 or target > 13 then
        return nil
    end

    return Ranks[target]
end
