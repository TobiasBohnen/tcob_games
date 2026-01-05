-- Copyright (c) 2026 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local DURATION       = 5500
local SEGMENT_LENGTH = 300
local THEMES         = { "normal", "dusk", "fall", "winter", "night" }

local gfx            = require('ps_gfx')
local sfx            = require('ps_sfx')


local game = {
    sockets         = {}, ---@type { [string]: socket }

    textures        = {
    },

    sounds          = {
    },

    track           = {},
    trackIndex      = 1,
    trackTheme      = 1,
    segmentProgress = 0,

    speed           = 4
}

---@param engine engine
function game:on_setup(engine)
    self.track[1] = 0
    for i = 1, 30 do
        if i % 3 == 0 then
            self.track[#self.track + 1] = 0
            self.track[#self.track + 1] = 0
        else
            self.track[#self.track + 1] = engine:irnd(-10, 10) * 0.1
        end
    end

    self:update_background(engine)
    gfx.create_textures(self, engine)
    engine:create_sounds(sfx.get_sounds(self, engine))
end

---@param engine engine
function game:on_turn_start(engine)
end

---@param engine engine
---@param deltaTime number
function game:on_turn_update(engine, deltaTime, turnTime)
    if turnTime >= DURATION then return GameStatus.TurnEnded end

    self:update_background(engine)
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
    if engine:irnd(1, 5) == 1 then
        local oldTheme = self.trackTheme
        repeat
            self.trackTheme = engine:irnd(1, #THEMES)
        until self.trackTheme ~= oldTheme
    end

    self:update_background(engine)
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

---@param engine engine
function game:update_background(engine)
    gfx.create_background(engine, self:get_curve(self.speed), self.segmentProgress / SEGMENT_LENGTH, THEMES[self.trackTheme])
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

return game
