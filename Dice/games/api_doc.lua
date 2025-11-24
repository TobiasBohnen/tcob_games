-- Copyright (c) 2025 Tobias Bohnen
-- MIT License

---@meta

--------------------------------
-- Game Module
--------------------------------

---@class game_module
---@
---@field draw_background fun(self: game_module, engine: engine, canvas: canvas, canvasSize: size)
---@field get_textures fun(self: game_module, engine: engine): table<texture, table>
---@
---@field on_setup fun(self: game_module, engine: engine)
---@
---@field on_run fun(self: game_module, deltaTime: number)
---@
---@field can_start fun(self: game_module, engine: engine): boolean
---@field on_start fun(self: game_module, engine: engine)
---@
---@field on_finish fun(self: game_module, engine: engine)
---@
---@field on_collision fun(self: game_module, engine: engine, spriteA: sprite, spriteB: sprite)
---@field on_slot_die_changed fun(self: game_module, engine: engine, slot: slot)
local game_module = {}

--------------------------------
-- Basic Types
--------------------------------

---@alias texture integer

---@class color
---@field r integer
---@field g integer
---@field b integer
---@field a integer|nil

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

---@class slot_owner
---@field color color
---@field value number
---@field op string|nil

---@class die_face
---@field color color
---@field values integer[]

---@class sprite_owner
---@field texture texture
---@field collisionEnabled boolean

---@class palette
---@field Transparent color
---@field Black color
---@field Gray color
---@field White color
---@field Red color
---@field Pink color
---@field DarkBrown color
---@field Brown color
---@field Orange color
---@field Yellow color
---@field DarkGreen color
---@field Green color
---@field LightGreen color
---@field DarkBlue color
---@field Blue color
---@field LightBlue color
palette = {}

--------------------------------
-- Canvas
--------------------------------

---@class canvas
local canvas = {
    ---@param self canvas
    begin_path = function(self) end,

    ---@param self canvas
    ---@param color color
    clear = function(self, color) end,

    ---@param self canvas
    ---@param path string
    path_2d = function(self, path) end,

    ---@param self canvas
    ---@param rect rect
    rect = function(self, rect) end,

    ---@param self canvas
    ---@param color color
    stroke_color = function(self, color) end,

    ---@param self canvas
    ---@param width number
    stroke_width = function(self, width) end,

    ---@param self canvas
    stroke = function(self) end,

    ---@param self canvas
    ---@param color color
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
---@field Texture texture

--------------------------------
-- Slot
--------------------------------

---@class slot
---@field IsEmpty boolean
---@field DieValue integer

--------------------------------
-- Die
--------------------------------

---@class die
---@

--------------------------------
-- Engine
--------------------------------

---@class engine
local engine = {

    ---@param self engine
    ---@param min number
    ---@param max number
    ---@return number
    random = function(self, min, max) end,

    ---@param self engine
    ---@param min integer
    ---@param max integer
    ---@return integer
    random_int = function(self, min, max) end,

    ---@param str string
    log = function(str) end,

    ---@param self engine
    ---@param owner sprite_owner
    ---@return sprite
    create_sprite = function(self, owner) end,

    ---@param self engine
    ---@param sprite sprite
    remove_sprite = function(self, sprite) end,

    ---@param self engine
    ---@param owner slot_owner
    ---@return slot
    create_slot = function(self, owner) end,

    ---@param self engine
    ---@param slots { [string]: slot }
    reset_slots = function(self, slots) end,

    ---@param self engine
    ---@param count integer
    ---@param dieFaces die_face[]
    create_dice = function(self, count, dieFaces) end,

    ---@param self engine
    roll_dice = function(self) end,

    ---@param self engine
    ---@param rect rect
    ---@param dots string
    blit_dmd = function(self, rect, dots) end,

    clear_dmd = function(self) end

}
