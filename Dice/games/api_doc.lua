-- Copyright (c) 2025 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

---@meta

------

local game_module = {
    draw_background = function(game_module, engine, canvas, canvasSize) end,

    on_setup = function(game_module, engine) end,
    can_process_turn = function(game_module, engine) end,
    on_process_turn = function(game_module, deltaTime) end,
    can_end_turn = function(game_module, engine) end,
    on_end_turn = function(game_module, engine) end,
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
---@field op string

---@class slot_def
---@field position point
---@field face slot_face|nil

---@class die_face
---@field color string
---@field value number

---@class die_def
---@field position point
---@field faces die_face[]|nil

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

---@class slots
local slots = {
    ---@return boolean
    is_complete = function(slots) end,

    lock = function(slots) end,
    unlock = function(slots) end,

    ---@return boolean
    are_locked = function(slots) end,

    ---@param idx number
    ---@return boolean
    slot_is_empty = function(slots, idx) end,

    ---@param idx number
    ---@return integer
    slot_die_value = function(slot, idx) end,
}

---@class engine
local engine = {
    ---@param min number
    ---@param max number
    ---@return number
    random = function(engine, min, max) end,

    ---@param min integer
    ---@param max integer
    ---@return integer
    randomInt = function(engine, min, max) end,

    ---@param size size
    ---@param func function
    ---@return texture
    create_texture = function(engine, size, func) end,

    ---@param index integer
    ---@param def spriteDef
    ---@return sprite
    create_sprite = function(engine, index, def) end,

    ---@param slotDefs slot_def[]
    ---@return slots
    create_slots = function(engine, slotDefs) end,

    ---@param dieDefs die_def[]
    create_dice = function(engine, dieDefs) end,

    roll_dice = function(engine) end,
}
