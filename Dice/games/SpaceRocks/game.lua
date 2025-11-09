-- Copyright (c) 2025 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

------

local function update_entity(e, deltaTime)
    local rad = math.rad(e.rotation - 90)
    local vx = math.cos(rad) * e.velocity / 1000
    local vy = math.sin(rad) * e.velocity / 1000

    e.x = (e.x + vx * deltaTime) % 1
    e.y = (e.y + vy * deltaTime) % 1

    e.sprite:position(e)
    e.sprite:rotation(e.rotation)
end

------

local game = {
    --public:
    Gfx = require('gfx'),
    Sfx = require('sfx'),

    --private:
    ship = {
        x = 0,
        y = 0,
        rotation = 0,
        velocity = 0,
        sprite = nil, ---@type sprite
    },

    bullets = {},

    asteroids = {},

    slots = {}, ---@type slots

    updateTime = 0,

    active = true,
}

---@param engine engine
function game:on_setup(engine)
    self.ship.x = 0.5
    self.ship.y = 0.5
    self.ship.rotation = 0
    self.ship.velocity = 0
    self.ship.sprite = engine:create_sprite(self.ship, self.Gfx.shipTexture)

    for i = 1, 5 do
        local sz = "large"
        local asteroid = {
            x = engine:random(0.0, 1.0),
            y = engine:random(0.0, 1.0),
            rotation = engine:random(0, 359),
            velocity = engine:random(0.01, 0.15),
            size = sz
        }
        asteroid.sprite = engine:create_sprite(asteroid, self.Gfx.asteroidTextures[sz])
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

    for _, a in ipairs(self.asteroids) do update_entity(a, deltaTime) end
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

return game
