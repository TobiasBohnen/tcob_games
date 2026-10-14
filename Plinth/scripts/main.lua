-- Copyright (c) 2026 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local generators = {
    prefab   = require("map/prefab_generator"),
    bsp      = require("map/bsp_generator"),
    drunkard = require("map/drunkard_generator"),
    city     = require("map/city_generator"),
}

local function find_player_start(cells, width, height)
    local floors = {}
    for i, cell in ipairs(cells) do
        if cell.type == "floor_cell" then floors[#floors + 1] = i end
    end

    if #floors == 0 then return width // 2, height // 2 end

    local index = floors[math.random(#floors)] - 1
    return index % width, index // width
end

return {
    generate_level = function(level, player)
        local defaults = {
            width                = MAP_WIDTH,
            height               = MAP_HEIGHT,
            generator            = "prefab",
            default_wall_texture = 1,
        }

        local width = defaults.width
        local cells = generators[defaults.generator].generate(defaults)

        for i, cell in ipairs(cells) do
            level:set_cell((i - 1) % width, (i - 1) // width, cell)
        end

        local x, y = find_player_start(cells, width, defaults.height)
        player.position = { x = x + 0.5, y = y + 0.5 }
    end,
}
