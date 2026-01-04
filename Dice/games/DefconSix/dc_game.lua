-- Copyright (c) 2026 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local DURATION                    = 2500

local CITY_COUNT                  = 5
local MAX_CITY_DAMAGE             = 3
local MIN_CHANCE_TO_MIRV          = 0.2
local MAX_CHANCE_TO_MIRV          = 0.6
local MIRV_ZONE_TOP               = 5
local MIRV_ZONE_BOTTOM            = 80
local MAX_MIRV                    = 2
local MIN_MISSILE_SPEED           = 10
local MAX_MISSILE_SPEED           = 15

local BASE_ENERGY_RESERVE_RESTORE = 400
local MAX_ENERGY_RESERVE          = 2400
local CANNON_CHARGE_DURATION      = DURATION / 4 * 3
local MAX_CANNON_CHARGE           = 100
local MAX_HEAT                    = 100
local SHOT_HEAT_GAIN              = 10
local HEAT_LOSS_FACTOR            = 10
local BASE_HEAT_LOSS              = 10

local gfx                         = require('dc_gfx')
local sfx                         = require('dc_sfx')

local game                        = {
    cities           = {}, ---@type [city]

    cannons          = {}, ---@type cannons
    cannonTypes      = { "left", "right", "center" },

    missiles         = {}, ---@type [missile]

    sockets          = {
        left          = {}, ---@type { [string]: socket }
        right         = {}, ---@type { [string]: socket }
        center        = {}, ---@type { [string]: socket }
        energyRestore = nil, ---@type socket
    },

    textures         = {
        city = { undamaged = 0, light_damage = 1, heavy_damage = 2, destroyed = 3 }, ---@type { [string]: texture }
        cannon = { left = 10, right = 11, center = 12 }, ---@type { [string]: texture }
        missile = 20, ---@type texture
    },

    sounds           = {
        missileExplosion = 1, ---@type sound
        cityExplosion    = 2, ---@type sound
        cannon           = 3, ---@type sound
    },

    energyReserve    = 0,
    relEnergyReserve = 0,

    destroyedCities  = 0,
}

---@param engine engine
function game:on_setup(engine)
    gfx.create_background(engine)
    gfx.create_textures(self, engine)
    engine:create_sounds(sfx.get_sounds(self, engine))

    self:create_cannons(engine)
    for i = 1, CITY_COUNT do
        self:create_city(i, engine)
        self:try_spawn_missile(engine)
    end

    self.sockets.left.chargeRate   = engine:create_socket { colors = { Palette.White } }
    self.sockets.left.coolRate     = engine:create_socket { colors = { Palette.White } }
    self.sockets.right.chargeRate  = engine:create_socket { colors = { Palette.White } }
    self.sockets.right.coolRate    = engine:create_socket { colors = { Palette.White } }
    self.sockets.center.chargeRate = engine:create_socket { colors = { Palette.White } }
    self.sockets.center.coolRate   = engine:create_socket { colors = { Palette.White } }

    self.sockets.energyRestore     = engine:create_socket { colors = { Palette.White } }

    self:set_energy_reserve(engine, MAX_ENERGY_RESERVE)
end

---@param engine engine
function game:on_turn_start(engine)
    engine.fg:clear()

    for i = #self.missiles, 1, -1 do
        self.missiles[i]:turn_start()
    end

    self.cannons:turn_start()
end

---@param engine engine
---@param deltaTime number
---@param turnTime number
function game:on_turn_update(engine, deltaTime, turnTime)
    if turnTime >= DURATION then return GameStatus.TurnEnded end

    for i = #self.missiles, 1, -1 do
        self.missiles[i]:update(i, deltaTime, turnTime)
    end

    self.cannons:update(deltaTime)

    if self.destroyedCities >= #self.cities then return GameStatus.GameOver end
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
    self:set_energy_reserve(engine, self.energyReserve + self.sockets.energyRestore.die_value * 100 + BASE_ENERGY_RESERVE_RESTORE)
    self.cannons:turn_end()

    local count = engine:irnd(1, 3)
    for i = 1, count do
        self:try_spawn_missile(engine)
    end
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

function game:set_energy_reserve(engine, val)
    self.energyReserve    = math.min(val, MAX_ENERGY_RESERVE)
    self.relEnergyReserve = self.energyReserve / MAX_ENERGY_RESERVE
    gfx.draw_dmd(engine.dmd, self)
end

---@param engine engine
function game:create_cannons(engine)
    local screenSize = engine.screenSize

    local cannons    = { ---@class cannons
        left       = {},
        right      = {},
        center     = {},

        turn_start = function(cannons)
            for _, value in ipairs(self.cannonTypes) do
                local cannon      = cannons[value]
                cannon.shotsLeft  = self.sockets[value].chargeRate.die_value
                cannon.chargeRate = (100 * cannon.shotsLeft) / CANNON_CHARGE_DURATION
                cannon.coolRate   = self.sockets[value].coolRate.die_value * HEAT_LOSS_FACTOR
            end
        end,

        update     = function(cannons, deltaTime)
            local order        = { 1, 2, 3 }
            local i            = engine:irnd(1, 3)
            order[1], order[i] = order[i], order[1]

            for _, idx in ipairs(order) do
                cannons[self.cannonTypes[idx]]:gain_energy(deltaTime)
            end
        end,

        turn_end   = function(cannons)
            for _, value in ipairs(self.cannonTypes) do
                cannons[value]:cool()
            end
        end,
    }

    self.cannons     = cannons

    self:create_cannon(
        engine, "left",
        { x = 0, y = screenSize.height / 3 * 2 },
        { min = 0, max = screenSize.width / 3 },
        { x = 14, y = 3 })
    self:create_cannon(
        engine, "right",
        { x = screenSize.width - gfx.sizes.cannon.width, y = screenSize.height / 3 * 2 },
        { min = screenSize.width / 3 * 2, max = screenSize.width },
        { x = 1, y = 3 })
    self:create_cannon(
        engine, "center",
        { x = (screenSize.width - gfx.sizes.cannon.width) / 2, y = screenSize.height / 3 * 2 },
        { min = screenSize.width / 3, max = screenSize.width / 3 * 2 },
        { x = 8, y = 1 })
end

---@param engine engine
function game:create_cannon(engine, type, pos, range, muzzle)
    local cannon = {
        heat        = 0,
        relHeat     = 0,
        coolRate    = 0,
        charge      = 0,
        relCharge   = 0,
        chargeRate  = 0,

        shotsLeft   = 0,

        type        = type,
        range       = range,
        muzzle      = muzzle,

        spriteInit  = {
            texture    = self.textures.cannon[type],
            position   = pos,
            wrappable  = false,
            collidable = false
        },

        sprite      = nil, ---@type sprite

        gain_energy = function(cannon, deltaTime)
            if cannon.shotsLeft <= 0 then return end

            local gain    = cannon.chargeRate * deltaTime
            gain          = math.min(self.energyReserve, math.min(100 - cannon.charge, gain))
            cannon.charge = cannon.charge + gain

            self:set_energy_reserve(engine, self.energyReserve - gain)

            if cannon.charge >= MAX_CANNON_CHARGE then
                cannon:fire()
            end

            cannon.relHeat   = cannon.heat / MAX_HEAT
            cannon.relCharge = cannon.charge / MAX_CANNON_CHARGE
        end,

        fire        = function(cannon)
            local targetMissile = nil
            local lowestY       = 0

            for i = #self.missiles, 1, -1 do
                local missile   = self.missiles[i]
                local missile_x = missile.sprite.position.x

                if cannon.range.min <= missile_x and cannon.range.max >= missile_x then
                    local missileY = missile.sprite.position.y
                    if missileY > MIRV_ZONE_TOP and missileY > lowestY then
                        lowestY       = missileY
                        targetMissile = missile
                    end
                end
            end

            if targetMissile then
                local muzzlePos      = { x = cannon.sprite.position.x + cannon.muzzle.x, y = cannon.sprite.position.y + cannon.muzzle.y }

                local dx             = targetMissile.sprite.center.x - muzzlePos.x
                local dy             = targetMissile.sprite.center.y - muzzlePos.y
                local distance       = math.sqrt(dx * dx + dy * dy)

                local distanceFactor = math.max(0.2, 1.0 - (distance / engine.screenSize.height))
                local heatFactor     = math.sqrt((MAX_HEAT - cannon.heat) / MAX_HEAT)
                local toHit          = distanceFactor * heatFactor

                engine.fg:line(muzzlePos, targetMissile.sprite.center, Palette.LightBlue)
                engine:play_sound(self.sounds.cannon)
                cannon.shotsLeft = cannon.shotsLeft - 1
                cannon.heat      = math.min(MAX_HEAT, cannon.heat + SHOT_HEAT_GAIN)

                if engine:rnd(0, 1) <= toHit then
                    engine:give_score(100)
                    engine:play_sound(self.sounds.missileExplosion)
                    targetMissile.markedForDeath = true
                    engine.fg:circle(targetMissile.sprite.center, 2, Palette.LightBlue, true)
                end

                cannon.charge = 0
            end

            gfx.draw_dmd(engine.dmd, self)
        end,

        cool        = function(cannon)
            cannon.heat      = math.max(0, cannon.heat - cannon.coolRate - BASE_HEAT_LOSS)

            cannon.relHeat   = cannon.heat / MAX_HEAT
            cannon.relCharge = cannon.charge / 100
        end,
    }

    cannon.sprite = engine:create_sprite(cannon)

    self.cannons[type] = cannon
end

---@param i integer
---@param engine engine
function game:create_city(i, engine)
    local CITY_TEXTURES = {
        [0] = self.textures.city.undamaged,
        [1] = self.textures.city.light_damage,
        [2] = self.textures.city.heavy_damage,
        [3] = self.textures.city.destroyed,
    }
    local screenSize    = engine.screenSize
    local cityOffset    = gfx.sizes.city.width + ((screenSize.width / CITY_COUNT) - gfx.sizes.city.width) / 2
    local city          = { ---@class city: sprite_owner
        type       = "city",
        damage     = 0,
        spriteInit = {
            position   = {
                x = ((screenSize.width / CITY_COUNT) * i) - cityOffset,
                y = screenSize.height / 5 * 4
            },
            texture    = CITY_TEXTURES[0],
            wrappable  = false,
            collidable = true
        },

        collide    = function(city, b)
            if b.type == "missile" then
                if city.damage == MAX_CITY_DAMAGE then return end

                city.damage         = city.damage + 1
                city.sprite.texture = CITY_TEXTURES[city.damage]
                engine:play_sound(self.sounds.cityExplosion)
                if city.damage == MAX_CITY_DAMAGE then
                    self.destroyedCities = self.destroyedCities + 1
                end
            end
        end
    }

    local sprite        = engine:create_sprite(city)
    city.sprite         = sprite
    city.center         = sprite.center
    self.cities[i]      = city
end

---@param engine engine
function game:try_spawn_missile(engine)
    local count = #self.missiles
    if count >= #self.cities * 4 then return end
    self:create_missile(engine)
end

---@param engine engine
---@param parent? missile
function game:create_missile(engine, parent)
    local screenSize = engine.screenSize
    local missile    = { ---@class missile: sprite_owner
        linearSpeed    = engine:rnd(MIN_MISSILE_SPEED, MAX_MISSILE_SPEED),
        type           = "missile",
        markedForDeath = false,
        target         = 0,

        trail          = {},
        trailOrder     = {},

        mirv           = {
            chance = engine:rnd(MIN_CHANCE_TO_MIRV, MAX_CHANCE_TO_MIRV)
        },

        spriteInit     = {
            position   = { x = engine:rnd(0, screenSize.width - 1), y = 0 },
            texture    = self.textures.missile,
            wrappable  = false,
            collidable = true,
        },

        turn_start     = function(missile)
            local y = missile.sprite.position.y
            if missile.mirv and y >= MIRV_ZONE_TOP and y <= MIRV_ZONE_BOTTOM then
                if engine:rnd(0, 1) <= missile.mirv.chance then
                    missile.mirv.time = engine:rnd(0, DURATION / 4 * 3)
                else
                    missile.mirv = nil
                end
            end

            for key, value in pairs(missile.trail) do
                engine.fg:pixel(value, Palette.Red)
            end
        end,

        update         = function(missile, i, deltaTime, turnTime)
            if missile.markedForDeath then
                engine:remove_sprite(missile.sprite)
                table.remove(self.missiles, i)
                engine.ssd = tostring(#self.missiles)
                return
            end

            --try mirv
            if missile.mirv and missile.mirv.time and turnTime >= missile.mirv.time and not missile.markedForDeath then
                local count = engine:irnd(1, MAX_MIRV)
                for j = 1, count do
                    self:create_missile(engine, missile)
                end
                missile.mirv = nil
            end

            -- position
            local pos    = missile.sprite.position
            local rad    = missile.direction
            local vx     = math.cos(rad) * missile.linearSpeed / 1000
            local vy     = math.sin(rad) * missile.linearSpeed / 1000

            local newPos = { x = pos.x + vx * deltaTime, y = pos.y + vy * deltaTime }
            if newPos.y < screenSize.height and newPos.x >= 0 and newPos.x < screenSize.width then
                missile.sprite.position = newPos
            else
                missile.markedForDeath = true
                return
            end

            -- trail
            local trailPos = missile.sprite.center
            local trailKey = math.floor(trailPos.x) + math.floor(trailPos.y) * engine.fg.size.width

            if not missile.trail[trailKey] then
                table.insert(missile.trailOrder, trailKey)
                missile.trail[trailKey] = trailPos

                if #missile.trailOrder > 100 then
                    local oldKey = table.remove(missile.trailOrder, 1)
                    local trailX = missile.trail[oldKey].x
                    local trailY = missile.trail[oldKey].y
                    if trailY < screenSize.height and trailX >= 0 and trailX < screenSize.width then
                        engine.fg:clear({ x = trailX, y = trailY, width = 1, height = 1 })
                    end
                    missile.trail[oldKey] = nil
                end
            end

            engine.fg:pixel(trailPos, Palette.Red)
        end,

        collide        = function(missile, b)
            if b.type == "city" then
                missile.markedForDeath = true
            end
        end
    }
    local dontTarget = parent and parent.target or 0
    repeat
        missile.target = engine:irnd(1, #self.cities)
    until missile.target ~= dontTarget

    local targetCity = self.cities[missile.target]

    missile.sprite   = engine:create_sprite(missile)
    if parent then
        missile.mirv = nil
        missile.sprite.position = parent.sprite.position
    end

    missile.direction                 = math.atan(
        targetCity.center.y - missile.sprite.position.y - gfx.sizes.missile.width / 2,
        targetCity.center.x - missile.sprite.position.x - gfx.sizes.missile.height / 2)
    self.missiles[#self.missiles + 1] = missile
    engine.ssd                        = tostring(#self.missiles)
end

return game
