-- Copyright (c) 2026 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local generators = {
    prefab = require("map/prefab_generator"),
    bsp    = require("map/bsp_generator"),
}

local DEFAULTS = {
    generator            = "prefab",
    width                = MAP_WIDTH,
    height               = MAP_HEIGHT,
    default_wall_texture = 1,
}

return {
    generate_map = function(player)
        return generators[DEFAULTS.generator].generate(DEFAULTS)
    end,
}

--[[
TODO:
- register external map generators
- make built-in generators moddable
- monsters (+ behavior, spawn)
- other objects (+ behavior, spawn)
- spell components
--]]
