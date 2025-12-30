-- Copyright (c) 2025 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local DURATION        = 2500
local HALF_DURATION   = DURATION * 0.5
local CITY_COUNT      = 4
local MAX_CITY_DAMAGE = 3

local gfx             = require('do.gfx')
local sfx             = require('do.sfx')

local game            = {
    cities = {},
    weapons = {},
    missiles = {},

    textures = {
        city = { undamaged = 0, light_damage = 1, heavy_damage = 2, destroyed = 3 }, ---@type { [string]: texture }
        weapon = { left = 10, right = 11, center = 12 }, ---@type { [string]: texture }
        missile = 20, ---@type texture
    },
}

local cityOffset      = gfx.sizes.city.width + ((ScreenSize.width / CITY_COUNT) - gfx.sizes.city.width) / 2

---@param engine engine
function game:on_setup(engine)
    engine.bg:blit({ x = 0, y = 0, width = ScreenSize.width, height = ScreenSize.height }, gfx.get_background(self, engine))
    engine:create_textures(gfx.get_textures(self, engine))
    engine:create_sounds(sfx.get_sounds(self, engine))

    for i = 1, CITY_COUNT do
        self:create_city(i, engine)
    end
    self:create_weapons(engine)

    gfx.draw_dmd(engine.dmd, self)
end

---@param engine engine
function game:on_turn_start(engine)
    self:try_spawn_missile(engine)
end

---@param engine engine
---@param deltaTime number
function game:on_turn_update(engine, deltaTime, turnTime)
    if turnTime >= DURATION then return GameStatus.TurnEnded end

    for i = #self.missiles, 1, -1 do
        self.missiles[i]:update(i, deltaTime)
    end

    for i = 1, #self.cities do
        if self.cities[i].damage < MAX_CITY_DAMAGE then
            return GameStatus.Running
        end
    end
    return GameStatus.GameOver
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
---@param socket socket
function game:on_die_change(engine, socket)
    gfx.draw_dmd(engine.dmd, self)
end

---@param engine engine
function game:on_die_motion(engine)
    gfx.draw_dmd(engine.dmd, self)
end

---@param engine engine
function game:on_turn_finish(engine)
    gfx.draw_dmd(engine.dmd, self)
end

---@param engine engine
function game:on_teardown(engine)
    gfx.draw_game_over(engine.dmd, self)
end

------
------

function game:create_city(i, engine)
    local CITY_DAMAGE_TEXTURES = {
        [0] = self.textures.city.undamaged,
        [1] = self.textures.city.light_damage,
        [2] = self.textures.city.heavy_damage,
        [3] = self.textures.city.destroyed,
    }
    local city                 = {
        type       = "city",
        damage     = 0,
        spriteInit = {
            position   = {
                x = ((ScreenSize.width / CITY_COUNT) * i) - cityOffset,
                y = ScreenSize.height / 5 * 4
            },
            texture    = CITY_DAMAGE_TEXTURES[0],
            wrappable  = false,
            collidable = true
        },

        do_damage  = function(city)
            city.damage         = math.min(city.damage + 1, MAX_CITY_DAMAGE)
            city.sprite.texture = CITY_DAMAGE_TEXTURES[city.damage]
        end,

        collide    = function(city, b)
            if b.type == "missile" then
                city:do_damage()
            end
        end
    }

    local sprite               = engine:create_sprite(city)
    city.sprite                = sprite
    city.center                = {
        x = sprite.position.x + sprite.size.width / 2,
        y = sprite.position.y + sprite.size.height / 2
    }
    self.cities[i]             = city
end

function game:create_weapons(engine)
    self.weapons = {
        left = {
            sprite = engine:create_sprite({
                spriteInit = {
                    texture = self.textures.weapon.left,
                    position = { x = 0, y = ScreenSize.height / 3 * 2 },
                    wrappable = false,
                    collidable = false
                }
            })
        },
        right = {
            sprite = engine:create_sprite({
                spriteInit = {
                    texture = self.textures.weapon.right,
                    position = { x = ScreenSize.width - gfx.sizes.weapon.width, y = ScreenSize.height / 3 * 2 },
                    wrappable = false,
                    collidable = false
                }
            })
        },
        center = {
            sprite = engine:create_sprite({
                spriteInit = {
                    texture = self.textures.weapon.center,
                    position = { x = (ScreenSize.width - gfx.sizes.weapon.width) / 2, y = ScreenSize.height / 3 * 2 },
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

function game:create_missile(engine)
    local missile                     = {
        target = engine:irnd(1, #self.cities),
        linearVelocity = engine:rnd(15, 30),
        type = "missile",
        markedForDeath = false,
        spriteInit = {
            position   = { x = engine:rnd(0, ScreenSize.width), y = 0 },
            texture    = self.textures.missile,
            wrappable  = false,
            collidable = true,
        },
        update = function(m, i, deltaTime)
            if m.markedForDeath then
                engine:remove_sprite(m.sprite)
                table.remove(self.missiles, i)
                return
            end
            local rad         = m.direction
            local vx          = math.cos(rad) * m.linearVelocity / 1000
            local vy          = math.sin(rad) * m.linearVelocity / 1000

            local pos         = m.sprite.position
            local newPos      = { x = (pos.x + vx * deltaTime), y = (pos.y + vy * deltaTime) }
            m.sprite.position = newPos
            if newPos.y > ScreenSize.height or newPos.x < 0 or newPos.x > ScreenSize.width then
                m.markedForDeath = true
            end
        end,
        collide = function(missile, b)
            if b.type == "missile" or b.type == "city" then
                missile.markedForDeath = true
            end
        end
    }
    local target                      = self.cities[missile.target]

    missile.sprite                    = engine:create_sprite(missile)
    missile.direction                 = math.atan(target.center.y - missile.sprite.position.y - 4, target.center.x - missile.sprite.position.x - 4)

    self.missiles[#self.missiles + 1] = missile
end

return game
