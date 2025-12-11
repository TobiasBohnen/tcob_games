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
---@field BlueGray color
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

---@enum slot_state
SlotState = {
    Idle = 0,
    Hover = 1,
    Accept = 2,
    Reject = 3
}

--------------------------------
-- Game Module
--------------------------------

---@class game_module
---@
---@field on_setup fun(self: game_module, engine: engine)
---@field on_teardown fun(self: game_module, engine: engine)
---@
---@field can_start_turn fun(self: game_module, engine: engine): boolean
---@field on_turn_start fun(self: game_module, engine: engine)
---@
---@field on_turn_update fun(self: game_module, deltaTime: number): game_status
---@
---@field on_turn_finish fun(self: game_module, engine: engine)
---@
---@field on_collision fun(self: game_module, engine: engine, spriteA: sprite, spriteB: sprite)
---@field on_die_change fun(self: game_module, engine: engine, slot: slot)
---@field on_die_motion fun(self: game_module, engine: engine)

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
---@field owner table @readonly
---@field is_empty boolean @readonly
---@field state slot_state @readonly
---@field die_value integer @readonly
---@field position point

--------------------------------
-- Die
--------------------------------

---@class die
---@field value integer @readonly

--------------------------------
-- DMD
--------------------------------

---@class dmd
---
---@field blit fun(self: dmd, rect: rect, dots: string)
---@field clear fun(self: dmd)
---@field print fun(self: dmd, pos: point, text: string, color: color|integer)

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
---@field background integer
---@field ssd_value string
---
---@field create_backgrounds fun(self: engine, bgMap: {[integer]: table})
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
