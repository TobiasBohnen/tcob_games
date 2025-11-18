-- Copyright (c) 2025 Tobias Bohnen
-- MIT License

---@meta

--------------------------------
-- Game Module
--------------------------------

---@class game_module
---@field draw_background fun(self: game_module, engine: engine, canvas: canvas, canvasSize: size)
---@field get_textures fun(self: game_module, engine: engine): table<texture, table>
---@field on_setup fun(self: game_module, engine: engine)
---@field can_run fun(self: game_module, engine: engine): boolean
---@field on_run fun(self: game_module, deltaTime: number)
---@field can_start fun(self: game_module, engine: engine): boolean
---@field on_start fun(self: game_module, engine: engine)
---@field on_finish fun(self: game_module, engine: engine)
---@field on_collision fun(self: game_module, engine: engine, spriteA: sprite, spriteB: sprite)
local game_module = {}

--------------------------------
-- Basic Types
--------------------------------

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
---@field texture texture
---@field collisionEnabled boolean

--------------------------------
-- Canvas
--------------------------------

---@class canvas
local canvas = {
    ---@param self canvas
    begin_path = function(self) end,

    ---@param self canvas
    ---@param color string
    clear = function(self, color) end,

    ---@param self canvas
    ---@param path string
    path_2d = function(self, path) end,

    ---@param self canvas
    ---@param rect rect
    rect = function(self, rect) end,

    ---@param self canvas
    ---@param color string
    stroke_color = function(self, color) end,

    ---@param self canvas
    ---@param width number
    stroke_width = function(self, width) end,

    ---@param self canvas
    stroke = function(self) end,

    ---@param self canvas
    ---@param color string
    fill_color = function(self, color) end,

    ---@param self canvas
    ---@param enforceWinding boolean|nil
    fill = function(self, enforceWinding) end,
}

--------------------------------
-- Sprite
--------------------------------

---@class sprite
---@field Position point
---@field Size size
---@field Bounds rect
---@field Rotation number
---@field Scale size
---@field Owner table

--------------------------------
-- Engine
--------------------------------

---@class engine
local engine = {

    ---@param self engine
    ---@param bounds rect
    set_dice_area = function(self, bounds) end,

    ---@param self engine
    ---@param min number
    ---@param max number
    ---@return number
    random = function(self, min, max) end,

    ---@param self engine
    ---@param min integer
    ---@param max integer
    ---@return integer
    randomInt = function(self, min, max) end,

    ---@param str string
    log = function(str) end,

    ---@param self engine
    ---@param def spriteDef
    ---@return sprite
    create_sprite = function(self, def) end,

    ---@param self engine
    ---@param spritePtr sprite
    remove_sprite = function(self, spritePtr) end,

    ---@param self engine
    ---@param pos point
    ---@param slotFace slot_face
    create_slot = function(self, pos, slotFace) end,

    ---@param self engine
    ---@param count integer
    ---@param dieFaces die_face[]
    create_dice = function(self, count, dieFaces) end,

    ---@param self engine
    ---@param slots table
    release_dice = function(self, slots) end,

    ---@param self engine
    roll_dice = function(self) end,

    ---@return boolean
    are_slots_filled = function(self) end,

    ---@return boolean
    are_slots_locked = function(self) end,

    ---@param idx integer
    ---@return boolean
    is_slot_empty = function(self, idx) end,

    ---@param idx integer
    ---@return integer
    slot_die_value = function(self, idx) end,
}
