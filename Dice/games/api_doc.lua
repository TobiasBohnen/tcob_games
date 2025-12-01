-- Copyright (c) 2025 Tobias Bohnen
-- MIT License

---@meta

--------------------------------
-- Basic Types
--------------------------------

---@alias texture integer
---@alias sound integer

---@class color
---@field r integer
---@field g integer
---@field b integer
---@field a? integer

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
---@field op? string

---@class die_face
---@field color color
---@field values integer[]

---@class sprite_owner
---@field texture texture
---@field collidable? boolean
---@field wrappable? boolean

---@class hand
---@field value string
---@field color string
---@field slots slot[]

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
Palette = {}

ScreenSize = { width = 0, height = 0 } ---@type size
DMDSize = { width = 0, height = 0 } ---@type size

---@enum game_status
GameStatus = {
    Running = 0,
    TurnEnded = 1,
    GameOver = 2
}

--------------------------------
-- Game Module
--------------------------------

---@class game_module
---@
---@field on_setup fun(self: game_module, engine: engine)
---@field on_teardown fun(self: game_module, engine: engine)
---@
---@field update fun(self: game_module, deltaTime: number): game_status
---@
---@field can_start fun(self: game_module, engine: engine): boolean
---@field start fun(self: game_module, engine: engine)
---@
---@field finish fun(self: game_module, engine: engine)
---@
---@field on_collision fun(self: game_module, engine: engine, spriteA: sprite, spriteB: sprite)
---@field on_die_changed fun(self: game_module, engine: engine, slot: slot)

--------------------------------
-- Sprite
--------------------------------

---@class sprite
---@field position point
---@field size size
---@field bounds rect
---@field rotation number
---@field scale size
---@field owner table @readonly
---@field texture texture

--------------------------------
-- Slot
--------------------------------

---@class slot
---@field position point
---@field isEmpty boolean @readonly
---@field dieValue integer @readonly
---@field owner table @readonly

--------------------------------
-- Die
--------------------------------

---@class die
---@

--------------------------------
-- DMD
--------------------------------

---@class dmd
---
---@field blit fun(self: dmd, rect: rect, dots: string)
---@field clear fun(self: dmd)

--------------------------------
-- SFX
--------------------------------

---@class sfx
---
---@field pickup_coin fun(self: sfx, seed: integer)
---@field laser_shoot fun(self: sfx, seed: integer)
---@field explosion fun(self: sfx, seed: integer)
---@field powerup fun(self: sfx, seed: integer)
---@field hit_hurt fun(self: sfx, seed: integer)
---@field jump fun(self: sfx, seed: integer)
---@field blip_select fun(self: sfx, seed: integer)
---@field random fun(self: sfx, seed: integer)

--------------------------------
-- Engine
--------------------------------

---@class engine
---
---@field dmd dmd
---@field sfx sfx
---
---@field create_background fun(self: engine, string: string)
---@field create_textures fun(self: engine, texMap: {[integer]: table})
---@field create_sounds fun(self: engine, soundMap: {[integer]: table})
---
---@field random fun(self: engine, min: number, max: number): number
---@field random_int fun(self: engine, min: integer, max: integer): integer
---
---@field log fun(str: string)
---
---@field create_sprite fun(self: engine, owner: sprite_owner): sprite
---@field remove_sprite fun(self: engine, sprite: sprite)
---
---@field create_slot fun(self: engine, owner: slot_owner): slot
---@field reset_slots fun(self: engine, slots: { [string]: slot })
---@field get_hand fun(self: engine, slots: { [string]: slot }): hand
---
---@field create_dice fun(self: engine, count: integer, dieFaces: die_face[])
---@field roll_dice fun(self: engine)
---
---@field give_score fun(self: engine, score: integer)
---
---@field play_sound fun(self: engine, id: integer)
