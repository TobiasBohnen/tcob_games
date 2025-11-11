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

    e.sprite.Position = e
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

    slots = {}, ---@type slots

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
            linearVelocity = engine:random(0.01, 0.15),
            angularVelocity = engine:random(0.01, 0.15),
            size = sz,
            texture = self.asteroidTextures[sz],
            type = "asteroid",
            collisionEnabled = true,
        }
        asteroid.sprite = engine:create_sprite(i, asteroid)
        self.asteroids[i] = asteroid
    end

    self.slots = engine:create_slots({
        { position = { x = 0.9, y = 0.1 } },
        { position = { x = 0.9, y = 0.15 } },
        { position = { x = 0.9, y = 0.2 } }
    })

    engine:create_dice({
        { position = { x = 0.85, y = 0.1 } },
        { position = { x = 0.85, y = 0.15 } },
        { position = { x = 0.85, y = 0.2 } }
    })
end

---@param engine engine
function game:can_process_turn(engine)
    -- return self.slots:are_locked()
    return self.active
end

---@param engine engine
---@param deltaTime number
function game:on_process_turn(engine, deltaTime)
    update_entity(self.ship, deltaTime)

    for _, a in ipairs(self.asteroids) do
        a.rotation = a.rotation + 0.1 * deltaTime
        update_entity(a, deltaTime)
    end
    for _, b in ipairs(self.bullets) do update_entity(b, deltaTime) end

    self.updateTime = self.updateTime + deltaTime
    if self.updateTime >= 2000 then
        --self.slots:unlock()
        engine:roll_dice()
        return true
    end

    return false
end

---@param engine engine
function game:can_end_turn(engine) return self.slots:is_complete() end

---@param engine engine
function game:on_end_turn(engine)
    self.updateTime = 0
    self.slots:lock()
    self.active = true
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

        a.direction = (a.direction + engine:random(75, 105)) % 360
        b.direction = (b.direction - engine:random(75, 105)) % 360

        update_entity(a, 10)
        update_entity(b, 10)
    end
end

return game
