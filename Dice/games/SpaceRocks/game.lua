-- Copyright (c) 2025 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

------

local function update_entity(e, deltaTime)
    e.rotation = (e.rotation + e.angularVelocity * deltaTime) % 360

    local rad = math.rad(e.direction - 90)
    local vx = math.cos(rad) * e.linearVelocity / 1000
    local vy = math.sin(rad) * e.linearVelocity / 1000

    e.x = (e.x + vx * deltaTime) % 1
    e.y = (e.y + vy * deltaTime) % 1

    e.sprite.Position = { e.x, e.y }
    e.sprite.Rotation = e.rotation
end

------

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
        angularVelocity = 0,
        sprite = nil, ---@type sprite
        texture = 0, ---@type texture
        type = "ship",
        collisionEnabled = true,
    },

    bullets = {},
    bulletTexture = 1, ---@type texture
    asteroids = {},
    asteroidTextures = { small = 2, medium = 3, large = 4 }, ---@type { [string]: texture }

    updateTime = 0,
    active = true,
}

---@param engine engine
function game:on_setup(engine)
    self.ship.sprite = engine:create_sprite(0, self.ship)

    local aType = { "large", "medium", "small" }
    for i = 1, 40 do
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
        asteroid.sprite = engine:create_sprite(i, asteroid)
        self.asteroids[i] = asteroid
    end

    engine:create_slot({ x = 1.2, y = 0.1 }, { value = 0, color = "blue", op = "Greater" })
    engine:create_slot({ x = 1.3, y = 0.1 }, { value = 0, color = "blue", op = "Greater" })
    engine:create_slot({ x = 1.4, y = 0.1 }, { value = 0, color = "blue", op = "Greater" })

    engine:set_dice_area({ x = 0.25, y = 0.5, width = 0.5, height = 0.25 })
    engine:create_dice(3, { { values = { 1, 2, 3, 4, 5, 6 }, color = "blue" } })
    engine:roll_dice()
end

---@param engine engine
function game:can_start(engine) return engine:are_slots_filled() end

---@param engine engine
function game:on_start(engine)
    self.updateTime = 0
    self.active = true
end

---@param engine engine
function game:can_run(engine) return engine:are_slots_locked() end

---@param engine engine
---@param deltaTime number
function game:on_run(engine, deltaTime)
    update_entity(self.ship, deltaTime)

    for _, a in ipairs(self.asteroids) do
        a.rotation = a.rotation + 0.1 * deltaTime
        update_entity(a, deltaTime)
    end
    for _, b in ipairs(self.bullets) do update_entity(b, deltaTime) end

    self.updateTime = self.updateTime + deltaTime
    if self.updateTime >= 2000 then
        engine:release_dice({ 1, 2, 3 })
        engine:roll_dice()
        return true
    end

    return false
end

---@param engine engine
---@param spriteA sprite
---@param spriteB sprite
function game:on_collision(engine, spriteA, spriteB)
    if (spriteA.Type == "ship" or spriteB.Type == "ship") and (spriteA.Type == "asteroid" or spriteB.Type == "asteroid") then
        --  self.active = false
    elseif spriteA.Type == "asteroid" and spriteB.Type == "asteroid" then
        local a = self.asteroids[spriteA.Index]
        local b = self.asteroids[spriteB.Index]

        a.direction = (a.direction + engine:random(45, 135)) % 360
        b.direction = (b.direction - engine:random(45, 135)) % 360

        update_entity(a, 10)
        update_entity(b, 10)
    end
end

return game
