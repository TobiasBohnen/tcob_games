-- Copyright (c) 2025 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT


------

local DURATION = 2500
local HALF_DURATION = DURATION * 0.5
local INIT_ASTEROID_COUNT = 40

local gfx = require('gfx')

local game = {
    --public:
    get_textures = gfx.get_textures,
    draw_background = gfx.draw_background,

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
    asteroids = {},
    asteroidTextures = { small = 2, medium = 3, large = 4 }, ---@type { [string]: texture }
    asteroidRemoveList = {},

    updateTime = 0,
    active = true,
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
    self.active = true

    self.ship.linearVelocityTarget = engine:slot_die_value(1) / 12.0
    self.ship.angularVelocityTarget = (engine:slot_die_value(2) - 3.5) / 25
    local bullets = engine:slot_die_value(3)
end

---@param engine engine
function game:can_run(engine) return engine:are_slots_locked() end

---@param engine engine
---@param deltaTime number
function game:on_run(engine, deltaTime)
    self.updateTime = self.updateTime + deltaTime
    if self.updateTime >= DURATION then
        engine:release_dice({ 1, 2, 3 })
        engine:roll_dice()
        return true
    end

    -- remove asteroids
    table.sort(self.asteroidRemoveList, function(x, y) return x.index > y.index end)
    for _, item in ipairs(self.asteroidRemoveList) do
        engine:remove_sprite(item.sprite)
        table.remove(self.asteroids, item.index)
    end
    self.asteroidRemoveList = {}

    -- update
    local function update_entity(e)
        local rad = math.rad(e.direction - 90)
        local vx = math.cos(rad) * e.linearVelocity / 1000
        local vy = math.sin(rad) * e.linearVelocity / 1000

        e.x = (e.x + vx * deltaTime) % 1
        e.y = (e.y + vy * deltaTime) % 1

        e.sprite.Position = { x = e.x, y = e.y }

        e.rotation = (e.rotation + e.angularVelocity * deltaTime) % 360
        e.sprite.Rotation = e.rotation
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

    -- update asteroids
    for _, a in ipairs(self.asteroids) do
        a.rotation = a.rotation + 0.1 * deltaTime
        update_entity(a)
    end
    -- update bullets
    for _, b in ipairs(self.bullets) do update_entity(b) end

    return false
end

---@param engine engine
---@param spriteA sprite
---@param spriteB sprite
function game:on_collision(engine, spriteA, spriteB)
    if (spriteA.Type == "ship" or spriteB.Type == "ship") and (spriteA.Type == "asteroid" or spriteB.Type == "asteroid") then
        --  self.active = false
    elseif spriteA.Type == "asteroid" and spriteB.Type == "asteroid" then
        local indexA, indexB
        for idx, ast in ipairs(self.asteroids) do
            if ast.sprite == spriteA then indexA = idx end
            if ast.sprite == spriteB then indexB = idx end
            if indexA and indexB then break end
        end

        local a = self.asteroids[indexA]
        local b = self.asteroids[indexB]

        -- bounce directions
        a.direction = (a.direction + engine:random(45, 135)) % 360
        b.direction = (b.direction - engine:random(45, 135)) % 360

        local function insertUnique(index, sprite)
            for _, item in ipairs(self.asteroidRemoveList) do
                if item.index == index then return end
            end
            table.insert(self.asteroidRemoveList, { index = index, sprite = sprite })
        end

        if a.size == "small" then insertUnique(indexA, spriteA) end
        if b.size == "small" then insertUnique(indexB, spriteB) end
    end
end

return game
