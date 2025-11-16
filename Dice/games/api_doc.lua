-- Copyright (c) 2025 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

---@meta

------

local game_module = {
    draw_background = function(game_module, engine, canvas, canvasSize) end,

    on_setup = function(game_module, engine) end,
    can_run = function(game_module, engine) end,
    on_run = function(game_module, deltaTime) end,
    can_start = function(game_module, engine) end,
    on_start = function(game_module, engine) end,
    on_collision = function(game_module, engine, spriteA, spriteB) end
}

local gfx_module = {
    get_textures = function(gfx_module, engine) end,
}

------

---@alias texture integer

---@class point
---@field x number
---@field y number

---@class rect
---@field x number
---@field y number
---@field width number
---@field height number

---@class size
---@field width number
---@field height number

---@class slot_face
---@field color string
---@field value number
---@field op string|nil

---@class die_face
---@field color string
---@field values integer[]

---@class spriteDef
---@field type string
---@field texture texture
---@field collisionEnabled boolean|nil

------
---wrapper

---@class canvas
local canvas = {
    begin_path = function(canvas) end,

    ---@param color string
    clear = function(canvas, color) end,

    ---@param path string
    path_2d = function(canvas, path) end,

    ---@param rect rect
    rect = function(canvas, rect) end,

    ---@param color string
    stroke_color = function(canvas, color) end,
    ---@param width number
    stroke_width = function(canvas, width) end,
    stroke = function(canvas) end,

    ---@param color string
    fill_color = function(canvas, color) end,
    ---@param enforeWinding boolean|nil
    fill = function(canvas, enforeWinding) end,
}

---@class sprite
local sprite = {
    Position = { x = 0, y = 0 }, ---@type point
    Rotation = 0, ---@type number
    Type = "", ---@type string
    Index = -1, ---@type number
}

---@class engine
local engine = {
    ---@param bounds rect
    set_dice_area = function(engine, bounds) end,

    ---@param min number
    ---@param max number
    ---@return number
    random = function(engine, min, max) end,

    ---@param min integer
    ---@param max integer
    ---@return integer
    randomInt = function(engine, min, max) end,

    ---@param index integer
    ---@param def spriteDef
    ---@return sprite
    create_sprite = function(engine, index, def) end,

    ---@param pos point
    ---@param slotFace slot_face
    create_slot = function(engine, pos, slotFace) end,

    ---@param count integer
    ---@param dieFaces die_face[]
    create_dice = function(engine, count, dieFaces) end,

    release_dice = function(engine, slots) end,
    roll_dice = function(engine) end,

    ---@return boolean
    are_slots_filled = function(slots) end,

    ---@return boolean
    are_slots_locked = function(slots) end,

    ---@param idx number
    ---@return boolean
    is_slot_empty = function(slots, idx) end,

    ---@param idx number
    ---@return integer
    slot_die_value = function(slot, idx) end,
}
