-- Copyright (c) 2026 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local DURATION       = 2500
local SEGMENT_LENGTH = 300
local BIOMES         = { "day", "dusk", "night", "fall", "winter", "desert", }

local gfx            = require('ps_gfx')
local sfx            = require('ps_sfx')
local events         = require('ps_events')

local game           = {
    car             = {},

    currentBiome    = 1,

    track           = {},
    trackIndex      = 1,
    segmentProgress = 0,

    eventQueue      = {}, ---@type event_base[]

    sockets         = {
        speed = nil, ---@type socket
        control = nil, ---@type socket
    },
}

---@param engine engine
function game:on_setup(engine)
    self:create_track(engine)

    self:update_background(engine, 0)
    gfx.create_textures(self, engine)
    engine:create_sounds(sfx.get_sounds(self, engine))

    self:create_car(engine)

    self:queue_event(engine, events:get_start(self, engine))
end

---@param engine engine
function game:on_turn_start(engine)
    for i = #self.eventQueue, 1, -1 do
        local event = self.eventQueue[i]
        if event.finished then
            table.remove(self.eventQueue, i)
        else
            event:turn_start(self, engine)
        end
    end

    if self.car.speed > 0 then
        engine:play_sound(sfx.sounds.car)
    end
end

---@param engine engine
---@param deltaTime number
function game:on_turn_update(engine, deltaTime, turnTime)
    if turnTime >= DURATION then return GameStatus.TurnEnded end
    local car         = self.car
    local carStopped  = car.speed == 0

    local curveAmount = self:get_curve(car.speed)
    car:update(turnTime, curveAmount)

    if car.speed > 0 and carStopped then
        engine:play_sound(sfx.sounds.car)
        carStopped = false
    end

    engine:give_score(math.floor(car.speed))

    for i = #self.eventQueue, 1, -1 do
        self.eventQueue[i]:update(self, engine, deltaTime, turnTime)
    end

    self:update_background(engine, curveAmount)

    if self.car.health == 0 then return GameStatus.GameOver end
    return GameStatus.Running
end

---@param engine engine
function game:on_turn_finish(engine)
    -- change biome
    if self.car.speed > 0 then --TODO: use distance instead
        if engine:irnd(1, 5) == 1 then
            local old = self.currentBiome
            repeat
                self.currentBiome = engine:irnd(1, #BIOMES)
            until self.currentBiome ~= old
        end
    end

    self:update_background(engine, self:get_curve(0))
end

---@param engine engine
function game:on_teardown(engine)
    gfx.draw_game_over(engine.dmd, self)
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
function game:on_draw_dmd(engine)
    gfx.draw_dmd(engine.dmd, self)
end

------
------

---@param engine engine
function game:create_track(engine)
    local track = {}
    track[1] = 0
    for i = 1, 30 do
        if i % 3 == 0 then
            track[#track + 1] = 0
            track[#track + 1] = 0
        else
            track[#track + 1] = engine:irnd(-10, 10) * 0.1
        end
    end

    self.track = track
end

---@param engine engine
function game:queue_event(engine, event)
    self.eventQueue[#self.eventQueue + 1] = event
    event:init(self, engine)
end

---@param engine engine
function game:update_background(engine, curveAmount)
    gfx.create_background(engine, curveAmount, self.segmentProgress / SEGMENT_LENGTH, BIOMES[self.currentBiome])
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
function game:create_car(engine)
    local car = {
        speed       = 0,
        speedTarget = 0,
        lateral     = 0,

        handling    = 10,
        health      = 10,

        spriteInit  = {
            position  = { x = 120, y = 140 },
            texture   = gfx.textures.car.straight,
            wrappable = false
        },

        update      = function(car, turnTime, curveAmount)
            local factor        = turnTime / DURATION
            car.speed           = car.speed + ((car.speedTarget - car.speed) * factor)

            local halfRoadWidth = engine.screenSize.width / 6
            local pos           = car.sprite.position
            local newX          = math.max(halfRoadWidth, math.min(engine.screenSize.width - halfRoadWidth - gfx.sizes.car.width, pos.x - curveAmount))

            car.sprite.position = { x = newX, y = pos.y }
        end
    }
    car.sprite = engine:create_sprite(car)

    self.car = car
end

return game
