-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

element("Empty", {
    Colors  = { "black" },
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
        { Neighbor = { Element = "Fire", NeighborResult = "Fire", Result = "Fire" } },
        { Temperature = { Above = 800, Result = "Fire" } },
    }
})

element("Wood", {
    Colors  = { "#ba8c63" },
    Gravity = 0,
    Density = 0.85,
    Type    = "Solid",
    Rules   = {
        { Temperature = { Above = 100, Result = "Charcoal" } },
    }
})

element("Charcoal", {
    Colors      = { "#f77a35", "#e34d30", "#c6371c" },
    Gravity     = 0,
    Density     = 1.4,
    Type        = "Solid",
    Temperature = 1200,
    Rules       = {
        { Temperature = { Below = 80, Result = "Ash" } },
    }
})

element("Ice", {
    Colors      = { "#b9e8ea" },
    Gravity     = 0,
    Density     = 0.91,
    Type        = "Solid",
    Temperature = -200,
    Rules       = {
        { Temperature = { Above = 0, Result = "Water" } },
    }
})

element("Metal", {
    Colors  = { "#8c8c8c", "#b0b0b0" },
    Gravity = 0,
    Density = 7.87,
    Type    = "Solid",
    Rules   = {
        { Temperature = { Above = 1500, Result = "Molten Metal" } }
    },
})

element("Solid Mercury", {
    Colors      = { "#dcdcdc", "#c0c0c0" },
    Gravity     = 0,
    Density     = 13.534,
    Type        = "Solid",
    Temperature = -50,
    Rules       = {
        { Temperature = { Above = -38, Result = "Mercury" } }
    },
})

element("Concrete", {
    Colors  = { "#9e9e9e", "#7d7d7d" },
    Gravity = 0,
    Density = 2.4,
    Type    = "Solid",
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
    Colors      = { "#f3fafd" },
    Gravity     = 1,
    Density     = 2.19,
    Dispersion  = 3,
    Dissolvable = true,
    Type        = "Powder"
})

element("Sawdust", {
    Colors  = { "#d9ead4" },
    Gravity = 1,
    Density = 0.21,
    Type    = "Powder",
    Rules   = {
        { Temperature = { Above = 80, Result = "Charcoal Dust" } }
    },
})

element("Ash", {
    Colors  = { "#d0c6c6", "#c3b9b9", "#b4a8a8", "#a99d9d", "#9f9393" },
    Gravity = 1,
    Density = 2.45,
    Type    = "Powder",
})

element("Charcoal Dust", {
    Colors              = { "#f77a35", "#e34d30", "#c6371c" },
    Gravity             = 1,
    Density             = 1.4,
    Type                = "Powder",
    Temperature         = 1200,
    ThermalConductivity = 0.10,
    Rules               = {
        { Temperature = { Below = 80, Result = "Ash" } }
    },
})

element("Snow", {
    Colors              = { "#b9e8ea" },
    Gravity             = 1,
    Density             = 0.91,
    Type                = "Powder",
    Temperature         = -200,
    ThermalConductivity = 0.10,
    Rules               = {
        { Temperature = { Above = 0, Result = "Water" } }
    },
})

element("Cement", {
    Colors  = { "#a8a8a8", "#bcbcbc" },
    Gravity = 1,
    Density = 3.1,
    Type    = "Powder",
    Rules   = {
        { Dissolve = { Element = "Water", Result = "Concrete" } },
    }
})

---Liquid

element("Water", {
    Colors              = { "#2389da", "#11a3d8", "#1ca3e1" },
    Gravity             = 1,
    Density             = 1,
    Type                = "Liquid",
    Temperature         = 10,
    Dispersion          = 5,
    ThermalConductivity = 1.0,
    Rules               = {
        { Temperature = { Above = 100, Result = "Steam" } },
        { Temperature = { Below = 0, Result = "Snow" } },
        { Dissolve = { Element = "Salt", Result = "Salt Water" } },
    }
})

element("Salt Water", {
    Colors      = { "#17577e", "#188a8d", "#3f9f7f" },
    Gravity     = 1,
    Density     = 1.03,
    Type        = "Liquid",
    Temperature = 10,
    Dispersion  = 5,
    Rules       = {
        { Temperature = { Above = 102, Result = "Steam" } },
        { Temperature = { Below = -21, Result = "Snow" } },
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
    Temperature         = 1200,
    ThermalConductivity = 0.01,
    Type                = "Liquid",
})

element("Molten Metal", {
    Colors              = { "#ffcc00", "#ffaa00" },
    Gravity             = 1,
    Density             = 7.87,
    Type                = "Liquid",
    Temperature         = 1500,
    ThermalConductivity = 0.05,
    Dispersion          = 1,
    Rules               = {
        { Temperature = { Below = 1300, Result = "Metal" } },
    }
})

element("Mercury", {
    Colors              = { "#dcdcdc", "#c0c0c0" },
    Gravity             = 1,
    Density             = 13.534,
    Type                = "Liquid",
    Dispersion          = 2,
    Temperature         = -38,
    ThermalConductivity = 0.09,
    Rules               = {
        { Temperature = { Below = -39, Result = "Solid Mercury" } },
    }
})

element("Acid", {
    Colors      = { "#a8e000", "#87d300", "#72bf00" },
    Gravity     = 1,
    Density     = 1.02,
    Type        = "Liquid",
    Temperature = 20,
    Dispersion  = 4,
    Dissolvable = false,
    Rules       = {
        { Temperature = { Above = 100, Result = "Acid Vapor" } },
        { Dissolve = { Element = "Any", Result = "Empty" } },
    }
})

--- Gas

element("Steam", {
    Colors      = { "#eee", "#ddd", "#ccc" },
    Gravity     = -1,
    Density     = 0.001,
    Type        = "Gas",
    Temperature = 100,
    Dispersion  = 10,
    Rules       = {
        { Temperature = { Below = 75, Result = "Water" } },
    }
})

element("Fire", {
    Colors      = { "red", "orange", "yellow" },
    Gravity     = -1,
    Density     = 0.001,
    Type        = "Gas",
    Temperature = 1500,
    Rules       = {
        { Temperature = { Below = 200, Result = "Smoke" } },
    }
})

element("Plasma", {
    Colors      = { "#ff41ca", "#5f007f", "#2335be" },
    Gravity     = -1,
    Density     = 0.0001,
    Type        = "Gas",
    Temperature = 5000,
    Dispersion  = 15,
    Rules       = {
        { Temperature = { Below = 1000, Result = "Fire" } },
    }
})

element("Acid Vapor", {
    Colors = { "#d3f0b0", "#c0e08a" },
    Gravity = -1,
    Density = 0.001,
    Type = "Gas",
    Temperature = 100,
    Dispersion = 8,
    Dissolvable = false,
    Rules = {
        { Temperature = { Below = 30, Result = "Acid" } },
        { Dissolve = { Element = "Any", Result = "Empty" } },
    }
})

element("Smoke", {
    Colors     = { "#333", "#666", "#999" },
    Gravity    = -1,
    Density    = 0.001,
    Type       = "Gas",
    Dispersion = 8,
    Rules      = {
        { Temperature = { Below = 10, Result = "Ash" } },
    }
})

element("Carbon Dioxide", {
    Colors     = { "#d9d9d9", "#c4c4c4" },
    Gravity    = -1,
    Density    = 0.00198,
    Type       = "Gas",
    Dispersion = 12,
})

element("Oxygen", {
    Colors     = { "#99d8ff", "#8dcff8" },
    Gravity    = -1,
    Density    = 0.00143,
    Type       = "Gas",
    Dispersion = 15,
    Rules      = {
        { Neighbor = { Element = "Fire", NeighborResult = "Fire", Result = "Fire" } },
    }
})

element("Helium", {
    Colors     = { "#f4f9ff", "#d9e8ff" },
    Gravity    = -2,
    Density    = 0.000179,
    Type       = "Gas",
    Dispersion = 20,
})
