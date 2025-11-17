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
    if self.updateTime >= DURATION then
        return false
    end

    -- spawn bullets
    if self.bulletsLeft > 0 then
        self.bulletTime = self.bulletTime - deltaTime
        if self.bulletTime <= 0 then
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
            }

            bullet.sprite                   = engine:create_sprite(bullet)
            bullet.life                     = HALF_DURATION
            self.bullets[#self.bullets + 1] = bullet
            self.bulletTime                 = self.bulletDuration
            self.bulletsLeft                = self.bulletsLeft - 1

            local shipBounds                = self.ship.sprite.Bounds
            local noseOffset                = shipBounds.height * 0.5
            local rad                       = math.rad(self.ship.sprite.Rotation)
            local noseX                     = self.ship.x + shipBounds.width * 0.5 + math.sin(rad) * noseOffset
            local noseY                     = self.ship.y + shipBounds.height * 0.5 - math.cos(rad) * noseOffset
            local bulletBounds              = bullet.sprite.Bounds
            bullet.x                        = noseX - bulletBounds.width * 0.5
            bullet.y                        = noseY - bulletBounds.height * 0.5
            bullet.sprite.Position          = { x = bullet.x, y = bullet.y }
        end
    end

    -- update
    local function update_entity(e) ---@param e entity
        local rad = math.rad(e.direction - 90)
        local vx = math.cos(rad) * e.linearVelocity / 1000
        local vy = math.sin(rad) * e.linearVelocity / 1000

        e.x = (e.x + vx * deltaTime) % 1
        e.y = (e.y + vy * deltaTime) % 1

        e.sprite.Position = { x = e.x, y = e.y }

        e.rotation = (e.rotation + e.angularVelocity * deltaTime) % 360
        e.sprite.Rotation = e.rotation
    end

    -- update asteroids
    for i = #self.asteroids, 1, -1 do
        local a = self.asteroids[i]
        if a.markForDeath then
            engine:remove_sprite(a.sprite)
            table.remove(self.asteroids, i)
        else
            a.rotation = a.rotation + 0.1 * deltaTime
            update_entity(a)
        end
    end

    -- update bullets
    for i = #self.bullets, 1, -1 do
        local b = self.bullets[i]
        b.life = b.life - deltaTime
        if b.life > 0 then
            update_entity(b)
        else
            engine:remove_sprite(b.sprite)
            table.remove(self.bullets, i)
        end
    end

    -- update ship
    local factor         = self.updateTime < HALF_DURATION
        and self.updateTime / HALF_DURATION
        or 1 - ((self.updateTime - HALF_DURATION) / HALF_DURATION)

    local ship           = self.ship
    ship.linearVelocity  = ship.linearVelocityTarget * factor
    ship.angularVelocity = ship.angularVelocityTarget * factor
    update_entity(ship)
    ship.direction = ship.rotation

    return true
end

---@param engine engine
---@param spriteA sprite
---@param spriteB sprite
function game:on_collision(engine, spriteA, spriteB)
    local indexA, indexB
    local function findAsteroids()
        for idx, ast in ipairs(self.asteroids) do
            if ast.sprite == spriteA then indexA = idx end
            if ast.sprite == spriteB then indexB = idx end
            if indexA and indexB then break end
        end
    end

    if (spriteA.Type == "ship" or spriteB.Type == "ship") and (spriteA.Type == "asteroid" or spriteB.Type == "asteroid") then

    elseif spriteA.Type == "asteroid" and spriteB.Type == "asteroid" then
        findAsteroids()
        local a = self.asteroids[indexA]
        a.direction = (a.direction + engine:random(45, 135)) % 360
        local b = self.asteroids[indexB]
        b.direction = (b.direction - engine:random(45, 135)) % 360
    elseif (spriteA.Type == "bullet" or spriteB.Type == "bullet") and (spriteA.Type == "asteroid" or spriteB.Type == "asteroid") then
        findAsteroids()
        if indexA then self.asteroids[indexA].markForDeath = true end
        if indexB then self.asteroids[indexB].markForDeath = true end
    end
end

function game:on_finish(engine)
    for i = #self.bullets, 1, -1 do
        print(i)
        engine:remove_sprite(self.bullets[i].sprite)
        table.remove(self.bullets, i)
    end

    engine:release_dice({ 1, 2, 3 })
    engine:roll_dice()
end

return game
