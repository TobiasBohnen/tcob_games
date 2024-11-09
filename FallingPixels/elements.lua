-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

----------------------
----------------------
local empty = {
    Colors  = { "Transparent" },
    Gravity = 0,
    Density = 100,
    Type    = "None",
}
register_element("Empty", empty)

----------------------
local wall = {
    Colors  = { "DarkGray" },
    Gravity = 0,
    Density = 100,
    Type    = "Solid",
}
register_element("Wall", wall)

--Powder
----------------------

local sand = {
    Colors  = { "#f6d7b0", "#f2d2a9", "#eccca2", "#e7c496", "#e1bf92" },
    Density = 2.5,
    Type    = "Powder",
}
register_element("Sand", sand)

----------------------
local salt = {
    Colors      = { "#f3fafd" },
    Density     = 2.19,
    Type        = "Powder",
    Transitions = { { Neighbor = "Water", Target = "Salt Water" } }
}
register_element("Salt", salt)

----------------------
local sawdust = {
    Colors      = { "#d9ead4" },
    Density     = 0.21,
    Type        = "Powder",
    Transitions = { { Temperature = 80, Op = "GreaterThanOrEqual", Target = "Charcoal Dust" } }
}
register_element("Sawdust", sawdust)

----------------------

local ash = {
    Colors  = { "#d0c6c6", "#c3b9b9", "#b4a8a8", "#a99d9d", "#9f9393" },
    Density = 2.45,
    Type    = "Powder",
}
register_element("Ash", ash)

----------------------

local charcoal_dust = {
    Colors      = { "#f77a35", "#e34d30", "#c6371c" },
    Density     = 1.4,
    Type        = "Powder",
    Temperature = 1200,
    Transitions = { { Temperature = 80, Op = "LessThan", Target = "Ash" } }
}
register_element("Charcoal Dust", charcoal_dust)

----------------------
local snow = {
    Colors      = { "#b9e8ea" },
    Density     = 0.91,
    Type        = "Powder",
    Temperature = -200,
    Transitions = { { Temperature = 0, Op = "GreaterThanOrEqual", Target = "Water" } }
}
register_element("Snow", snow)

---Liquid
----------------------
local water = {
    Colors      = { "#2389da", "#1ca3ec", "#5abcd8" },
    Density     = 1,
    Type        = "Liquid",
    Temperature = 10,
    Transitions = {
        { Temperature = 100, Op = "GreaterThanOrEqual", Target = "Steam" },
        { Temperature = 0,   Op = "LessThan",           Target = "Snow" },
    }
}
register_element("Water", water)

----------------------
local salt_water = {
    Colors      = { "#00bbbb", "#77eedd", "#007799" },
    Density     = 1.03,
    Type        = "Liquid",
    Temperature = 10,
    Transitions = {
        { Temperature = 102, Op = "GreaterThanOrEqual", Target = "Steam" },
        { Temperature = -21, Op = "LessThan",           Target = "Snow" },
    }
}
register_element("Salt Water", salt_water)

----------------------
local olive_oil = {
    Colors  = { "#d1ef71", "#c1d64d", "#8eb027", "#65880f", "#5f720f" },
    Density = 0.9,
    Type    = "Liquid",
}
register_element("Olive Oil", olive_oil)

----------------------
local honey = {
    Colors  = { "#f9c901" },
    Density = 1.6,
    Type    = "Liquid",
}
register_element("Honey", honey)

--- Solid
----------------------
local wood = {
    Colors      = { "#ba8c63" },
    Gravity     = 0,
    Density     = 0.85,
    Type        = "Solid",
    Transitions = { { Temperature = 100, Op = "GreaterThanOrEqual", Target = "Charcoal" } }
}
register_element("Wood", wood)

----------------------

local charcoal = {
    Colors      = { "#f77a35", "#e34d30", "#c6371c" },
    Gravity     = 0,
    Density     = 1.4,
    Type        = "Solid",
    Temperature = 1200,
    Transitions = { { Temperature = 80, Op = "LessThan", Target = "Ash" } }
}
register_element("Charcoal", charcoal)

----------------------
local ice = {
    Colors      = { "#b9e8ea" },
    Gravity     = 0,
    Density     = 0.91,
    Type        = "Solid",
    Temperature = -200,
    Transitions = { { Temperature = 0, Op = "GreaterThanOrEqual", Target = "Water" } }
}
register_element("Ice", ice)

--- Gas
----------------------
local steam = {
    Colors      = { "#eee", "#ddd", "#ccc" },
    Density     = 0.001,
    Type        = "Gas",
    Temperature = 100,
    Transitions = { { Temperature = 10, Op = "LessThanOrEqual", Target = "Water" } }
}
register_element("Steam", steam)

----------------------
