-- Copyright (c) 2025 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT


---@class entity
---@field x number
---@field y number
---@field direction number
---@field rotation number
---@field linearVelocity number
---@field linearVelocityTarget number
---@field angularVelocity number
---@field angularVelocityTarget number
---@field sprite sprite

------

local DURATION = 2500
local HALF_DURATION = DURATION * 0.5
local INIT_ASTEROID_COUNT = 100
local EXPLOSION_DURATION = 7500

local gfx = require('gfx')

local game = {
    --public:
    draw_background = gfx.draw_background,
    get_textures = gfx.get_textures,

    --private:
    ship = {
        x = 0.5,
        y = 0.5,
        direction = 0,
        rotation = 0,
        linearVelocity = 0,
        linearVelocityTarget = 0,
        angularVelocity = 0,
        angularVelocityTarget = 0,
        sprite = nil, ---@type sprite
        texture = 0, ---@type texture
        type = "ship",
        collisionEnabled = true,
    },

    bullets = {},
    bulletTexture = 1, ---@type texture
    bulletsLeft = 0,
    bulletTime = 0,
    bulletDuration = 0,

    asteroids = {},
    asteroidTextures = { small = 2, medium = 3, large = 4 }, ---@type { [string]: texture }

    explosions = {},
    explosionTexture = 5, ---@type texture

    updateTime = 0,
}

---@param engine engine
function game:on_setup(engine)
    self.ship.sprite = engine:create_sprite(self.ship)

    local aType = { "large", "medium", "small" }
    for i = 1, INIT_ASTEROID_COUNT do
        local sz = aType[engine:randomInt(1, 3)]
        local asteroid = {
            x = engine:random(0.0, 1.0),
            y = engine:random(0.0, 1.0),
            direction = engine:random(0, 359),
            rotation = engine:random(0, 359),
            linearVelocity = engine:random(0.05, 0.15),
            angularVelocity = engine:random(0.01, 0.15),
            size = sz,
            texture = self.asteroidTextures[sz],
            type = "asteroid",
            collisionEnabled = true,
            markForDeath = false,
        }
        asteroid.sprite = engine:create_sprite(asteroid)
        self.asteroids[i] = asteroid
    end

    engine:create_slot({ x = 1.2, y = 0.1 }, { value = 0, color = "silver" })
    engine:create_slot({ x = 1.3, y = 0.1 }, { value = 0, color = "silver" })
    engine:create_slot({ x = 1.4, y = 0.1 }, { value = 0, color = "silver" })

    engine:set_dice_area({ x = 0.25, y = 0.5, width = 0.5, height = 0.25 })
    engine:create_dice(3, { { values = { 1, 2, 3, 4, 5, 6 }, color = "silver" } })
    engine:roll_dice()
end

---@param engine engine
function game:can_start(engine) return engine:are_slots_filled() end

---@param engine engine
function game:on_start(engine)
    self.updateTime = 0
    self.bulletTime = 0

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

    elseif typeA == "asteroid" and typeB == "asteroid" then
        ownerA.direction = (ownerA.direction + engine:random(45, 135)) % 360
        ownerB.direction = (ownerB.direction - engine:random(45, 135)) % 360
    elseif (typeA == "bullet" or typeB == "bullet") and (typeA == "asteroid" or typeB == "asteroid") then
        if typeA == "asteroid" then ownerA.markForDeath = true end
        if typeB == "asteroid" then ownerB.markForDeath = true end
        if typeA == "bullet" then ownerA.lifetime = HALF_DURATION - 1 end
        if typeB == "bullet" then ownerB.lifetime = HALF_DURATION - 1 end
    end
end

---@param engine engine
function game:on_finish(engine)
    for i = #self.bullets, 1, -1 do
        print(i)
        engine:remove_sprite(self.bullets[i].sprite)
        table.remove(self.bullets, i)
    end

    engine:release_dice({ 1, 2, 3 })
    engine:roll_dice()
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
        if a.markForDeath then
            local explosion = {
                x                = a.x,
                y                = a.y,
                texture          = self.explosionTexture,
                collisionEnabled = false,
                rotation         = 0,
                scale            = 1,
                lifetime         = 0,
            }
            explosion.sprite = engine:create_sprite(explosion)
            self.explosions[#self.explosions + 1] = explosion

            engine:remove_sprite(a.sprite)
            table.remove(self.asteroids, i)
        else
            a.rotation = a.rotation + 0.1 * deltaTime
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
    ship.direction       = ship.rotation
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
            e.rotation       = e.rotation + 720 * deltaTime / 1000
            s.Rotation       = e.rotation

            local shrinkRate = deltaTime / EXPLOSION_DURATION
            e.scale          = e.scale - shrinkRate
            if e.scale < 0 then e.scale = 0 end

            local size    = s.Size
            local newSize = { width = size.width * e.scale, height = size.height * e.scale }
            s.Size        = newSize
            local newPos  = { x = e.x + (size.width - newSize.width) / 2, y = e.y + (size.height - newSize.height) / 2 }
            s.Position    = newPos
            e.x           = newPos.x
            e.y           = newPos.y
        end
    end
end

---@param engine engine
function game:spawn_bullet(engine)
    local bullet                    = {
        x = 0,
        y = 0,
        direction = self.ship.direction,
        rotation = 0,
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
    local rad                       = math.rad(self.ship.rotation)
    local bulletBounds              = bullet.sprite.Bounds

    bullet.x                        = self.ship.x + (shipBounds.width * 0.5)
        + (math.sin(rad) * shipBounds.height * 0.5)
        - (bulletBounds.width * 0.5)

    bullet.y                        = self.ship.y + (shipBounds.height * 0.5)
        - (math.cos(rad) * shipBounds.height * 0.5)
        - (bulletBounds.height * 0.5)

    bullet.sprite.Position          = { x = bullet.x, y = bullet.y }
end

function game:update_entity(e, deltaTime)
    local rad = math.rad(e.direction - 90)
    local vx = math.cos(rad) * e.linearVelocity / 1000
    local vy = math.sin(rad) * e.linearVelocity / 1000

    e.x = (e.x + vx * deltaTime) % 1
    e.y = (e.y + vy * deltaTime) % 1

    e.sprite.Position = { x = e.x, y = e.y }

    e.rotation = (e.rotation + e.angularVelocity * deltaTime) % 360
    e.sprite.Rotation = e.rotation
end

return game
