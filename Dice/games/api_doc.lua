-- Copyright (c) 2025 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

---@meta

------

local game_module = {
    on_setup = function(game_module, engine) end,
    can_process_turn = function(game_module, engine) end,
    on_process_turn = function(game_module, deltaTime) end,
    can_end_turn = function(game_module, engine) end,
    on_end_turn = function(game_module, engine) end
}
local gfx_module = {
    draw_background = function(gfx_module, engine, canvas, canvasSize) end,
    get_textures = function(gfx_module, engine) end,
}

------

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

---@alias texture integer

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
    ---@param point point
    position = function(sprite, point) end,
    ---@param rot number
    rotation = function(sprite, rot) end,
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

    ---@param size size
    ---@param func function
    ---@return texture
    create_texture = function(engine, size, func) end,

    ---@param center point
    ---@param texture texture
    ---@return sprite
    create_sprite = function(engine, center, texture) end,

    ---@param slotDefs slot_def[]
    ---@return slots
    create_slots = function(engine, slotDefs) end,

    ---@param dieDefs die_def[]
    create_dice = function(engine, dieDefs) end,

    roll_dice = function(engine) end,
}
