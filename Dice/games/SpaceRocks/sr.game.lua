local DURATION            = 2500
local HALF_DURATION       = DURATION * 0.5
local INIT_ASTEROID_COUNT = 5
local EXPLOSION_DURATION  = 750

local gfx                 = require('sr.gfx')
local sfx                 = require('sr.sfx')

local game                = {
    ship             = {},

    shipTextures     = { [0] = 0, [45] = 1, [90] = 2, [135] = 3, [180] = 4, [225] = 5, [270] = 6, [315] = 7 },         --@type texture[]
    shipHurtTextures = { [0] = 10, [45] = 11, [90] = 12, [135] = 13, [180] = 14, [225] = 15, [270] = 16, [315] = 17 }, --@type texture[]

    bullets          = {},
    bulletTexture    = 20, ---@type texture
    bulletsLeft      = 0,
    bulletTime       = 0,
    bulletSound      = 2, ---@type sound

    asteroids        = {},
    asteroidTextures = { small = 30, medium = 31, large = 32 }, ---@type { [string]: texture }

    explosions       = {},
    explosionTexture = 40, ---@type texture
    explosionSound   = 1, ---@type sound

    slots            = {}, ---@type { [string]: slot }

    updateTime       = 0,

    powerup          = false
}

---@param engine engine
function game:on_setup(engine)
    engine:create_backgrounds(gfx.get_background(self, engine))
    engine:create_textures(gfx.get_textures(self, engine))
    engine:create_sounds(sfx.get_sounds(self, engine))

    self.ship                 = self:create_ship()
    self.ship.sprite          = engine:create_sprite(self.ship)
    self.ship.sprite.position = { x = ScreenSize.width / 2 - 12, y = ScreenSize.height / 2 - 12 }

    while #self.asteroids < INIT_ASTEROID_COUNT do
        self:try_spawn_asteroid(engine)
    end

    self.slots.speed   = engine:create_slot({})
    self.slots.turn    = engine:create_slot({})
    self.slots.bullets = engine:create_slot({})

    engine:create_dice(4, { { values = { 1, 2, 3, 4, 5, 6 }, color = Palette.White } }) -- { 1, 1, 1, 1, 1, 1 }
    engine:roll_dice()
    gfx.draw_dmd(engine.dmd, self)
end

---@param engine engine
function game:can_start_turn(engine)
    local s = self.slots
    return not (s.speed.is_empty or s.turn.is_empty or s.bullets.is_empty)
end

---@param engine engine
function game:on_turn_start(engine)
    self.updateTime = 0
    self.bulletTime = 0

    self:try_spawn_asteroid(engine)

    local ship                = self.ship
    ship.linearVelocityTarget = self.slots.speed.die_value * 30

    local dirs                = { -135, -90, -45, 45, 90, 135 }
    local directionTarget     = dirs[self.slots.turn.die_value]
    ship.turnStepsTotal       = directionTarget / 45
    ship.turnStepsDone        = 0

    self.bulletsLeft          = self.slots.bullets.die_value

    if engine:get_hand(self.slots).value == "ThreeOfAKind" then
        ship:set_invulnerable(true)
        self.powerup = true
        gfx.draw_dmd(engine.dmd, self)
    end
end

---@param engine engine
---@param deltaTime number
function game:on_turn_update(engine, deltaTime)
    self.updateTime = self.updateTime + deltaTime
    if self.updateTime >= DURATION then return GameStatus.TurnEnded end

    self:try_spawn_bullet(engine, deltaTime)

    self:update_asteroids(engine, deltaTime)
    self:update_explosions(engine, deltaTime)
    self:update_bullets(engine, deltaTime)
    self:update_ship(self.ship, deltaTime)

    if self.ship.health == 0 then return GameStatus.GameOver end
    return GameStatus.Running
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
            ship.health = ship.health - 1
            ship:set_invulnerable(true)
            gfx.draw_dmd(engine.dmd, self)
        end

        local asteroid          = first.type == "asteroid" and first or second
        asteroid.markedForDeath = true
    elseif key == "asteroid_bullet" then
        local asteroid          = first.type == "asteroid" and first or second
        local bullet            = first.type == "bullet" and first or second
        asteroid.markedForDeath = true
        bullet.lifetime         = DURATION - 1
    end
end

---@param engine engine
---@param slot slot
function game:on_die_change(engine, slot)
    gfx.draw_dmd(engine.dmd, self)
end

---@param engine engine
function game:on_die_motion(engine)
    gfx.draw_dmd(engine.dmd, self)
end

---@param engine engine
function game:on_turn_finish(engine)
    engine:reset_slots(self.slots)

    self.powerup = false
    self.ship:set_invulnerable(false)
    gfx.draw_dmd(engine.dmd, self)
end

---@param engine engine
function game:on_teardown(engine)
    gfx.draw_game_over(engine.dmd, self)
end

------
------

function game:update_ship(ship, deltaTime)
    local factor = self.updateTime < HALF_DURATION
        and self.updateTime / HALF_DURATION
        or 1 - ((self.updateTime - HALF_DURATION) / HALF_DURATION)

    ship:update(factor)
    ship.sprite.texture = ship.invulnerable and self.shipHurtTextures[ship.direction] or self.shipTextures[ship.direction]

    self:update_entity(ship, deltaTime)
end

function game:create_ship()
    return {
        direction            = 0,
        directionTarget      = 0,
        linearVelocity       = 0,
        linearVelocityTarget = 0,
        sprite               = nil, ---@type sprite
        texture              = 0,
        type                 = "ship",
        health               = 5,
        invulnerable         = false,

        set_invulnerable     = function(ship, val)
            ship.invulnerable   = val
            ship.sprite.texture = val and self.shipHurtTextures[ship.direction] or self.shipTextures[ship.direction]
        end,

        update               = function(ship, factor)
            ship.linearVelocity = ship.linearVelocityTarget * factor

            local stepsTodo     = math.floor(math.abs(ship.turnStepsTotal) * factor + 0.0001)
            local stepsToApply  = stepsTodo - ship.turnStepsDone
            if stepsToApply > 0 then
                local sign         = ship.turnStepsTotal > 0 and 1 or -1
                ship.direction     = ship.direction + sign * 45 * stepsToApply
                ship.turnStepsDone = ship.turnStepsDone + stepsToApply
            end

            ship.direction = ship.direction % 360
        end
    }
end

function game:update_explosions(engine, deltaTime)
    for i = #self.explosions, 1, -1 do
        local e    = self.explosions[i]
        local s    = e.sprite

        e.lifetime = e.lifetime + deltaTime
        if e.lifetime >= EXPLOSION_DURATION then
            engine:remove_sprite(s)
            table.remove(self.explosions, i)
        end
    end
end

function game:update_entity(e, deltaTime)
    local rad         = math.rad((e.direction or 0) - 90)
    local vx          = math.cos(rad) * e.linearVelocity / 1000
    local vy          = math.sin(rad) * e.linearVelocity / 1000

    local pos         = e.sprite.position
    e.sprite.position = { x = (pos.x + vx * deltaTime) % ScreenSize.width, y = (pos.y + vy * deltaTime) % ScreenSize.height }
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
function game:try_spawn_bullet(engine, deltaTime)
    if self.bulletsLeft <= 0 then return end
    self.bulletTime = self.bulletTime - deltaTime
    if self.bulletTime > 0 then return end

    local ship                      = self.ship

    local bullet                    = {
        direction      = ship.direction,
        linearVelocity = math.max(180, ship.linearVelocity * 1.5),
        texture        = self.bulletTexture,
        type           = "bullet",
        lifetime       = 0,
    }

    bullet.sprite                   = engine:create_sprite(bullet)
    self.bullets[#self.bullets + 1] = bullet

    local shipSprite                = ship.sprite
    local shipBounds                = shipSprite.bounds
    local bulletBounds              = bullet.sprite.bounds
    local rad                       = math.rad(ship.direction)

    local bx                        =
        shipBounds.x + shipBounds.width * 0.5
        + math.sin(rad) * shipBounds.width * 0.5
        - bulletBounds.width * 0.5

    local by                        =
        shipBounds.y + shipBounds.height * 0.5
        - math.cos(rad) * shipBounds.height * 0.5
        - bulletBounds.height * 0.5

    bullet.sprite.position          = { x = bx, y = by }

    self.bulletTime                 = HALF_DURATION / self.slots.bullets.die_value
    self.bulletsLeft                = self.bulletsLeft - 1

    engine:play_sound(self.bulletSound)
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
                local b       = a.sprite.bounds
                local cx      = b.x + b.width * 0.5
                local cy      = b.y + b.height * 0.5
                local offset  = 8
                self:spawn_asteroid(engine, newSize, cx - offset, cy - offset)
                self:spawn_asteroid(engine, newSize, cx + offset, cy + offset)
            end

            engine:remove_sprite(a.sprite)
            table.remove(self.asteroids, i)
            engine:play_sound(self.explosionSound)
            engine:give_score(100)
        else
            self:update_entity(a, deltaTime)
        end
    end
end

---@param engine engine
function game:try_spawn_asteroid(engine)
    local count = #self.asteroids
    if count >= INIT_ASTEROID_COUNT then return end

    local x = engine:random(0, ScreenSize.width)
    local y = engine:random(0, ScreenSize.height)

    local sx, sy = self.ship.sprite.position.x, self.ship.sprite.position.y
    if math.sqrt((sx - x) ^ 2 + (sy - y) ^ 2) < 100 then return end

    self:spawn_asteroid(engine, "large", x, y)
end

---@param engine engine
function game:spawn_asteroid(engine, size, x, y)
    local asteroid                      = {
        direction      = engine:random(0, 359),
        linearVelocity = engine:random(15, 30),
        size           = size,
        texture        = self.asteroidTextures[size],
        type           = "asteroid",
        markedForDeath = false,
    }
    asteroid.sprite                     = engine:create_sprite(asteroid)
    asteroid.sprite.position            = { x = x, y = y }
    self.asteroids[#self.asteroids + 1] = asteroid
end

return game
