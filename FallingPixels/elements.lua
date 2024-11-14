-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

element("Empty", {
    Colors  = { "transparent" },
    Gravity = 0,
    Density = 0,
    Type    = "None",
})


--- Solid

element("Stone", {
    Colors              = { "darkgray" },
    Gravity             = 0,
    Density             = 100,
    ThermalConductivity = 0.00,
    Type                = "Solid",
})

element("Fuse", {
    Colors              = { "darkred" },
    Gravity             = 0,
    Density             = 100,
    Type                = "Solid",
    ThermalConductivity = 0.01,
    Rules               = {
        { Neighbor = "Fire", NeighborTransformTo = "Fire", TransformTo = "Fire" },
        { Temperature = 800, Op = "GreaterThan",           TransformTo = "Fire" },
    }
})

element("Wood", {
    Colors  = { "#ba8c63" },
    Gravity = 0,
    Density = 0.85,
    Type    = "Solid",
    Rules   = { { Temperature = 100, Op = "GreaterThanOrEqual", TransformTo = "Charcoal" } }
})

element("Charcoal", {
    Colors          = { "#f77a35", "#e34d30", "#c6371c" },
    Gravity         = 0,
    Density         = 1.4,
    Type            = "Solid",
    BaseTemperature = 1200,
    Rules           = { { Temperature = 80, Op = "LessThan", TransformTo = "Ash" } }
})

element("Ice", {
    Colors          = { "#b9e8ea" },
    Gravity         = 0,
    Density         = 0.91,
    Type            = "Solid",
    BaseTemperature = -200,
    Rules           = { { Temperature = 0, Op = "GreaterThanOrEqual", TransformTo = "Water" } }
})

element("Metal", {
    Colors  = { "#8c8c8c", "#b0b0b0" },
    Gravity = 0,
    Density = 7.87,
    Type    = "Solid",
    Rules   = { { Temperature = 1500, Op = "GreaterThanOrEqual", TransformTo = "Molten Metal" } }
})

element("Solid Mercury", {
    Colors          = { "#dcdcdc", "#c0c0c0" },
    Gravity         = 0,
    Density         = 13.534,
    Type            = "Solid",
    BaseTemperature = -50,
    Rules           = { { Temperature = -38, Op = "GreaterThanOrEqual", TransformTo = "Mercury" } }
})

--Powder

element("Sand", {
    Colors     = { "#f6d7b0", "#f2d2a9", "#eccca2", "#e7c496", "#e1bf92" },
    Gravity    = 1,
    Density    = 2.5,
    Dispersion = 3,
    Type       = "Powder",
})

element("Salt", {
    Colors     = { "#f3fafd" },
    Gravity    = 1,
    Density    = 2.19,
    Dispersion = 3,
    Type       = "Powder",
    Rules      = { { Neighbor = "Water", NeighborTransformTo = "Salt Water", TransformTo = "Empty" } }
})

element("Sawdust", {
    Colors  = { "#d9ead4" },
    Gravity = 1,
    Density = 0.21,
    Type    = "Powder",
    Rules   = { { Temperature = 80, Op = "GreaterThanOrEqual", TransformTo = "Charcoal Dust" } }
})

element("Ash", {
    Colors  = { "#d0c6c6", "#c3b9b9", "#b4a8a8", "#a99d9d", "#9f9393" },
    Gravity = 1,
    Density = 2.45,
    Type    = "Powder",
})

element("Charcoal Dust", {
    Colors = { "#f77a35", "#e34d30", "#c6371c" },
    Gravity = 1,
    Density = 1.4,
    Type = "Powder",
    BaseTemperature = 1200,
    ThermalConductivity = 0.10,
    Rules = { { Temperature = 80, Op = "LessThan", TransformTo = "Ash" } }
})

element("Snow", {
    Colors = { "#b9e8ea" },
    Gravity = 1,
    Density = 0.91,
    Type = "Powder",
    BaseTemperature = -200,
    ThermalConductivity = 0.10,
    Rules = { { Temperature = 0, Op = "GreaterThanOrEqual", TransformTo = "Water" } }
})

---Liquid

element("Water", {
    Colors              = { "#2389da", "#11a3d8", "#1ca3e1" },
    Gravity             = 1,
    Density             = 1,
    Type                = "Liquid",
    BaseTemperature     = 10,
    Dispersion          = 5,
    ThermalConductivity = 1.0,
    Rules               = {
        { Temperature = 100, Op = "GreaterThanOrEqual", TransformTo = "Steam" },
        { Temperature = 0,   Op = "LessThan",           TransformTo = "Snow" },
    }
})

element("Salt Water", {
    Colors          = { "#17577e", "#188a8d", "#3f9f7f" },
    Gravity         = 1,
    Density         = 1.03,
    Type            = "Liquid",
    BaseTemperature = 10,
    Dispersion      = 5,
    Rules           = {
        { Temperature = 102, Op = "GreaterThanOrEqual", TransformTo = "Steam" },
        { Temperature = -21, Op = "LessThan",           TransformTo = "Snow" },
    }
})

element("Olive Oil", {
    Colors     = { "#8eb027", "#65880f", "#5f720f" },
    Gravity    = 1,
    Density    = 0.9,
    Dispersion = 3,
    Type       = "Liquid",
})

element("Honey", {
    Colors     = { "#f9c901" },
    Gravity    = 1,
    Density    = 1.6,
    Dispersion = 1,
    Type       = "Liquid",
})

element("Lava", {
    Colors              = { "#ff2500", "#ff6600", "#f2f217" },
    Gravity             = 1,
    Density             = 2.5,
    Dispersion          = 1,
    BaseTemperature     = 1200,
    ThermalConductivity = 0.01,
    Type                = "Liquid",
})

element("Molten Metal", {
    Colors              = { "#ffcc00", "#ffaa00" },
    Gravity             = 1,
    Density             = 7.87,
    Type                = "Liquid",
    BaseTemperature     = 1500,
    ThermalConductivity = 0.05,
    Dispersion          = 1,
    Rules               = { { Temperature = 1300, Op = "LessThan", TransformTo = "Metal" } }
})

element("Mercury", {
    Colors              = { "#dcdcdc", "#c0c0c0" },
    Gravity             = 1,
    Density             = 13.534,
    Type                = "Liquid",
    Dispersion          = 2,
    BaseTemperature     = -38,
    ThermalConductivity = 0.09,
    Rules               = { { Temperature = -39, Op = "LessThan", TransformTo = "Solid Mercury" } }
})

element("Acid", {
    Colors          = { "#a8e000", "#87d300", "#72bf00" },
    Gravity         = 1,
    Density         = 1.02,
    Type            = "Liquid",
    BaseTemperature = 20,
    Dispersion      = 4,
    Rules           = {
        { Neighbor = "Metal", NeighborTransformTo = "Empty", TransformTo = "Empty" },
        { Temperature = 100,  Op = "GreaterThanOrEqual",     TransformTo = "Acid Vapor" },
    }
})

--- Gas

element("Steam", {
    Colors          = { "#eee", "#ddd", "#ccc" },
    Gravity         = -1,
    Density         = 0.001,
    Type            = "Gas",
    BaseTemperature = 100,
    Dispersion      = 10,
    Rules           = { { Temperature = 10, Op = "LessThanOrEqual", TransformTo = "Water" } }
})

element("Fire", {
    Colors          = { "red", "orange", "yellow" },
    Gravity         = -1,
    Density         = 0.001,
    Type            = "Gas",
    BaseTemperature = 1500,
    Rules           = { { Temperature = 200, Op = "LessThan", TransformTo = "Empty" } }
})

element("Plasma", {
    Colors          = { "#ff41ca", "#5f007f", "#2335be" },
    Gravity         = -1,
    Density         = 0.0001,
    Type            = "Gas",
    BaseTemperature = 5000,
    Dispersion      = 15,
    Rules           = { { Temperature = 1000, Op = "LessThan", TransformTo = "Fire" } }
})

element("Acid Vapor", {
    Colors          = { "#d3f0b0", "#c0e08a" },
    Gravity         = -1,
    Density         = 0.001,
    Type            = "Gas",
    BaseTemperature = 100,
    Dispersion      = 8,
    Rules           = {
        { Temperature = 30,   Op = "LessThan",               TransformTo = "Acid" },
        { Neighbor = "Metal", NeighborTransformTo = "Empty", TransformTo = "Empty" }, }
})

element("Smoke", {
    Colors     = { "#333333", "#666666", "#999999" },
    Gravity    = -1,
    Density    = 0.001,
    Type       = "Gas",
    Dispersion = 8,
    Rules      = { { Temperature = 10, Op = "LessThanOrEqual", TransformTo = "Ash" } }
})
