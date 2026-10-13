-- Copyright (c) 2026 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

-- City generator: the map is open floor; a BSP split gives every leaf a walled building with
-- one door. The streets are whatever is left between the buildings.
-- based on: https://github.com/AtTheMatinee/dungeon-generation (MIT License, Copyright (c) 2017 AtTheMatinee)

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

local function door_cell(params, orientation)
    return {
        type           = "door_wall",
        Texture        = params.default_wall_texture,
        FrameTexture   = params.default_wall_texture,
        FloorTexture   = params.floor_texture,
        CeilingTexture = params.ceiling_texture,
        Orientation    = orientation,
    }
end

---------------------------------------------------------------------------------------------------
-- leaves
---------------------------------------------------------------------------------------------------

-- Recursively splits `rect` and collects the resulting leaves.
local function collect_leaves(rect, params, leaves)
    local min_size          = params.min_leaf_size
    local can_divide_width  = rect.w >= min_size * 2
    local can_divide_height = rect.h >= min_size * 2

    -- split leaves that are too big, and others at random for varied block sizes
    local too_big           = rect.w > params.max_leaf_size or rect.h > params.max_leaf_size
    local split             = (can_divide_width or can_divide_height) and (too_big or math.random(0, 100) > 80)
    if not split then
        leaves[#leaves + 1] = rect
        return
    end

    local divide_width
    if not can_divide_height then
        divide_width = true
    elseif not can_divide_width then
        divide_width = false
    elseif rect.w >= rect.h * 1.25 then
        divide_width = true
    elseif rect.h >= rect.w * 1.25 then
        divide_width = false
    else
        divide_width = math.random(1, 2) == 1
    end

    if divide_width then
        local pos = math.random(min_size, rect.w - min_size)
        collect_leaves({ x = rect.x, y = rect.y, w = pos, h = rect.h }, params, leaves)
        collect_leaves({ x = rect.x + pos, y = rect.y, w = rect.w - pos, h = rect.h }, params, leaves)
    else
        local pos = math.random(min_size, rect.h - min_size)
        collect_leaves({ x = rect.x, y = rect.y, w = rect.w, h = pos }, params, leaves)
        collect_leaves({ x = rect.x, y = rect.y + pos, w = rect.w, h = rect.h - pos }, params, leaves)
    end
end

-- Random building footprint inside the leaf with a 1-cell margin (so streets stay open);
-- nil if even the smallest building (wall, floor, wall) doesn't fit.
local function make_building(leaf, params)
    local max_w = math.min(params.max_room_size, leaf.w - 2)
    local max_h = math.min(params.max_room_size, leaf.h - 2)
    if max_w < 3 or max_h < 3 then return nil end

    local w = math.random(math.min(params.min_room_size, max_w), max_w)
    local h = math.random(math.min(params.min_room_size, max_h), max_h)
    local x = math.random(leaf.x + 1, leaf.x + leaf.w - 1 - w)
    local y = math.random(leaf.y + 1, leaf.y + leaf.h - 1 - h)
    return { x = x, y = y, w = w, h = h }
end

---------------------------------------------------------------------------------------------------
-- drawing (1-based, cells[y][x])
---------------------------------------------------------------------------------------------------

-- Outer ring of walls, floor inside.
local function draw_building(state, params, room)
    for y = room.y, room.y + room.h - 1 do
        for x = room.x, room.x + room.w - 1 do
            local on_wall = x == room.x or x == room.x + room.w - 1
                or y == room.y or y == room.y + room.h - 1
            state.cells[y][x] = on_wall and wall_cell(params.default_wall_texture) or floor_cell(params)
        end
    end
end

-- One door in the middle of a random side.
local function add_door(state, params, room)
    local center_x, center_y = room.x + room.w // 2, room.y + room.h // 2
    local side               = math.random(1, 4)

    if side == 1 then
        state.cells[room.y][center_x] = door_cell(params, "BlocksNorthSouth")
    elseif side == 2 then
        state.cells[room.y + room.h - 1][center_x] = door_cell(params, "BlocksNorthSouth")
    elseif side == 3 then
        state.cells[center_y][room.x] = door_cell(params, "BlocksEastWest")
    else
        state.cells[center_y][room.x + room.w - 1] = door_cell(params, "BlocksEastWest")
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
        for x = 1, state.width do
            local on_border = x == 1 or y == 1 or x == state.width or y == state.height
            row[x] = on_border and wall_cell(params.default_wall_texture) or floor_cell(params)
        end
        state.cells[y] = row
    end

    -- the root leaf is the interior, inside the 1-cell border
    local leaves = {}
    collect_leaves({ x = 2, y = 2, w = state.width - 2, h = state.height - 2 }, params, leaves)

    for _, leaf in ipairs(leaves) do
        local room = make_building(leaf, params)
        if room then
            draw_building(state, params, room)
            add_door(state, params, room)
        end
    end

    return flatten(state)
end

local M = {}

-- params: width, height, default_wall_texture (required);
--         min_leaf_size, max_leaf_size, min_room_size, max_room_size,
--         floor_texture, ceiling_texture (optional)
-- Returns the complete map as a flat, row-major list of cells: index = (y - 1) * width + x.
function M.generate(params)
    local defaults = {
        min_leaf_size   = 10,
        max_leaf_size   = 30,
        min_room_size   = 6,
        max_room_size   = 14,
        floor_texture   = 10,
        ceiling_texture = 11,
    }

    local merged = {}
    for _, source in ipairs { defaults, params } do
        for key, value in pairs(source) do merged[key] = value end
    end
    return generate(merged)
end

return M
