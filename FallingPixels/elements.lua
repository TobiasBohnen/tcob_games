-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local neighbors = { { -1, -1 }, { -1, 0 }, { -1, 1 }, { 1, -1 }, { 1, 0 }, { 1, 1 }, { 0, -1 }, { 0, 1 } }
local EMPTY = 0

----------------------
local empty = {
    Color   = { "Transparent", 0 },
    Gravity = 0,
    Density = 100,
    Type    = "None",
}
register_element(EMPTY, "Empty", empty)

----------------------
local wall = {
    Color   = { "DarkGray", 0 },
    Gravity = 0,
    Density = 100,
    Type    = "Solid",
}
register_element(1, "Wall", wall)

----------------------
local sand = {
    Color   = { "SandyBrown", 8 },
    Density = 2.5,
    Type    = "Powder",
}
register_element(2, "Sand", sand)

----------------------
local salt = {
    Color   = { "#F3FAFD", 8 },
    Density = 2.19,
    Type    = "Powder",
    Update  = function(system, pos)
        for _, n in ipairs(neighbors) do
            local neighborPos = { x = pos.x + n[1], y = pos.y + n[2] }
            if system:name(neighborPos) == "Water" then
                system:set(pos, EMPTY)
                break
            end
        end
    end
}
register_element(3, "Salt", salt)

----------------------
local sawdust = {
    Color = { "#F9EAD4", 16 },
    Density = 0.21,
    Flammable = true,
    Type = "Powder",
}
register_element(4, "Sawdust", sawdust)

----------------------
local water = {
    Color   = { "Blue", 16 },
    Density = 1,
    Type    = "Liquid",
}
register_element(5, "Water", water)

----------------------
local olive_oil = {
    Color   = { "Olive", 8 },
    Density = 0.9,
    Type    = "Liquid",
}
register_element(6, "Olive Oil", olive_oil)

----------------------
local honey = {
    Color   = { "#F9C901", 8 },
    Density = 1.6,
    Type    = "Liquid",
}
register_element(7, "Honey", honey)

----------------------
local wood = {
    Color     = { "#BA8C63", 8 },
    Gravity   = 0,
    Density   = 0.85,
    Flammable = true,
    Type      = "Solid",
}
register_element(8, "Wood", wood)

----------------------
local fire = {
    Color   = { "Red", 8 },
    Gravity = 0,
    Density = 100,
    Type    = "Gas",
    Update  = function(system, pos)
        local check = false
        for _, n in ipairs(neighbors) do
            local neighborPos = { x = pos.x + n[1], y = pos.y + n[2] }
            if system:flammable(neighborPos) then
                check = true
                system:set(neighborPos, 9)
            end
        end
        if not check then
            system:set(pos, 10) -- turn into ash without flammable neighbors
        end
    end
}
register_element(9, "Fire", fire)

----------------------
local ash = {
    Color   = { "#B2BEB5", 36 },
    Gravity = 1,
    Density = 2.45,
    Type    = "Powder",
}
register_element(10, "Ash", ash)

----------------------
