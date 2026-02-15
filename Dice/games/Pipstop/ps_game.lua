-- Copyright (c) 2026 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local DURATION       = 2500
local SEGMENT_LENGTH = 300
local THEMES         = { "day", "dusk", "night", "fall", "winter", "desert", }
local CAR_Y          = 140
local FUEL_PER_MS    = 0.04

local gfx            = require('ps_gfx')
local sfx            = require('ps_sfx')
local events         = require('ps_events')

local game           = {
    car        = {},

    opponents  = {},

    track      = {
        segments = {},
        themes = {},

        currentIndex = 1,
        currentTheme = 1,
        currentCurve = 0,

        segmentProgress = 0,
    },

    eventQueue = {}, ---@type event_base[]

    sockets    = {
        wheel = nil, ---@type socket
    },
}

---@param engine engine
function game:on_setup(engine)
    gfx.create_textures(self, engine)
    sfx.create_sounds(self, engine)

    self:create_track(engine)
    self:create_car(engine)

    self:update_background(engine)

    self:queue_event(engine, events:get_start(self, engine))
    self:queue_event(engine, events:get_next(self, engine))
    self:queue_event(engine, events:get_next(self, engine))
    self.sockets.wheel = socket.new { colors = { Palette.Red, Palette.White } }

    engine:update_hud()
end

---@param engine engine
function game:on_turn_start(engine)
    for i = #self.eventQueue, 1, -1 do
        local event = self.eventQueue[i]
        if event.finished then
            table.remove(self.eventQueue, i)
        else
            event:turn_start()
        end
    end
    engine:update_hud()
end

---@param engine engine
---@param deltaTime number
function game:on_turn_update(engine, deltaTime, turnTime)
    if turnTime > DURATION then return GameStatus.Waiting end

    local car = self.car
    car:update(deltaTime, turnTime)
    self:update_track(car.speed.current / 20)

    for i = #self.opponents, 1, -1 do
        local opponent = self.opponents[i]
        opponent:update(turnTime)

        if opponent.markedForDeath then
            opponent.sprite:remove()
            table.remove(self.opponents, i)
        end
    end

    for i = #self.eventQueue, 1, -1 do
        self.eventQueue[i]:update(deltaTime, turnTime)
    end

    self:update_background(engine)
    engine:update_hud()

    sprite.y_sort()

    if self.car.fuel == 0 then return GameStatus.GameOver end
    return GameStatus.Running
end

---@param engine engine
function game:on_turn_finish(engine)
    self:update_background(engine)
    engine:update_hud()
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
---@param hud tex
function game:on_draw_hud(engine, hud)
    if engine.is_game_over then
        gf.draw_game_over(hud)
    else
        gfx.draw_hud(hud, self)
    end
end

------
------

---@param engine engine
function game:create_car(engine)
    local car = {
        speed         = {
            start   = nil,
            current = 0,
            target  = 0
        },

        lateral       = 0,
        lateralTarget = 0,

        handling      = 100,
        fuel          = 100,

        type          = "car",

        spriteInit    = {
            position  = { x = (ScreenSize.width - gfx.sizes.car.width) / 2, y = CAR_Y },
            texture   = gfx.textures.car,
            wrappable = false
        },


        update           = function(car, deltaTime, turnTime)
            local progress    = math.min(1.0, turnTime / DURATION)
            local speedFactor = car.speed.current / 100

            -- speed
            if car.speed.current ~= car.speed.target and car.speed.start then
                car.speed.current = car.speed.start + (car.speed.target - car.speed.start) * progress
            end
            if progress >= 1.0 then
                car.speed.start = nil
            end

            -- fuel
            if car.fuel > 0 then
                local fuelLoss = speedFactor * FUEL_PER_MS
                car.fuel = math.max(0, car.fuel - fuelLoss)
            else
                car.speed.target = 0
            end


            local halfRoadWidth = ScreenSize.width / 6
            local pos           = car.sprite.position
            local newX          = math.max(halfRoadWidth,
                math.min(ScreenSize.width - halfRoadWidth - gfx.sizes.car.width,
                    pos.x - self.track.currentCurve * speedFactor))

            car.sprite.position = { x = newX, y = pos.y }
            car:play_sound()

            engine:give_score(math.floor(speedFactor * 5))
        end,

        play_sound       = function(car)
            if car.speed.current > 0 then
                local sndIdx = math.min(math.floor(car.speed.current / 10) + 1, 10)
                local sound  = "car_speed_" .. sndIdx
                engine:play_sound(sfx.sounds[sound], 0, false)
            end
        end,

        set_target_speed = function(car, speed)
            car.speed.start  = car.speed.current
            car.speed.target = math.min(speed, 100)
        end
    }
    car.sprite = sprite.new(car)

    self.car = car
end

local LANES = { 0.25, 0.5, 0.75 }

---@param engine engine
function game:try_spawn_opponent(engine)
    local availableLanes = {}

    for _, laneX in ipairs(LANES) do
        local laneOccupied = false

        for _, opp in ipairs(self.opponents) do
            if opp.lane == laneX then
                laneOccupied = true
                break
            end
        end

        if not laneOccupied then
            availableLanes[#availableLanes + 1] = laneX
        end
    end

    if #availableLanes == 0 then return end

    self:create_opponent(engine, availableLanes[engine:irnd(1, #availableLanes)])
end

---@param engine engine
function game:create_opponent(engine, lane)
    local opponent = {
        distance       = 0,
        speed          = self.car.speed.current - engine:irnd(10, 30),
        lane           = lane,

        markedForDeath = false,

        color          = engine:irnd(1, #gfx.textures.opp_car),

        spriteInit     = {},

        type           = "opponent",

        update         = function(opponent, turnTime)
            local relativeSpeed = self.car.speed.current - opponent.speed
            opponent.distance   = opponent.distance + (relativeSpeed / 100) * 0.1

            if opponent.distance < -0.5 or opponent.distance > 20 then
                opponent.markedForDeath = true
                return
            end

            local scaleIndex         = math.max(1, math.min(10, math.floor(opponent.distance) + 1))
            opponent.sprite.texture  = gfx.textures.opp_car[opponent.color][scaleIndex]

            local baseY              = gfx.horizonHeight + (opponent.distance * (CAR_Y - gfx.horizonHeight) / 10)
            local rd                 = gfx.get_road_at_y(baseY)
            opponent.sprite.position = { x = rd.leftEdge + (lane * rd.roadWidth) - opponent.sprite.size.width / 2, y = baseY }
        end,

        collide        = function(opponent, b)

        end
    }
    opponent.spriteInit = {
        texture   = gfx.textures.opp_car[opponent.color][1],
        wrappable = false
    }

    opponent.sprite = sprite.new(opponent)

    self.opponents[#self.opponents + 1] = opponent
end

---@param engine engine
function game:create_track(engine)
    local segments = {}
    segments[1]    = 0
    local themes   = {}
    themes[1]      = 1

    for i = 1, 30 do
        local theme = themes[#themes]
        if engine:irnd(1, 5) == 1 then
            repeat
                theme = engine:irnd(1, #THEMES)
            until theme ~= themes[#themes]
        end

        if i % 3 == 0 then
            segments[#segments + 1] = 0
            segments[#segments + 1] = 0

            themes[#themes + 1] = theme
            themes[#themes + 1] = theme
        else
            segments[#segments + 1] = engine:irnd(-10, 10) * 0.1

            themes[#themes + 1] = theme
        end
    end

    self.track.segments = segments
    self.track.themes   = themes
    assert(#segments == #themes)
end

function game:update_track(speed)
    self.track.segmentProgress = self.track.segmentProgress + speed

    if self.track.segmentProgress >= SEGMENT_LENGTH then
        self.track.segmentProgress = self.track.segmentProgress - SEGMENT_LENGTH
        self.track.currentIndex    = (self.track.currentIndex % #self.track.segments) + 1
        self.track.currentTheme    = self.track.themes[self.track.currentIndex]
    end

    local currentCurve      = self.track.segments[self.track.currentIndex]
    local nextCurve         = self.track.segments[(self.track.currentIndex % #self.track.segments) + 1]

    local t                 = self.track.segmentProgress / SEGMENT_LENGTH
    self.track.currentCurve = currentCurve + (nextCurve - currentCurve) * t
end

---@param engine engine
function game:queue_event(engine, event)
    self.eventQueue[#self.eventQueue + 1] = event
    event:init(self, engine)
end

---@param engine engine
function game:update_background(engine)
    gfx.create_background(engine, self.track.currentCurve, (self.car.speed.current / 100), self.track.segmentProgress / SEGMENT_LENGTH, THEMES[self.track.currentTheme])
end

return game
