-- Copyright (c) 2026 Tobias Bohnen
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
---@field position? point The position of the sprite.
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

---@class blit_settings
---@field transparent? integer The palette index to treat as transparent.
---@field rotation? rotation Rotation angle in degrees: 0, 90, 180, or 270.
---@field flip_h? boolean Flip horizontally (mirror left-right).
---@field flip_v? boolean Flip vertically (mirror top-bottom).

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

---Analyzes the dice currently held in the provided sockets to determine the hand.
---@param sockets socket[] A table of sockets to check.
---@return hand
function get_hand(sockets) end

---Returns the arithmetic sum of all die face values.
---@param sockets socket[] A table of sockets to check.
---@return integer
function get_sum(sockets) end

---Returns the weighted score by combining the hand's base value and the dice sum.
---@param sockets socket[] A table of sockets to check.
---@param baseHandValue? integer
---@return integer
function get_value(sockets, baseHandValue) end

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
---@field on_draw_dmd fun(self: game_module, engine: engine)

--------------------------------
-- Sprite
--------------------------------

---@class sprite
---@description A graphical object managed by the game engine.
---@field position point The position of the sprite.
---@field size size The size of the sprite. @readonly
---@field center point The center of the sprite. @readonly
---@field owner table The Lua table defining this sprite's behavior. @readonly
---@field texture integer The ID of the texture from the packed atlas.

---Creates a new sprite and adds it to the game world.
---@param owner sprite_owner The table defining the sprite's properties and behavior.
---@return sprite
function sprite.new(owner) end

---Removes a sprite from the game world.
function sprite:remove() end

--------------------------------
-- Socket
--------------------------------

---@class socket
---@description A logical container for dice, mapped to the DMD coordinate system.
---@field position point The integer coordinate on the DMD grid.
---@field is_empty boolean Returns true if no die is currently placed in this socket. @readonly
---@field state socket_state The current interaction state (e.g., Idle, Accept, Hover). @readonly
---@field die_value integer The value (1-6) of the die in the socket, or 0 if empty. @readonly

---Creates a new socket for dice interaction.
---@param init socket_init The table defining the socket's properties and behavior.
---@return socket
function socket.new(init) end

---Removes a socket from the game world.
function socket:remove() end

--------------------------------
-- TEX
--------------------------------
---@class tex
---@description Interface for drawing a textures. All color values should be between 0 and 15.
---@field bounds rect The bounding rectangle of the texture (Position + Size). @readonly
---@field size size The width and height of the texture in pixels.
local tex = {}

---Copies a raw string of dot data into the specified rectangle.
---@param rect rect The destination area on the texture.
---@param data string A string where each character represents a pixel.
---@param blitSettings? blit_settings Optional table controlling how the blit operation is performed.
function tex:blit(rect, data, blitSettings) end

---Clears the entire texture, setting all pixels to transparent/black.
---@param rect? rect The destination area on the texture.
function tex:clear(rect) end

---Draws a line between two points.
---@param startPoint point The starting coordinate {x, y}.
---@param endPoint point The ending coordinate {x, y}.
---@param color color The palette index.
function tex:line(startPoint, endPoint, color) end

function tex:pixel(point, color) end

---Draws a circle centered at a specific point.
---@param center point The center coordinate {x, y}.
---@param radius integer The distance from the center to the edge.
---@param color color The palette index.
---@param fill boolean Whether to draw a solid circle or just the outline.
function tex:circle(center, radius, color, fill) end

---Draws a rectangle based on a rect object.
---@param rect rect The position and size of the rectangle.
---@param color color The palette index.
---@param fill boolean Whether to draw a solid rectangle or just the outline.
function tex:rect(rect, color, fill) end

---Prints text at the specified position.
---@param pos point The top-left coordinate for the text.
---@param text string The string to display.
---@param color color The palette index.
---@param font? string
function tex:print(pos, text, color, font) end

function tex:socket(socket) end

--------------------------------
-- Engine
--------------------------------

---@class engine
---@description The main interface between the Lua script and the game engine hardware/state.
---@field screenSize size The size of the screen textures. @readonly
---@field dmd tex Access to the Dot Matrix Display drawing functions. @readonly
---@field fg tex Access to the foreground drawing functions. @readonly
---@field bg tex Access to the background drawing functions. @readonly
---@field spr tex Access to the sprite drawing functions. @readonly
---@field ssd string The string value displayed on the Seven-Segment Display.
local engine = {}

---@section Asset Initialization

---Defines a texture region from the sprite atlas.
---@param id integer Unique identifier for this texture.
---@param uv rect UV coordinates in pixel coordinates defining the texture region.
function engine:define_texture(id, uv) end

---Initializes the sound buffer cache.
---@param id integer Unique identifier for this sound.
---@param soundwave table
function engine:define_sound(id, soundwave) end

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

---@section Game State

---Adds points to the player's current score.
---@param score integer The amount of points to add.
function engine:give_score(score) end

---Plays a sound effect by its ID.
---@param id integer The ID of the sound defined in define_sound.
---@param channel? integer
---@param playNow? boolean
function engine:play_sound(id, channel, playNow) end
