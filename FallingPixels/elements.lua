-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

----------------------
----------------------

local empty = {
    Colors  = { "transparent" },
    Gravity = 0,
    Density = 0,
    Type    = "None",
}
register_element("Empty", empty)

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
    Colors  = { "#f3fafd" },
    Density = 2.19,
    Type    = "Powder",
    Rules   = { { Neighbor = "Water", NeighborTransformTo = "Salt Water", TransformTo = "Empty" } }
}
register_element("Salt", salt)

----------------------

local sawdust = {
    Colors  = { "#d9ead4" },
    Density = 0.21,
    Type    = "Powder",
    Rules   = { { Temperature = 80, Op = "GreaterThanOrEqual", TransformTo = "Charcoal Dust" } }
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
    Colors = { "#f77a35", "#e34d30", "#c6371c" },
    Density = 1.4,
    Type = "Powder",
    BaseTemperature = 1200,
    ThermalConductivity = 0.10,
    Rules = { { Temperature = 80, Op = "LessThan", TransformTo = "Ash" } }
}
register_element("Charcoal Dust", charcoal_dust)

----------------------

local snow = {
    Colors = { "#b9e8ea" },
    Density = 0.91,
    Type = "Powder",
    BaseTemperature = -200,
    ThermalConductivity = 0.10,
    Rules = { { Temperature = 0, Op = "GreaterThanOrEqual", TransformTo = "Water" } }
}
register_element("Snow", snow)

---Liquid
----------------------

local water = {
    Colors              = { "#2389da", "#11a3d8", "#1ca3e1" },
    Density             = 1,
    Type                = "Liquid",
    BaseTemperature     = 10,
    Dispersion          = 5,
    ThermalConductivity = 1.0,
    Rules               = {
        { Temperature = 100, Op = "GreaterThanOrEqual", TransformTo = "Steam" },
        { Temperature = 0,   Op = "LessThan",           TransformTo = "Snow" },
    }
}
register_element("Water", water)

----------------------

local salt_water = {
    Colors          = { "#17577e", "#188a8d", "#3f9f7f" },
    Density         = 1.03,
    Type            = "Liquid",
    BaseTemperature = 10,
    Dispersion      = 5,
    Rules           = {
        { Temperature = 102, Op = "GreaterThanOrEqual", TransformTo = "Steam" },
        { Temperature = -21, Op = "LessThan",           TransformTo = "Snow" },
    }
}
register_element("Salt Water", salt_water)

----------------------

local olive_oil = {
    Colors     = { "#8eb027", "#65880f", "#5f720f" },
    Density    = 0.9,
    Dispersion = 3,
    Type       = "Liquid",
}
register_element("Olive Oil", olive_oil)

----------------------

local honey = {
    Colors     = { "#f9c901" },
    Density    = 1.6,
    Dispersion = 1,
    Type       = "Liquid",
}
register_element("Honey", honey)

----------------------

local lava = {
    Colors              = { "#ff2500", "#ff6600", "#f2f217" },
    Density             = 2.5,
    Dispersion          = 1,
    BaseTemperature     = 1200,
    ThermalConductivity = 0.01,
    Type                = "Liquid",
}
register_element("Lava", lava)

--- Solid
----------------------

----------------------

local stone = {
    Colors              = { "darkgray" },
    Gravity             = 0,
    Density             = 100,
    ThermalConductivity = 0.00,
    Type                = "Solid",
}
register_element("Stone", stone)

----------------------

local fuse = {
    Colors              = { "darkred" },
    Gravity             = 0,
    Density             = 100,
    Type                = "Solid",
    ThermalConductivity = 0.01,
    Rules               = {
        { Neighbor = "Fire", NeighborTransformTo = "Fire", TransformTo = "Fire" },
        { Temperature = 800, Op = "GreaterThan",           TransformTo = "Fire" },
    }
}
register_element("Fuse", fuse)

----------------------

local wood = {
    Colors  = { "#ba8c63" },
    Gravity = 0,
    Density = 0.85,
    Type    = "Solid",
    Rules   = { { Temperature = 100, Op = "GreaterThanOrEqual", TransformTo = "Charcoal" } }
}
register_element("Wood", wood)

----------------------

local charcoal = {
    Colors          = { "#f77a35", "#e34d30", "#c6371c" },
    Gravity         = 0,
    Density         = 1.4,
    Type            = "Solid",
    BaseTemperature = 1200,
    Rules           = { { Temperature = 80, Op = "LessThan", TransformTo = "Ash" } }
}
register_element("Charcoal", charcoal)

----------------------

local ice = {
    Colors          = { "#b9e8ea" },
    Gravity         = 0,
    Density         = 0.91,
    Type            = "Solid",
    BaseTemperature = -200,
    Rules           = { { Temperature = 0, Op = "GreaterThanOrEqual", TransformTo = "Water" } }
}
register_element("Ice", ice)

--- Gas
----------------------

local steam = {
    Colors          = { "#eee", "#ddd", "#ccc" },
    Density         = 0.001,
    Type            = "Gas",
    BaseTemperature = 100,
    Dispersion      = 10,
    Rules           = { { Temperature = 10, Op = "LessThanOrEqual", TransformTo = "Water" } }
}
register_element("Steam", steam)

----------------------

local fire = {
    Colors          = { "red", "orange", "yellow" },
    Density         = 0.001,
    Type            = "Gas",
    BaseTemperature = 1500,
    Rules           = { { Temperature = 200, Op = "LessThan", TransformTo = "Empty" } }
}
register_element("Fire", fire)


----------------------
