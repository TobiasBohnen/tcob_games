-- Copyright (c) 2026 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

-- Drunkard's walk generator: a random walker carves floor out of solid wall until a target
-- share of the map is open. Produces organic, cave-like layouts that are always connected.
-- based on: https://github.com/AtTheMatinee/dungeon-generation (MIT License, Copyright (c) 2017 AtTheMatinee)

local UP, DOWN, RIGHT, LEFT = 1, 2, 3, 4

local MOVES = {
    [UP]    = { dx = 0, dy = -1 },
    [DOWN]  = { dx = 0, dy = 1 },
    [RIGHT] = { dx = 1, dy = 0 },
    [LEFT]  = { dx = -1, dy = 0 },
}

---------------------------------------------------------------------------------------------------
-- cells
---------------------------------------------------------------------------------------------------

local function wall_cell(texture)
    return { type = "normal_wall", Texture = texture }
end

local function floor_cell(params)
    return {
        type           = "floor_cell",
        FloorTexture   = params.floor_texture,
        CeilingTexture = params.ceiling_texture,
    }
end

---------------------------------------------------------------------------------------------------
-- walk
---------------------------------------------------------------------------------------------------

-- One weight per direction: nudged back towards the map center when the walker nears an edge,
-- and towards the direction of the previous step.
local function move_weights(state, params, x, y, previous)
    local weights = { 1, 1, 1, 1 }
    local center  = params.weighted_toward_center

    if x < state.width * 0.25 then
        weights[RIGHT] = weights[RIGHT] + center
    elseif x > state.width * 0.75 then
        weights[LEFT] = weights[LEFT] + center
    end
    if y < state.height * 0.25 then
        weights[DOWN] = weights[DOWN] + center
    elseif y > state.height * 0.75 then
        weights[UP] = weights[UP] + center
    end

    if previous then
        weights[previous] = weights[previous] + params.weighted_toward_previous_direction
    end

    return weights
end

-- Weighted random pick; returns the index of the chosen direction.
local function pick_move(weights)
    local total = 0
    for _, weight in ipairs(weights) do total = total + weight end

    local choice = math.random(0, 9999) / 10000 * total
    for i, weight in ipairs(weights) do
        choice = choice - weight
        if choice < 0 then return i end
    end
    return #weights
end

local function walk(state, params)
    local goal        = state.width * state.height * params.percent_goal

    local x, y        = math.random(3, state.width - 1), math.random(3, state.height - 1)
    state.cells[y][x] = floor_cell(params)
    local filled      = 1
    local previous    = nil

    for _ = 1, state.width * state.height * 10 do
        if filled >= goal then break end

        local move   = pick_move(move_weights(state, params, x, y, previous))
        local nx, ny = x + MOVES[move].dx, y + MOVES[move].dy

        -- never step onto the outer border
        if nx > 1 and nx < state.width and ny > 1 and ny < state.height then
            x, y = nx, ny
            if state.cells[y][x].type ~= "floor_cell" then
                state.cells[y][x] = floor_cell(params)
                filled = filled + 1
            end
            previous = move
        end
    end
end

---------------------------------------------------------------------------------------------------
-- generate
---------------------------------------------------------------------------------------------------

-- Row-major list: index = (y - 1) * width + x
local function flatten(state)
    local flat = {}
    for y = 1, state.height do
        for x = 1, state.width do flat[#flat + 1] = state.cells[y][x] end
    end
    return flat
end

local function generate(params)
    local state = { width = params.width, height = params.height, cells = {} }
    for y = 1, state.height do
        local row = {}
        for x = 1, state.width do row[x] = wall_cell(params.default_wall_texture) end
        state.cells[y] = row
    end

    walk(state, params)

    return flatten(state)
end

local M = {}

-- params: width, height, default_wall_texture (required);
--         percent_goal, weighted_toward_center, weighted_toward_previous_direction,
--         floor_texture, ceiling_texture (optional)
-- Returns the complete map as a flat, row-major list of cells: index = (y - 1) * width + x.
function M.generate(params)
    local defaults = {
        percent_goal                       = 0.4,
        weighted_toward_center             = 0.15,
        weighted_toward_previous_direction = 0.7,
        floor_texture                      = 10,
        ceiling_texture                    = 11,
    }

    local merged = {}
    for _, source in ipairs { defaults, params } do
        for key, value in pairs(source) do merged[key] = value end
    end
    return generate(merged)
end

return M
