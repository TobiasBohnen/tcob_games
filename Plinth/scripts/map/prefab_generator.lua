-- Copyright (c) 2026 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

-- Prefab generator: stamps ASCII prefab rooms onto the map and links them with corridors.

---------------------------------------------------------------------------------------------------
-- prefab library (textures default to 1)
---------------------------------------------------------------------------------------------------

local PREFABS = {
    { -- plain 7x5 room, one connector centered on each edge
        rows = {
            "1..o..1",
            "1.....1",
            "o.....o",
            "1.....1",
            "1..o..1",
        },
    },
    { -- 7x7 room with two pillars
        rows = {
            "2..o..2",
            "2.....2",
            "2..P..2",
            "o.....o",
            "2..P..2",
            "2.....2",
            "2..o..2",
        },
    },
    { -- L-shaped room (7x7 bounding box, top-right corner walled off)
        rows = {
            "333....",
            "3.D....",
            "333....",
            "o......",
            "3.....o",
            "3......",
            "3..o...",
        },
    },
    { -- small room with a real door (guards the only entrance)
        rows            = {
            "44444",
            "4...4",
            "4...4",
            "4...4",
            "##D##",
        },
        wall_texture    = 2,
        floor_texture   = 2,
        ceiling_texture = 2,
    },
    { -- cross-shaped room
        rows = {
            "555o555",
            "555.555",
            "o.....o",
            "5.....5",
            "o.....o",
            "555.555",
            "555o555",
        },
    },
    { -- long corridor room with pillars
        rows = {
            "o..........o",
            "6.P..P..P..6",
            "6..........6",
            "o..........o",
        },
    },
    { -- double-door vault (two separate entrances)
        rows            = {
            "77777D77777",
            "7.........7",
            "7....P....7",
            "7.........7",
            "77777D77777",
        },
        wall_texture    = 2,
        floor_texture   = 2,
        ceiling_texture = 2,
    },
    { -- room with a secret push-wall exit (south edge)
        rows = {
            "8..o..8",
            "8.....8",
            "8.....8",
            "8.....8",
            "8..S..8",
        },
    },
    { -- room with a freestanding box obstacle (crate) in the middle
        rows = {
            "9..o..9",
            "9.....9",
            "o..B..o",
            "9.....9",
            "9..o..9",
        },
    },
    { -- octagon-ish room using diagonal walls to cut the corners
        rows = {
            "X..o..1",
            "2.....3",
            "o.....o",
            "4.....5",
            "6..o..X",
        },
    },
}

for _, prefab in ipairs(PREFABS) do
    prefab.wall_texture    = prefab.wall_texture or 1
    prefab.floor_texture   = prefab.floor_texture or 1
    prefab.ceiling_texture = prefab.ceiling_texture or 1
end

---------------------------------------------------------------------------------------------------
-- prefab parsing
---------------------------------------------------------------------------------------------------

-- ASCII legend:
--   '#'      normal_wall (prefab.wall_texture)
--   '1'-'9'  normal_wall with that literal texture index
--   '.'      floor_cell
--   'D'      door_wall, orientation from which edge it sits on; connector only on the boundary
--   'S'      push_wall, push_direction from which edge it sits on
--   'B'      obstacle (box)
--   'P'      obstacle (three round pillars)
--   'X'      diagonal_wall, orientation auto-detected from solid N/S/E/W neighbors
--   'o'      connector (open floor cell), boundary only
--   anything else is a floor_cell

local CONNECTOR_SYMBOLS = { D = true, o = true }

local function prefab_size(prefab)
    return #prefab.rows[1], #prefab.rows
end

-- A neighbor counts as "solid" if it's anything other than open floor ('.' or 'o'), or if it
-- falls outside the prefab's own rows.
local function is_solid(prefab, x, y)
    local row = prefab.rows[y]
    if not row or x < 1 or x > #row then return true end

    local symbol = row:sub(x, x)
    return symbol ~= "." and symbol ~= "o"
end

local function wall_cell(texture)
    return { type = "normal_wall", Texture = texture }
end

-- `source` is anything with floor_texture/ceiling_texture: a prefab, or the generator params.
local function with_surfaces(source, cell)
    cell.FloorTexture   = source.floor_texture
    cell.CeilingTexture = source.ceiling_texture
    return cell
end

-- Prefab floors pass their prefab, corridors pass the generator params.
local function floor_cell(source)
    return with_surfaces(source, { type = "floor_cell" })
end

-- Square obstacle shape; position and size are fractions of the cell.
local function shape(x, y, size, is_round, texture)
    return {
        LocalBounds = { x = x, y = y, width = size, height = size },
        IsRound     = is_round,
        Texture     = texture,
    }
end

local cell_factories = {
    ["#"] = function(prefab)
        return wall_cell(prefab.wall_texture)
    end,

    D = function(prefab, _, y)
        return with_surfaces(prefab, {
            type         = "door_wall",
            Texture      = prefab.wall_texture,
            FrameTexture = prefab.wall_texture,
            Orientation  = (y == 1 or y == #prefab.rows) and "BlocksNorthSouth" or "BlocksEastWest",
        })
    end,

    S = function(prefab, x, y)
        local width, height = prefab_size(prefab)

        local direction = { x = 0, y = 0 }
        if y == 1 then
            direction = { x = 0, y = -1 }
        elseif y == height then
            direction = { x = 0, y = 1 }
        elseif x == 1 then
            direction = { x = -1, y = 0 }
        elseif x == width then
            direction = { x = 1, y = 0 }
        end

        return with_surfaces(prefab, {
            type          = "push_wall",
            Texture       = prefab.wall_texture,
            PushDirection = direction,
        })
    end,

    B = function(prefab)
        return with_surfaces(prefab, {
            type   = "obstacle",
            Shapes = { shape(0.2, 0.2, 0.6, false, prefab.wall_texture) },
        })
    end,

    P = function(prefab)
        local texture = prefab.wall_texture
        return with_surfaces(prefab, {
            type   = "obstacle",
            Shapes = {
                shape(0.1, 0.1, 0.2, true, texture),
                shape(0.5, 0.5, 0.2, true, texture),
                shape(0.7, 0.3, 0.2, true, texture),
            },
        })
    end,

    X = function(prefab, x, y)
        local north, south = is_solid(prefab, x, y - 1), is_solid(prefab, x, y + 1)
        local west, east   = is_solid(prefab, x - 1, y), is_solid(prefab, x + 1, y)
        local rising       = (north and west) or (south and east)

        return with_surfaces(prefab, {
            type        = "diagonal_wall",
            Texture     = prefab.wall_texture,
            Orientation = rising and "SouthWestToNorthEast" or "NorthWestToSouthEast",
        })
    end,

    fallback = function(p, _, _) return floor_cell(p) end
}

for digit = 1, 9 do
    cell_factories[tostring(digit)] = function() return wall_cell(digit) end
end

---------------------------------------------------------------------------------------------------
-- grid helpers (1-based, grid[y][x])
---------------------------------------------------------------------------------------------------

local function new_grid(width, height, value)
    local grid = {}
    for y = 1, height do
        local row = {}
        for x = 1, width do row[x] = value end
        grid[y] = row
    end
    return grid
end

local function copy_grid(grid)
    local copy = {}
    for y, row in ipairs(grid) do copy[y] = table.move(row, 1, #row, 1, {}) end
    return copy
end

local function in_bounds(grid, x, y)
    return grid[y] ~= nil and grid[y][x] ~= nil
end

-- Every cell write goes through here so `occupied` can't get out of sync.
local function set_cell(state, x, y, cell)
    state.cells[y][x]    = cell
    state.occupied[y][x] = true
end

---------------------------------------------------------------------------------------------------
-- placement
---------------------------------------------------------------------------------------------------

local function stamp_prefab(state, prefab, ox, oy)
    local width, height = prefab_size(prefab)
    local connectors    = {}

    for y = 1, height do
        for x = 1, width do
            local symbol  = prefab.rows[y]:sub(x, x)
            local wx, wy  = ox + x - 1, oy + y - 1

            local factory = cell_factories[symbol] or cell_factories.fallback
            set_cell(state, wx, wy, factory(prefab, x, y))

            local on_edge = x == 1 or y == 1 or x == width or y == height
            if on_edge and CONNECTOR_SYMBOLS[symbol] then
                connectors[#connectors + 1] = { x = wx, y = wy }
            end
        end
    end

    return connectors
end

local function is_area_free(occupied, x1, y1, x2, y2)
    for y = y1, y2 do
        for x = x1, x2 do
            if occupied[y][x] ~= false then return false end
        end
    end
    return true
end

-- Returns the origin of a free spot (padded by 1 cell so prefabs never touch), or nil.
local function try_place_prefab(state, prefab, params)
    local width, height = prefab_size(prefab)
    local max_x, max_y  = params.width - width, params.height - height
    if max_x < 2 or max_y < 2 then return nil end

    for _ = 1, params.placement_attempts do
        local ox, oy = math.random(2, max_x), math.random(2, max_y)
        if is_area_free(state.occupied, ox - 1, oy - 1, ox + width, oy + height) then
            return ox, oy
        end
    end
end

local function add_border_walls(state, texture)
    for x = 1, state.width do
        set_cell(state, x, 1, wall_cell(texture))
        set_cell(state, x, state.height, wall_cell(texture))
    end
    for y = 2, state.height - 1 do
        set_cell(state, 1, y, wall_cell(texture))
        set_cell(state, state.width, y, wall_cell(texture))
    end
end

local function pick_weighted_prefab(total_weight)
    local roll = math.random(0, total_weight - 1)
    for _, prefab in ipairs(PREFABS) do
        roll = roll - (prefab.weight or 1)
        if roll < 0 then return prefab end
    end
end

-- Keeps placing random prefabs until one no longer fits; returns the placed prefabs.
local function place_prefabs(state, params)
    local total_weight = 0
    for _, prefab in ipairs(PREFABS) do total_weight = total_weight + (prefab.weight or 1) end

    local placed = {}
    while total_weight > 0 do
        local prefab = pick_weighted_prefab(total_weight)
        if not prefab then break end

        local ox, oy = try_place_prefab(state, prefab, params)
        if not ox then break end

        local width, height = prefab_size(prefab)
        placed[#placed + 1] = {
            x          = ox,
            y          = oy,
            width      = width,
            height     = height,
            connectors = stamp_prefab(state, prefab, ox, oy),
        }
    end

    return placed
end

---------------------------------------------------------------------------------------------------
-- corridors
---------------------------------------------------------------------------------------------------

local DIRECTIONS = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } }

-- BFS around blocked cells; returns nil if no route exists (caller decides fallback).
local function find_corridor_path(blocked, from, to)
    local width = #blocked[1]
    local function key(x, y) return y * width + x end

    local came_from = { [key(from.x, from.y)] = from }
    local frontier  = { from }
    local head      = 1

    while head <= #frontier do
        local cur = frontier[head]
        head = head + 1

        if cur.x == to.x and cur.y == to.y then
            local path = { cur }
            while cur ~= from do
                cur = came_from[key(cur.x, cur.y)]
                table.insert(path, 1, cur)
            end
            return path
        end

        for _, d in ipairs(DIRECTIONS) do
            local nx, ny = cur.x + d[1], cur.y + d[2]
            if in_bounds(blocked, nx, ny) and not came_from[key(nx, ny)] then
                -- stepping onto the destination is allowed even though it's "blocked" (its own connector)
                local is_target = nx == to.x and ny == to.y
                if is_target or not blocked[ny][nx] then
                    came_from[key(nx, ny)]  = cur
                    frontier[#frontier + 1] = { x = nx, y = ny }
                end
            end
        end
    end
end

local function carve_point(state, params, blocked, p)
    local radius = params.corridor_radius
    for y = p.y - radius, p.y + radius do
        for x = p.x - radius, p.x + radius do
            if in_bounds(state.cells, x, y) and not blocked[y][x] then
                set_cell(state, x, y, floor_cell(params))
            end
        end
    end
end

local function carve_corridor(state, params, from, to, blocked)
    local path = find_corridor_path(blocked, from, to)

    if path then
        for _, p in ipairs(path) do carve_point(state, params, blocked, p) end
        return
    end

    -- No route around obstacles (extremely tight layout): straight-line fallback,
    -- so the rooms end up connected even if it cuts through something.

    local x, y           = from.x, from.y
    local step_x, step_y = math.sign(to.x - from.x), math.sign(to.y - from.y)
    while x ~= to.x or y ~= to.y do
        carve_point(state, params, blocked, { x = x, y = y })
        if x ~= to.x then x = x + step_x end
        if y ~= to.y then y = y + step_y end
    end
    carve_point(state, params, blocked, to)
end

---------------------------------------------------------------------------------------------------
-- linking
---------------------------------------------------------------------------------------------------

local function center_of(placed)
    return placed.x + placed.width // 2, placed.y + placed.height // 2
end

local function find_nearest_unlinked_pair(placed, linked)
    local best_from, best_to, best_dist = nil, nil, math.huge

    for i, a in ipairs(placed) do
        if linked[i] then
            local ax, ay = center_of(a)
            for j, b in ipairs(placed) do
                if not linked[j] then
                    local bx, by = center_of(b)
                    local dist   = (ax - bx) ^ 2 + (ay - by) ^ 2
                    if dist < best_dist then best_from, best_to, best_dist = i, j, dist end
                end
            end
        end
    end

    return best_from, best_to
end

local function pick_connector(placed, used)
    local connector
    if #placed.connectors == 0 then
        local cx, cy = center_of(placed)
        connector = { x = cx, y = cy }
    else
        connector = placed.connectors[math.random(#placed.connectors)]
    end

    used[connector.y][connector.x] = true
    return connector
end

local function connect_prefabs(state, params, placed)
    if #placed < 2 then return end

    -- Snapshot prefab cells before carving, so corridors are only blocked by rooms,
    -- never by earlier corridor segments.
    local blocked = copy_grid(state.occupied)
    local used    = new_grid(state.width, state.height, false)

    -- Minimum spanning tree over room centers, so every room is reachable.
    local linked  = { true }

    for _ = 2, #placed do
        local from, to = find_nearest_unlinked_pair(placed, linked)
        if not from or not to then break end

        carve_corridor(state, params, pick_connector(placed[from], used), pick_connector(placed[to], used), blocked)
        linked[to] = true
    end

    -- Seal every connector that was never used as a corridor endpoint.
    for _, p in ipairs(placed) do
        for _, c in ipairs(p.connectors) do
            if not used[c.y][c.x] then
                set_cell(state, c.x, c.y, wall_cell(params.default_wall_texture))
            end
        end
    end
end

---------------------------------------------------------------------------------------------------
-- generate
---------------------------------------------------------------------------------------------------

local function fill_remaining_with_wall(state, texture)
    for y = 1, state.height do
        for x = 1, state.width do
            if not state.occupied[y][x] then
                set_cell(state, x, y, wall_cell(texture))
            end
        end
    end
end

-- Row-major list: index = (y - 1) * width + x
local function flatten(state)
    local flat = {}
    for y = 1, state.height do
        for x = 1, state.width do flat[#flat + 1] = state.cells[y][x] end
    end
    return flat
end

local function generate(params)
    local state = {
        width    = params.width,
        height   = params.height,
        cells    = new_grid(params.width, params.height, false),
        occupied = new_grid(params.width, params.height, false),
    }

    add_border_walls(state, params.default_wall_texture)
    local placed = place_prefabs(state, params)
    connect_prefabs(state, params, placed)
    fill_remaining_with_wall(state, params.default_wall_texture)

    return flatten(state)
end

local M = {}

-- params: width, height, default_wall_texture (required);
--         placement_attempts, corridor_radius, floor_texture, ceiling_texture (optional;
--         the textures apply to corridors)
-- Returns the complete map as a flat, row-major list of cells: index = (y - 1) * width + x.
function M.generate(params)
    local defaults = {
        placement_attempts = 50,
        corridor_radius    = 0,
        floor_texture      = 10,
        ceiling_texture    = 11,
    }

    local merged = {}
    for _, source in ipairs { defaults, params } do
        for key, value in pairs(source) do merged[key] = value end
    end
    return generate(merged)
end

return M
