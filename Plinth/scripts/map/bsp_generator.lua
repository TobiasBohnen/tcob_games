-- Copyright (c) 2026 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

-- BSP generator: recursively splits the map into leaves, puts a room in each leaf and joins
-- the rooms of sibling subtrees with L-shaped corridors.
-- https://www.roguebasin.com/index.php/Basic_BSP_Dungeon_generation

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
-- tree
---------------------------------------------------------------------------------------------------

-- Splits a leaf into two children (leaf.left / leaf.right), recursively.
local function split_leaf(leaf, params)
    local min_size          = params.min_leaf_size
    local can_divide_width  = leaf.w >= min_size * 2
    local can_divide_height = leaf.h >= min_size * 2
    if not (can_divide_width or can_divide_height) then return end

    -- once a leaf is small enough, stop at random for more varied room sizes
    local small_enough = leaf.w <= params.max_leaf_size and leaf.h <= params.max_leaf_size
    if small_enough and math.random(1, 100) > 75 then return end

    local divide_width
    if not can_divide_height then
        divide_width = true
    elseif not can_divide_width then
        divide_width = false
    elseif leaf.w > leaf.h * 1.25 then
        divide_width = true
    elseif leaf.h > leaf.w * 1.25 then
        divide_width = false
    else
        divide_width = math.random(1, 2) == 1
    end

    if divide_width then
        local pos  = math.random(min_size, leaf.w - min_size)
        leaf.left  = { x = leaf.x, y = leaf.y, w = pos, h = leaf.h }
        leaf.right = { x = leaf.x + pos, y = leaf.y, w = leaf.w - pos, h = leaf.h }
    else
        local pos  = math.random(min_size, leaf.h - min_size)
        leaf.left  = { x = leaf.x, y = leaf.y, w = leaf.w, h = pos }
        leaf.right = { x = leaf.x, y = leaf.y + pos, w = leaf.w, h = leaf.h - pos }
    end

    split_leaf(leaf.left, params)
    split_leaf(leaf.right, params)
end

-- Random room inside the leaf, keeping a 1-cell margin to the leaf edges; nil if it doesn't fit.
local function make_room(leaf, params)
    local max_w, max_h = leaf.w - 2, leaf.h - 2
    if max_w < 1 or max_h < 1 then return nil end

    local w = math.random(math.min(params.min_room_size, max_w), max_w)
    local h = math.random(math.min(params.min_room_size, max_h), max_h)
    local x = math.random(leaf.x + 1, leaf.x + leaf.w - 1 - w)
    local y = math.random(leaf.y + 1, leaf.y + leaf.h - 1 - h)
    return { x = x, y = y, w = w, h = h }
end

-- Random room from a subtree, or nil if it has none.
local function pick_room(leaf)
    if leaf.room then return leaf.room end
    if not leaf.left then return nil end

    local first, second = leaf.left, leaf.right
    if math.random(1, 2) == 1 then first, second = second, first end
    return pick_room(first) or pick_room(second)
end

---------------------------------------------------------------------------------------------------
-- carving (1-based, cells[y][x]; the outer border stays wall)
---------------------------------------------------------------------------------------------------

local function carve_point(state, params, x, y)
    local radius = params.corridor_radius
    for cy = y - radius, y + radius do
        for cx = x - radius, x + radius do
            if cx >= 2 and cx <= state.width - 1 and cy >= 2 and cy <= state.height - 1 then
                state.cells[cy][cx] = floor_cell(params)
            end
        end
    end
end

-- Carves an axis-aligned line.
local function carve_line(state, params, x1, y1, x2, y2)
    local dx, dy = math.sign(x2 - x1), math.sign(y2 - y1)
    local x, y   = x1, y1

    carve_point(state, params, x, y)
    while x ~= x2 or y ~= y2 do
        x, y = x + dx, y + dy
        carve_point(state, params, x, y)
    end
end

-- L-shaped corridor with a random bend.
local function carve_corridor(state, params, ax, ay, bx, by)
    local bend_x, bend_y = bx, ay
    if math.random(1, 2) == 1 then bend_x, bend_y = ax, by end

    carve_line(state, params, ax, ay, bend_x, bend_y)
    carve_line(state, params, bend_x, bend_y, bx, by)
end

local function carve_room(state, params, room)
    for y = room.y, room.y + room.h - 1 do
        for x = room.x, room.x + room.w - 1 do
            state.cells[y][x] = floor_cell(params)
        end
    end
end

-- Creates and carves the rooms bottom-up, joining the two subtrees of every inner node.
local function build(state, params, leaf)
    if not leaf.left then
        leaf.room = make_room(leaf, params)
        if leaf.room then carve_room(state, params, leaf.room) end
        return
    end

    build(state, params, leaf.left)
    build(state, params, leaf.right)

    local a, b = pick_room(leaf.left), pick_room(leaf.right)
    if a and b then
        carve_corridor(state,
            params,
            math.random(a.x, a.x + a.w - 1), math.random(a.y, a.y + a.h - 1),
            math.random(b.x, b.x + b.w - 1), math.random(b.y, b.y + b.h - 1))
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

    -- the root leaf is the interior, inside the 1-cell border
    local root = { x = 2, y = 2, w = state.width - 2, h = state.height - 2 }
    split_leaf(root, params)
    build(state, params, root)

    return flatten(state)
end

local M = {}

-- params: width, height, default_wall_texture (required);
--         min_leaf_size, max_leaf_size, min_room_size, corridor_radius,
--         floor_texture, ceiling_texture (optional)
-- Returns the complete map as a flat, row-major list of cells: index = (y - 1) * width + x.
function M.generate(params)
    local defaults = {
        min_leaf_size   = 8,
        max_leaf_size   = 20,
        min_room_size   = 4,
        corridor_radius = 0,
        floor_texture   = 10,
        ceiling_texture = 11,
    }

    return generate(table.merge(params, table.merge(defaults, {})))
end

return M
