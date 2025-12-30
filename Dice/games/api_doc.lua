-- Copyright (c) 2025 Tobias Bohnen
-- MIT License

---@meta

--------------------------------
-- Basic Types
--------------------------------

---@alias texture integer
---@alias sound integer
---@alias color integer

---@class point
---@field x number
---@field y number

---@class size
---@field width number
---@field height number

---@class rect
---@field x number
---@field y number
---@field width number
---@field height number

---@class socket_init
---@description Defines the validation rules for dice placement.
---@field values? number[] The values to compare against. If nil or empty, any die value is accepted.
---@field colors? color[] The color indices to compare against. If nil or empty, any die color is accepted.

---@class sprite_owner
---@field spriteInit? sprite_init

---@class sprite_init
---@description The initialization table used to define a new sprite's properties and behavior.
---@field texture? integer The ID of the texture to use from the sprite atlas.
---@field position? point The world position of the sprite.
---@field collidable? boolean Whether the sprite should trigger collision events.
---@field wrappable? boolean Whether the sprite wraps around the screen edges.

---@alias value_category_str
---| '"None"'
---| '"OnePair"'
---| '"TwoPair"'
---| '"ThreeOfAKind"'
---| '"FullHouse"'
---| '"Straight"'
---| '"FourOfAKind"'
---| '"FiveOfAKind"'

---@alias color_category_str
---| '"None"'
---| '"Flush"'
---| '"Rainbow"'

---@class hand
---@description The calculated result of the dice currently in the provided sockets.
---@field value value_category_str The identified value pattern (e.g., "FullHouse").
---@field color color_category_str The identified color pattern (e.g., "Flush").
---@field sockets socket[] The subset of sockets that actually form the hand (e.g., the 3 sockets in a ThreeOfAKind).

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

---@enum socket_state
SocketState = {
    Idle = 0,
    Hover = 1,
    Accept = 2,
    Reject = 3
}

---@enum rotation
Rot = {
    R0   = 0,
    R90  = 1,
    R180 = 2,
    R270 = 3,
}

--------------------------------
-- Game Module
--------------------------------

---@class game_module
---@
---@field on_setup fun(self: game_module, engine: engine)
---@field on_teardown fun(self: game_module, engine: engine)
---@
---@field on_turn_start fun(self: game_module, engine: engine)
---@
---@field on_turn_update fun(self: game_module, deltaTime: number, turnTime: number): game_status
---@
---@field on_turn_finish fun(self: game_module, engine: engine)
---@
---@field on_collision fun(self: game_module, engine: engine, spriteA: sprite, spriteB: sprite)
---@
---@field draw_dmd fun(self: game_module, engine: engine)

--------------------------------
-- Sprite
--------------------------------

---@class sprite
---@description A graphical object managed by the game engine.
---@field position point The world position of the sprite.
---@field size size The width and height of the sprite in world units.
---@field bounds rect The bounding rectangle of the sprite (Position + Size). @readonly
---@field owner table The Lua table defining this sprite's behavior. @readonly
---@field texture integer The ID of the texture from the packed atlas.

--------------------------------
-- Socket
--------------------------------

---@class socket
---@description A logical container for dice, mapped to the DMD coordinate system.
---@field position point The integer coordinate on the DMD grid.
---@field is_empty boolean Returns true if no die is currently placed in this socket. @readonly
---@field state socket_state The current interaction state (e.g., Idle, Accept, Hover). @readonly
---@field die_value integer The value (1-6) of the die in the socket, or 0 if empty. @readonly

--------------------------------
-- Die
--------------------------------

---@class die
---@field value integer @readonly

--------------------------------
-- DMD
--------------------------------
---@class dmd
---@description Interface for drawing to the Dot Matrix Display. All color values should be between 0 and 15.
local dmd = {}

---Copies a raw string of dot data into the specified rectangle.
---@param rect rect The destination area on the DMD.
---@param dots string A string where each character represents a pixel/dot.
function dmd:blit(rect, dots) end

---Clears the entire DMD, setting all pixels to 0 (Black).
function dmd:clear() end

---Draws a line between two points.
---@param start point The starting coordinate {x, y}.
---@param end_point point The ending coordinate {x, y}.
---@param color color The palette index.
function dmd:line(start, end_point, color) end

---Draws a circle centered at a specific point.
---@param center point The center coordinate {x, y}.
---@param radius integer The distance from the center to the edge.
---@param color color The palette index.
---@param fill boolean Whether to draw a solid circle or just the outline.
function dmd:circle(center, radius, color, fill) end

---Draws a rectangle based on a rect object.
---@param rect rect The position and size of the rectangle.
---@param color color The palette index.
---@param fill boolean Whether to draw a solid rectangle or just the outline.
function dmd:rect(rect, color, fill) end

---Prints text at the specified position.
---@param pos point The top-left coordinate for the text.
---@param text string The string to display.
---@param color color The palette index.
function dmd:print(pos, text, color) end

function dmd:socket(socket) end

--------------------------------
-- SCREEN
--------------------------------
---@class screen
---@description Interface for drawing to the screen. All color values should be between 0 and 15.
local screen = {}

---Copies a raw string of dot data into the specified rectangle.
---@param rect rect The destination area on the screen.
---@param dots string A string where each character represents a pixel/dot.
function screen:blit(rect, dots) end

---Clears the entire DMD, setting all pixels to transparent.
function screen:clear() end

---Draws a line between two points.
---@param start point The starting coordinate {x, y}.
---@param end_point point The ending coordinate {x, y}.
---@param color color The palette index.
function screen:line(start, end_point, color) end

---Draws a circle centered at a specific point.
---@param center point The center coordinate {x, y}.
---@param radius integer The distance from the center to the edge.
---@param color color The palette index.
---@param fill boolean Whether to draw a solid circle or just the outline.
function screen:circle(center, radius, color, fill) end

---Draws a rectangle based on a rect object.
---@param rect rect The position and size of the rectangle.
---@param color color The palette index.
---@param fill boolean Whether to draw a solid rectangle or just the outline.
function screen:rect(rect, color, fill) end

--------------------------------
-- Engine
--------------------------------

---@class engine
---@description The main interface between the Lua script and the game engine hardware/state.
---@field dmd dmd Access to the Dot Matrix Display drawing functions. @readonly
---@field fg screen Access to the foreground drawing functions. @readonly
---@field bg screen Access to the background drawing functions. @readonly
---@field ssd string The string value displayed on the Seven-Segment Display.
local engine = {}

---@section Asset Initialization

---Initializes and packs textures into the sprite atlas.
---@param texMap table<integer, table> Map of IDs to texture definitions.
function engine:create_textures(texMap) end

---Initializes the sound buffer cache.
---@param soundMap table<integer, table> Map of IDs to sound wave data.
function engine:create_sounds(soundMap) end

---@section Utilities

---Generates a random floating point number.
---@param min number Minimum value (inclusive).
---@param max number Maximum value (inclusive).
---@return number
function engine:rnd(min, max) end

---Generates a random integer number.
---@param min integer Minimum value (inclusive).
---@param max integer Maximum value (inclusive).
---@return integer
function engine:irnd(min, max) end

---Prints a message to the engine's log.
---@param str string The message to log.
function engine:log(str) end

---@section Object Management

---Creates a new sprite and adds it to the game world.
---@param owner sprite_owner The table defining the sprite's properties and behavior.
---@return sprite
function engine:create_sprite(owner) end

---Removes a sprite from the game world.
---@param sprite sprite The sprite instance to remove.
function engine:remove_sprite(sprite) end

---Creates a new socket for dice interaction.
---@param init socket_init The table defining the socket's properties and behavior.
---@return socket
function engine:create_socket(init) end

---Removes a socket from the game world.
---@param socket socket The socket instance to remove.
function engine:remove_socket(socket) end

---Analyzes the dice currently held in the provided sockets to determine the "hand".
---@param sockets table<string, socket> A table of sockets to check.
---@return hand
function engine:get_hand(sockets) end

---@section Game State

---Adds points to the player's current score.
---@param score integer The amount of points to add.
function engine:give_score(score) end

---Plays a sound effect by its ID.
---@param id integer The ID of the sound defined in create_sounds.
function engine:play_sound(id) end
