-- Copyright (c) 2025 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

------

local DURATION            = 2500
local HALF_DURATION       = DURATION * 0.5
local INIT_ASTEROID_COUNT = 10
local EXPLOSION_DURATION  = 750

local gfx                 = require('gfx')
local sfx                 = require('sfx')

local game                = {
    --public:
    get_background   = gfx.get_background,
    get_textures     = gfx.get_textures,
    get_sounds       = sfx.get_sounds,

    --private:
    ship             = {
        direction = 0,
        linearVelocity = 0,
        linearVelocityTarget = 0,
        angularVelocity = 0,
        angularVelocityTarget = 0,
        sprite = nil, ---@type sprite
        texture = 0, ---@type texture
        hurtTexture = 1, ---@type texture
        type = "ship",
        health = 5,
        invulnerable = false
    },

    bullets          = {},
    bulletTexture    = 10, ---@type texture
    bulletsLeft      = 0,
    bulletTime       = 0,

    asteroids        = {},
    asteroidTextures = { small = 20, medium = 21, large = 22 }, ---@type { [string]: texture }

    explosions       = {},
    explosionTexture = 30, ---@type texture

    slots            = {}, ---@type { [string]: slot }

    updateTime       = 0,
}

---@param engine engine
function game:on_setup(engine)
    self.ship.sprite          = engine:create_sprite(self.ship)
    self.ship.sprite.position = { x = 0.5, y = 0.5 }

    while #self.asteroids < INIT_ASTEROID_COUNT do
        self:try_spawn_asteroid(engine)
    end

    self.slots.speed            = engine:create_slot({ value = 0, color = Palette.White })
    self.slots.speed.position   = { x = 0, y = 75 }
    self.slots.turn             = engine:create_slot({ value = 0, color = Palette.White })
    self.slots.turn.position    = { x = 66, y = 75 }
    self.slots.bullets          = engine:create_slot({ value = 0, color = Palette.White })
    self.slots.bullets.position = { x = 30, y = 95 }

    engine:create_dice(4, { { values = { 1, 2, 3, 4, 5, 6 }, color = Palette.White } })
    engine:roll_dice()
    gfx.draw_dmd(engine.dmd, self)
end

---@param engine engine
function game:can_start(engine)
    local s = self.slots
    return not (s.speed.isEmpty or s.turn.isEmpty or s.bullets.isEmpty)
end

---@param engine engine
function game:on_start(engine)
    self.updateTime = 0
    self.bulletTime = 0

    self:try_spawn_asteroid(engine)

    self.ship.linearVelocityTarget  = self.slots.speed.dieValue / 12.0
    self.ship.angularVelocityTarget = (self.slots.turn.dieValue - 3.5) / 25
    self.bulletsLeft                = self.slots.bullets.dieValue
end

---@param engine engine
---@param deltaTime number
function game:on_run(engine, deltaTime)
    self.updateTime = self.updateTime + deltaTime
    if self.updateTime >= DURATION then return false end

    self:try_spawn_bullet(engine, deltaTime)

    self:update_asteroids(engine, deltaTime)
    self:update_explosions(engine, deltaTime)
    self:update_bullets(engine, deltaTime)
    self:update_ship(deltaTime)
    return true
end

---@param engine engine
---@param spriteA sprite
---@param spriteB sprite
function game:on_collision(engine, spriteA, spriteB)
    local a, b   = spriteA.owner, spriteB.owner
    local tA, tB = a.type, b.type

    local key, first, second
    if tA < tB then
        key, first, second = tA .. "_" .. tB, a, b
    else
        key, first, second = tB .. "_" .. tA, b, a
    end

    if key == "asteroid_ship" then
        local ship = self.ship
        if not ship.invulnerable then
            ship.health         = ship.health - 1
            ship.invulnerable   = true
            ship.sprite.texture = ship.hurtTexture
            gfx.draw_dmd(engine.dmd, self)
        end

        local asteroid          = first.type == "asteroid" and first or second
        asteroid.direction      = ship.direction
        asteroid.linearVelocity = ship.linearVelocity * 1.1
        asteroid.markedForDeath = true
    elseif key == "asteroid_bullet" then
        local asteroid          = first.type == "asteroid" and first or second
        local bullet            = first.type == "bullet" and first or second
        asteroid.markedForDeath = true
        bullet.lifetime         = DURATION - 1
    elseif key == "asteroid_asteroid" then
        first.direction  = (first.direction + engine:random(45, 135)) % 360
        second.direction = (second.direction - engine:random(45, 135)) % 360
    end
end

---@param engine engine
---@param slot slot
function game:on_slot_die_changed(engine, slot)
    gfx.draw_dmd(engine.dmd, self)
end

---@param engine engine
function game:on_finish(engine)
    engine:reset_slots(self.slots)
    engine:roll_dice()
    gfx.draw_dmd(engine.dmd, self)

    self.ship.invulnerable   = false
    self.ship.sprite.texture = self.ship.texture
end

---@param engine engine
function game:update_bullets(engine, deltaTime)
    for i = #self.bullets, 1, -1 do
        local b = self.bullets[i]
        if b.lifetime < DURATION then
            self:update_entity(b, deltaTime)
        else
            engine:remove_sprite(b.sprite)
            table.remove(self.bullets, i)
        end
        b.lifetime = b.lifetime + deltaTime
    end
end

---@param engine engine
function game:update_asteroids(engine, deltaTime)
    for i = #self.asteroids, 1, -1 do
        local a = self.asteroids[i]
        if a.markedForDeath then
            if a.size == "small" then
                local explosion                       = {
                    texture    = self.explosionTexture,
                    collidable = false,
                    lifetime   = 0,
                }
                explosion.sprite                      = engine:create_sprite(explosion)
                explosion.sprite.position             = a.sprite.position
                self.explosions[#self.explosions + 1] = explosion
            else
                local newSize = a.size == "medium" and "small" or "medium"
                local aBounds = a.sprite.bounds

                self:spawn_asteroid(engine, newSize, aBounds.x, aBounds.y)
                self:spawn_asteroid(engine, newSize, aBounds.x + aBounds.width, aBounds.y + aBounds.height)
            end

            engine:remove_sprite(a.sprite)
            table.remove(self.asteroids, i)
            engine:play_sound(0)
            engine:give_score(100)
        else
            a.sprite.rotation = a.sprite.rotation + 0.1 * deltaTime
            self:update_entity(a, deltaTime)
        end
    end
end

function game:update_ship(deltaTime)
    local factor         = self.updateTime < HALF_DURATION
        and self.updateTime / HALF_DURATION
        or 1 - ((self.updateTime - HALF_DURATION) / HALF_DURATION)

    local ship           = self.ship
    ship.linearVelocity  = ship.linearVelocityTarget * factor
    ship.angularVelocity = ship.angularVelocityTarget * factor
    ship.direction       = ship.sprite.rotation
    self:update_entity(ship, deltaTime)
end

function game:update_explosions(engine, deltaTime)
    for i = #self.explosions, 1, -1 do
        local e    = self.explosions[i]
        local s    = e.sprite

        e.lifetime = e.lifetime + deltaTime
        if e.lifetime >= EXPLOSION_DURATION then
            engine:remove_sprite(s)
            table.remove(self.explosions, i)
        else
            s.rotation  = s.rotation + 720 * deltaTime / 1000
            local scale = 1 - (e.lifetime / EXPLOSION_DURATION)
            s.scale     = { width = scale, height = scale }
        end
    end
end

function game:update_entity(e, deltaTime)
    local rad         = math.rad(e.direction - 90)
    local vx          = math.cos(rad) * e.linearVelocity / 1000
    local vy          = math.sin(rad) * e.linearVelocity / 1000

    local pos         = e.sprite.position
    e.sprite.position = { x = (pos.x + vx * deltaTime) % 1, y = (pos.y + vy * deltaTime) % 1 }
    e.sprite.rotation = (e.sprite.rotation + e.angularVelocity * deltaTime) % 360
end

---@param engine engine
function game:try_spawn_bullet(engine, deltaTime)
    if self.bulletsLeft <= 0 then return end
    self.bulletTime = self.bulletTime - deltaTime
    if self.bulletTime > 0 then return end

    local bullet                    = {
        direction       = self.ship.direction,
        linearVelocity  = math.max(0.5, self.ship.linearVelocity * 1.5),
        angularVelocity = 0,
        texture         = self.bulletTexture,
        type            = "bullet",
        lifetime        = 0,
    }

    bullet.sprite                   = engine:create_sprite(bullet)
    self.bullets[#self.bullets + 1] = bullet

    local ship                      = self.ship.sprite
    local shipBounds                = ship.bounds
    local bulletBounds              = bullet.sprite.bounds
    local rad                       = math.rad(ship.rotation)

    local bx                        =
        shipBounds.x + shipBounds.width * 0.5
        + math.sin(rad) * shipBounds.width * 0.5
        - bulletBounds.width * 0.5

    local by                        =
        shipBounds.y + shipBounds.height * 0.5
        - math.cos(rad) * shipBounds.height * 0.5
        - bulletBounds.height * 0.5

    bullet.sprite.position          = { x = bx, y = by }

    self.bulletTime                 = HALF_DURATION / self.slots.bullets.dieValue
    self.bulletsLeft                = self.bulletsLeft - 1
end

---@param engine engine
function game:try_spawn_asteroid(engine)
    local count = #self.asteroids
    if count >= INIT_ASTEROID_COUNT then return end

    local edge = engine:random_int(1, 4)
    local x, y
    if edge == 1 then
        x = engine:random(0.0, 0.05); y = engine:random(0.0, 1.0)
    elseif edge == 2 then
        x = engine:random(0.95, 1.0); y = engine:random(0.0, 1.0)
    elseif edge == 3 then
        x = engine:random(0.0, 1.0); y = engine:random(0.0, 0.05)
    else
        x = engine:random(0.0, 1.0); y = engine:random(0.95, 1.0)
    end

    local sx, sy = self.ship.sprite.position.x, self.ship.sprite.position.y
    if math.sqrt((sx - x) ^ 2 + (sy - y) ^ 2) < 0.15 then return end

    self:spawn_asteroid(engine, "large", x, y)
end

---@param engine engine
function game:spawn_asteroid(engine, size, x, y)
    local asteroid                      = {
        direction       = engine:random(0, 359),
        linearVelocity  = engine:random(0.05, 0.15),
        angularVelocity = engine:random(0.01, 0.15),
        size            = size,
        texture         = self.asteroidTextures[size],
        type            = "asteroid",
        markedForDeath  = false,
    }
    asteroid.sprite                     = engine:create_sprite(asteroid)
    asteroid.sprite.rotation            = engine:random(0, 359)
    asteroid.sprite.position            = { x = x, y = y }
    self.asteroids[#self.asteroids + 1] = asteroid
end

return game
