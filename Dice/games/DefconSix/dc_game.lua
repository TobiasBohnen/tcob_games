-- Copyright (c) 2026 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local DURATION           = 2500
local HALF_DURATION      = DURATION * 0.5
local CITY_COUNT         = 5
local MAX_CITY_DAMAGE    = 3
local MIN_CHANCE_TO_MIRV = 20
local MAX_CHANCE_TO_MIRV = 60
local MIRV_ZONE_TOP      = 5
local MIRV_ZONE_BOTTOM   = 80
local MAX_MIRV           = 2
local MIN_MISSILE_SPEED  = 10
local MAX_MISSILE_SPEED  = 15

local gfx                = require('dc_gfx')
local sfx                = require('dc_sfx')

local game               = {
    cities          = {},
    weapons         = {},
    missiles        = {},

    sockets         = {
        left = {}, ---@type { [string]: socket }
        right = {}, ---@type { [string]: socket }
        center = {}, ---@type { [string]: socket }
    },

    textures        = {
        city = { undamaged = 0, light_damage = 1, heavy_damage = 2, destroyed = 3 }, ---@type { [string]: texture }
        weapon = { left = 10, right = 11, center = 12 }, ---@type { [string]: texture }
        missile = 20, ---@type texture
    },

    destroyedCities = 0
}

---@param engine engine
function game:on_setup(engine)
    gfx.create_background(engine)
    gfx.create_textures(self, engine)
    engine:create_sounds(sfx.get_sounds(self, engine))

    for i = 1, CITY_COUNT do
        self:create_city(i, engine)
    end
    self:create_weapons(engine)

    self.sockets.left.shots   = engine:create_socket { colors = { Palette.White } }
    self.sockets.left.power   = engine:create_socket { colors = { Palette.White } }
    self.sockets.right.shots  = engine:create_socket { colors = { Palette.White } }
    self.sockets.right.power  = engine:create_socket { colors = { Palette.White } }
    self.sockets.center.shots = engine:create_socket { colors = { Palette.White } }
    self.sockets.center.power = engine:create_socket { colors = { Palette.White } }
end

---@param engine engine
function game:on_turn_start(engine)
    self:try_spawn_missile(engine)

    engine.fg:clear()

    for i = #self.missiles, 1, -1 do
        self.missiles[i]:turn_start()
    end
end

---@param engine engine
---@param deltaTime number
function game:on_turn_update(engine, deltaTime, turnTime)
    if turnTime >= DURATION then return GameStatus.TurnEnded end

    for i = #self.missiles, 1, -1 do
        self.missiles[i]:update(i, deltaTime, turnTime)
    end

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
end

---@param engine engine
function game:on_teardown(engine)
    gfx.draw_game_over(engine.dmd, self)
end

---@param engine engine
function game:draw_dmd(engine)
    gfx.draw_dmd(engine.dmd, self)
end

------
------

function game:create_city(i, engine)
    local CITY_TEXTURES = {
        [0] = self.textures.city.undamaged,
        [1] = self.textures.city.light_damage,
        [2] = self.textures.city.heavy_damage,
        [3] = self.textures.city.destroyed,
    }
    local screenSize    = engine.screenSize
    local cityOffset    = gfx.sizes.city.width + ((screenSize.width / CITY_COUNT) - gfx.sizes.city.width) / 2
    local city          = {
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

                if city.damage == MAX_CITY_DAMAGE then
                    self.destroyedCities = self.destroyedCities + 1
                end
            end
        end
    }

    local sprite        = engine:create_sprite(city)
    city.sprite         = sprite
    city.center         = {
        x = sprite.position.x + sprite.bounds.width / 2,
        y = sprite.position.y + sprite.bounds.height / 2
    }
    self.cities[i]      = city
end

function game:create_weapons(engine)
    local screenSize = engine.screenSize
    self.weapons = {
        left = {
            sprite = engine:create_sprite({
                spriteInit = {
                    texture = self.textures.weapon.left,
                    position = { x = 0, y = screenSize.height / 3 * 2 },
                    wrappable = false,
                    collidable = false
                }
            })
        },
        right = {
            sprite = engine:create_sprite({
                spriteInit = {
                    texture = self.textures.weapon.right,
                    position = { x = screenSize.width - gfx.sizes.weapon.width, y = screenSize.height / 3 * 2 },
                    wrappable = false,
                    collidable = false
                }
            })
        },
        center = {
            sprite = engine:create_sprite({
                spriteInit = {
                    texture = self.textures.weapon.center,
                    position = { x = (screenSize.width - gfx.sizes.weapon.width) / 2, y = screenSize.height / 3 * 2 },
                    wrappable = false,
                    collidable = false
                }
            })
        }
    }
end

---@param engine engine
function game:try_spawn_missile(engine)
    local count = #self.missiles
    if count >= #self.cities * 2 then return end
    self:create_missile(engine)
end

function game:create_missile(engine, parent)
    local screenSize = engine.screenSize
    local missile    = {
        linearSpeed    = engine:rnd(MIN_MISSILE_SPEED, MAX_MISSILE_SPEED),
        type           = "missile",
        markedForDeath = false,
        target         = 0,

        trail          = {},
        trailOrder     = {},

        mirv           = {
            chance = engine:irnd(MIN_CHANCE_TO_MIRV, MAX_CHANCE_TO_MIRV)
        },

        spriteInit     = {
            position   = { x = engine:rnd(0, screenSize.width - 1), y = 0 },
            texture    = self.textures.missile,
            wrappable  = false,
            collidable = true,
        },

        update         = function(missile, i, deltaTime, turnTime)
            if missile.markedForDeath then
                engine:remove_sprite(missile.sprite)
                table.remove(self.missiles, i)
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

            local bounds            = missile.sprite.bounds

            -- position
            local rad               = missile.direction
            local vx                = math.cos(rad) * missile.linearSpeed / 1000
            local vy                = math.sin(rad) * missile.linearSpeed / 1000

            local newPos            = { x = bounds.x + vx * deltaTime, y = bounds.y + vy * deltaTime }
            missile.sprite.position = newPos
            if newPos.y >= screenSize.height or newPos.x < 0 or newPos.x >= screenSize.width then
                missile.markedForDeath = true
                return
            end

            -- trail
            local trailPos = { x = math.floor(bounds.x + bounds.width / 2), y = math.floor(bounds.y + bounds.height / 2) }
            local trailKey = trailPos.x + trailPos.y * engine.fg.size.width

            if not missile.trail[trailKey] then
                table.insert(missile.trailOrder, trailKey)
                missile.trail[trailKey] = trailPos

                if #missile.trailOrder > 100 then
                    local oldKey = table.remove(missile.trailOrder, 1)
                    engine.fg:clear({ x = missile.trail[oldKey].x, y = missile.trail[oldKey].y, width = 1, height = 1 })
                    missile.trail[oldKey] = nil
                end
            end

            engine.fg:pixel(trailPos, Palette.Red)
        end,

        turn_start     = function(missile)
            local y = missile.sprite.position.y
            if missile.mirv and y >= MIRV_ZONE_TOP and y <= MIRV_ZONE_BOTTOM then
                if engine:irnd(1, 100) <= missile.mirv.chance then
                    missile.mirv.time = engine:rnd(0, DURATION / 4 * 3)
                else
                    missile.mirv = nil
                end
            end

            for key, value in pairs(missile.trail) do
                engine.fg:pixel(value, Palette.Red)
            end
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
end

return game
