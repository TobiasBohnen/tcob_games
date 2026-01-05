-- Copyright (c) 2026 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local DURATION       = 5500
local SEGMENT_LENGTH = 300

local gfx            = require('ps_gfx')
local sfx            = require('ps_sfx')

local game           = {
    sockets         = {}, ---@type { [string]: socket }

    textures        = {
    },

    sounds          = {
    },

    track           = {
        0, 0,
        0.5, -0.5, 0.5,
        0, 0,
        -0.8, -0.8, -0.8,
        0, 0,
        0.3, 0.3,
        -0.3, -0.3,
        0, 0, 0,
        1.0, 1.0, 1.0,
        0.5,
        0, 0,
        -0.5, -0.5,
        0, 0, 0,
        0.7, 0.7,
        -0.7, -0.7,
        0, 0, 0
    },
    trackIndex      = 1,
    segmentProgress = 0,
}

---@param engine engine
function game:on_setup(engine)
    gfx.create_background(engine, 0, 0, 1)
    gfx.create_textures(self, engine)
    engine:create_sounds(sfx.get_sounds(self, engine))
end

local x = 0

---@param engine engine
function game:on_turn_start(engine)
    x = x % 4 + 1
end

function game:get_curve(speed)
    self.segmentProgress = self.segmentProgress + speed

    if self.segmentProgress >= SEGMENT_LENGTH then
        self.segmentProgress = self.segmentProgress - SEGMENT_LENGTH
        self.trackIndex = (self.trackIndex % #self.track) + 1
    end

    local currentCurve = self.track[self.trackIndex]
    local nextCurve    = self.track[(self.trackIndex % #self.track) + 1]

    local t            = self.segmentProgress / SEGMENT_LENGTH
    return currentCurve + (nextCurve - currentCurve) * t
end

---@param engine engine
---@param deltaTime number
function game:on_turn_update(engine, deltaTime, turnTime)
    if turnTime >= DURATION then return GameStatus.TurnEnded end

    gfx.create_background(engine, self:get_curve(4), self.segmentProgress / SEGMENT_LENGTH, x)
    --  if self.ship.health == 0 then return GameStatus.GameOver end
    return GameStatus.Running
end

---@param engine engine
---@param spriteA sprite
---@param spriteB sprite
function game:on_collision(engine, spriteA, spriteB)
    local a, b = spriteA.owner, spriteB.owner
    if a.collide then a:collide(b) end
    if b.collide then b:collide(a) end
end

---@param engine engine
function game:on_turn_finish(engine)

end

---@param engine engine
function game:on_teardown(engine)
    gfx.draw_game_over(engine.dmd, self)
end

---@param engine engine
function game:on_draw_dmd(engine)
    gfx.draw_dmd(engine.dmd, self)
end

------
------


return game
