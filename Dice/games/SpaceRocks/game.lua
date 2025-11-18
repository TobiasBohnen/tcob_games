-- Copyright (c) 2025 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT


---@class entity
---@field direction number
---@field linearVelocity number
---@field linearVelocityTarget number
---@field angularVelocity number
---@field angularVelocityTarget number
---@field sprite sprite

------

local DURATION = 2500
local HALF_DURATION = DURATION * 0.5
local INIT_ASTEROID_COUNT = 10
local EXPLOSION_DURATION = 750

local gfx = require('gfx')

local game = {
    --public:
    draw_background = gfx.draw_background,
    get_textures = gfx.get_textures,

    --private:
    ship = {
        direction = 0,
        linearVelocity = 0,
        linearVelocityTarget = 0,
        angularVelocity = 0,
        angularVelocityTarget = 0,
        sprite = nil, ---@type sprite
        texture = 0, ---@type texture
        hurtTexture = 1, ---@type texture
        type = "ship",
        collisionEnabled = true,
        health = 10,
        invulnerable = false
    },

    bullets = {},
    bulletTexture = 10, ---@type texture
    bulletsLeft = 0,
    bulletTime = 0,
    bulletDuration = 0,

    asteroids = {},
    asteroidTextures = { small = 20, medium = 21, large = 22 }, ---@type { [string]: texture }

    explosions = {},
    explosionTexture = 30, ---@type texture

    updateTime = 0,
}

---@param engine engine
function game:on_setup(engine)
    self.ship.sprite = engine:create_sprite(self.ship)
    self.ship.sprite.Position = { x = 0.5, y = 0.5 }

    for i = 1, INIT_ASTEROID_COUNT do
        self:spawn_asteroid(engine, "large", engine:random(0.0, 1.0), engine:random(0.0, 1.0))
    end

    engine:create_slot({ x = 1.2, y = 0.1 }, { value = 0, color = "silver" })
    engine:create_slot({ x = 1.3, y = 0.1 }, { value = 0, color = "silver" })
    engine:create_slot({ x = 1.4, y = 0.1 }, { value = 0, color = "silver" })

    engine:set_dice_area({ x = 0.25, y = 0.25, width = 0.5, height = 0.25 })
    engine:create_dice(3, { { values = { 1, 2, 3, 4, 5, 6 }, color = "silver" } })
    engine:roll_dice()
end

---@param engine engine
function game:can_start(engine) return engine:are_slots_filled() end

---@param engine engine
function game:on_start(engine)
    self.updateTime = 0
    self.bulletTime = 0
    engine.log(tostring(self.ship.health))

    local spawnAsteroid = (#self.asteroids < INIT_ASTEROID_COUNT / 5)
        or (#self.asteroids < INIT_ASTEROID_COUNT and engine:random(0, 1) == 1)

    if spawnAsteroid then
        self:spawn_asteroid(engine, "large", engine:random(0.0, 1.0), engine:random(0.0, 1.0))
        engine.log("spawn at " .. #self.asteroids)
    end

    self.ship.linearVelocityTarget = engine:slot_die_value(1) / 12.0
    self.ship.angularVelocityTarget = (engine:slot_die_value(2) - 3.5) / 25
    self.bulletsLeft = engine:slot_die_value(3)
    self.bulletDuration = HALF_DURATION / self.bulletsLeft
end

---@param engine engine
function game:can_run(engine) return engine:are_slots_locked() end

---@param engine engine
---@param deltaTime number
function game:on_run(engine, deltaTime)
    self.updateTime = self.updateTime + deltaTime
    if self.updateTime >= DURATION then return false end

    -- bullets spawn
    if self.bulletsLeft > 0 then
        self.bulletTime = self.bulletTime - deltaTime
        if self.bulletTime <= 0 then
            self:spawn_bullet(engine)
            self.bulletTime  = self.bulletDuration
            self.bulletsLeft = self.bulletsLeft - 1
        end
    end

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
    local ownerA = spriteA.Owner
    local typeA = ownerA.type
    local ownerB = spriteB.Owner
    local typeB = ownerB.type

    if (typeA == "ship" or typeB == "ship") and (typeA == "asteroid" or typeB == "asteroid") then
        if not self.ship.invulnerable then
            self.ship.health = self.ship.health - 1
            self.ship.invulnerable = true
            self.ship.sprite.Texture = self.ship.hurtTexture
        end

        if typeA == "asteroid" then
            ownerA.direction = self.ship.direction
            ownerA.linearVelocity = self.ship.linearVelocity * 1.1
        end
        if typeB == "asteroid" then
            ownerB.direction = self.ship.direction
            ownerB.linearVelocity = self.ship.linearVelocity * 1.1
        end
    elseif typeA == "asteroid" and typeB == "asteroid" then
        ownerA.direction = (ownerA.direction + engine:random(45, 135)) % 360
        ownerB.direction = (ownerB.direction - engine:random(45, 135)) % 360
    elseif (typeA == "bullet" or typeB == "bullet") and (typeA == "asteroid" or typeB == "asteroid") then
        if typeA == "asteroid" then ownerA.markedForDeath = true end
        if typeB == "asteroid" then ownerB.markedForDeath = true end
        if typeA == "bullet" then ownerA.lifetime = HALF_DURATION - 1 end
        if typeB == "bullet" then ownerB.lifetime = HALF_DURATION - 1 end
    end
end

---@param engine engine
function game:on_finish(engine)
    engine:release_dice({ 1, 2, 3 })
    engine:roll_dice()

    self.ship.invulnerable = false
    self.ship.sprite.Texture = self.ship.texture
end

---@param engine engine
function game:update_bullets(engine, deltaTime)
    for i = #self.bullets, 1, -1 do
        local b = self.bullets[i]
        if b.lifetime < HALF_DURATION then
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
                local explosion = {
                    texture          = self.explosionTexture,
                    collisionEnabled = false,
                    lifetime         = 0,
                }
                explosion.sprite = engine:create_sprite(explosion)
                explosion.sprite.Position = a.sprite.Position
                self.explosions[#self.explosions + 1] = explosion
            else
                local newSize = a.size == "medium" and "small" or "medium"
                local aBounds = a.sprite.Bounds

                self:spawn_asteroid(engine, newSize, aBounds.x, aBounds.y)
                self:spawn_asteroid(engine, newSize, aBounds.x + aBounds.width, aBounds.y + aBounds.height)
            end


            engine:remove_sprite(a.sprite)
            table.remove(self.asteroids, i)
        else
            a.sprite.Rotation = a.sprite.Rotation + 0.1 * deltaTime
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
    ship.direction       = ship.sprite.Rotation
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
            s.Rotation = s.Rotation + 720 * deltaTime / 1000
            local scale = 1 - (e.lifetime / EXPLOSION_DURATION)
            s.Scale = { width = scale, height = scale }
        end
    end
end

function game:update_entity(e, deltaTime)
    local rad = math.rad(e.direction - 90)
    local vx = math.cos(rad) * e.linearVelocity / 1000
    local vy = math.sin(rad) * e.linearVelocity / 1000

    local pos = e.sprite.Position
    e.sprite.Position = { x = (pos.x + vx * deltaTime) % 1, y = (pos.y + vy * deltaTime) % 1 }
    e.sprite.Rotation = (e.sprite.Rotation + e.angularVelocity * deltaTime) % 360
end

---@param engine engine
function game:spawn_bullet(engine)
    local bullet                    = {
        direction = self.ship.direction,
        linearVelocity = math.max(0.5, self.ship.linearVelocity * 1.5),
        angularVelocity = 0,
        texture = self.bulletTexture,
        type = "bullet",
        collisionEnabled = true,
        lifetime = 0,
    }

    bullet.sprite                   = engine:create_sprite(bullet)
    self.bullets[#self.bullets + 1] = bullet

    local shipBounds                = self.ship.sprite.Bounds
    local rad                       = math.rad(self.ship.sprite.Rotation)
    local bulletBounds              = bullet.sprite.Bounds

    local bx                        = shipBounds.x + (shipBounds.width * 0.5)
        + (math.sin(rad) * shipBounds.height * 0.5)
        - (bulletBounds.width * 0.5)

    local by                        = shipBounds.y + (shipBounds.height * 0.5)
        - (math.cos(rad) * shipBounds.height * 0.5)
        - (bulletBounds.height * 0.5)

    bullet.sprite.Position          = { x = bx, y = by }
end

---@param engine engine
function game:spawn_asteroid(engine, size, x, y)
    local asteroid = {
        direction = engine:random(0, 359),
        linearVelocity = engine:random(0.05, 0.15),
        angularVelocity = engine:random(0.01, 0.15),
        size = size,
        texture = self.asteroidTextures[size],
        type = "asteroid",
        collisionEnabled = true,
        markedForDeath = false,
    }
    asteroid.sprite = engine:create_sprite(asteroid)
    asteroid.sprite.Rotation = engine:random(0, 359)
    asteroid.sprite.Position = { x = x, y = y }
    self.asteroids[#self.asteroids + 1] = asteroid
end

return game
