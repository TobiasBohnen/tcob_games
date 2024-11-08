-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local neighbors = { { -1, -1 }, { -1, 0 }, { -1, 1 }, { 1, -1 }, { 1, 0 }, { 1, 1 }, { 0, -1 }, { 0, 1 } }
local EMPTY = 0

local function process_neighbors(x, y, pred, func)
    for _, n in ipairs(neighbors) do
        local np = { x = x + n[1], y = y + n[2] }
        if pred(np.x, np.y) then
            if not func(np.x, np.y) then return end
        end
    end
end

----------------------
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
    Update  = function(x, y)
        process_neighbors(x, y,
            function(nx, ny) return system:name(nx, ny) == "Water" end,
            function()
                system:set_id(x, y, EMPTY)
                return false
            end
        )
        return true
    end
}
register_element(3, "Salt", salt)

----------------------
local sawdust = {
    Color   = { "#F9EAD4", 16 },
    Density = 0.21,
    Type    = "Powder",
    Update  = function(x, y)
        if system:temperature(x, y) >= 80 then
            process_neighbors(x, y, function(nx, ny) return system:empty(nx, ny) end,
                function()
                    system:set_id(x, y, 13) --> turn to charcoal
                    return false
                end)
            return false
        end
        return true
    end
}
register_element(4, "Sawdust", sawdust)

----------------------
local water = {
    Color     = { "Blue", 16 },
    Density   = 1,
    Type      = "Liquid",
    SpawnHeat = 10,
    Update    = function(x, y)
        local temp = system:temperature(x, y)
        if temp >= 100 then
            system:set_id(x, y, 11) --> turn to steam
        elseif temp <= 0 then
            system:set_id(x, y, 12) --> turn to ice
        end
        return true
    end
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
    Color   = { "#BA8C63", 8 },
    Gravity = 0,
    Density = 0.85,
    Type    = "Solid",
    Update  = function(x, y)
        if system:temperature(x, y) >= 100 then
            --[[             process_neighbors(x, y, function(nx, ny) return system:empty(nx, ny) end,
                function()
                    system:set_id(x, y, 9)
                    return false
                end) ]]
            system:set_id(x, y, 9) --> turn to charcoal
            return false
        end
        return true
    end
}
register_element(8, "Wood", wood)

----------------------

local charcoal = {
    Color     = { "#c6371c", 32 },
    Gravity   = 0,
    Density   = 1.4,
    Type      = "Solid",
    SpawnHeat = 1200,
    Update    = function(x, y)
        if system:temperature(x, y) < 80 then
            system:set_id(x, y, 10) --> turn to ash
        end
        return true
    end
}
register_element(9, "Charcoal", charcoal)

----------------------
local ash = {
    Color   = { "#B2BEB5", 36 },
    Gravity = 1,
    Density = 2.45,
    Type    = "Powder",
}
register_element(10, "Ash", ash)

----------------------
local steam = {
    Color     = { "#DDD", 12 },
    Gravity   = 1,
    Density   = 0.001,
    Type      = "Gas",
    SpawnHeat = 100,
    Update    = function(x, y)
        if system:temperature(x, y) <= 10 then
            system:set_id(x, y, 5) --> turn to water
        end
        return true
    end
}
register_element(11, "Steam", steam)

----------------------
local ice = {
    Color     = { "#b9e8ea", 8 },
    Gravity   = 0,
    Density   = 0.91,
    Type      = "Solid",
    SpawnHeat = -200,
    Update    = function(x, y)
        if system:temperature(x, y) >= 0 then
            system:set_id(x, y, 5) --> turn to water
        end
        return true
    end
}
register_element(12, "Ice", ice)

----------------------

local charcoal_dust = {
    Color     = { "#c6371c", 32 },
    Gravity   = 1,
    Density   = 1.4,
    Type      = "Powder",
    SpawnHeat = 1200,
    Update    = function(x, y)
        if system:temperature(x, y) < 80 then
            system:set_id(x, y, 10) --> turn to ash
        end
        return true
    end
}
register_element(13, "Charcoal Dust", charcoal_dust)

----------------------
