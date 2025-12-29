-- Copyright (c) 2025 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local DURATION              = 2500
local HALF_DURATION         = DURATION * 0.5
local INIT_ASTEROID_COUNT   = 5
local EXPLOSION_DURATION    = 750
local MIN_BULLET_SPEED      = 180
local MIN_SPAWN_DISTANCE    = 80
local MIN_SPAWN_DISTANCE_SQ = MIN_SPAWN_DISTANCE * MIN_SPAWN_DISTANCE
local ASTEROID_SCORES       = {
    small = 100,
    medium = 50,
    large = 20
}

local gfx                   = require('sr.gfx')
local sfx                   = require('sr.sfx')

local game                  = {
    ship        = {},

    bullets     = {},
    bulletsLeft = 0,
    bulletTime  = 0,

    asteroids   = {},

    explosions  = {},

    sockets     = {}, ---@type { [string]: socket }

    textures    = {
        ship      = { [0] = 0, [45] = 1, [90] = 2, [135] = 3, [180] = 4, [225] = 5, [270] = 6, [315] = 7 }, ---@type { [number]: texture }
        hurtShip  = { [0] = 10, [45] = 11, [90] = 12, [135] = 13, [180] = 14, [225] = 15, [270] = 16, [315] = 17 }, ---@type { [number]: texture }
        bullet    = 20, ---@type texture
        asteroid  = { small = 30, medium = 31, large = 32 }, ---@type { [string]: texture }
        explosion = 40, ---@type texture
    },

    sounds      = {
        explosion = 1, ---@type sound
        bullet = 2, ---@type sound
    },

    powerup     = false,
}

---@param engine engine
function game:on_setup(engine)
    engine:create_backgrounds(gfx.get_background(self, engine))
    engine:create_textures(gfx.get_textures(self, engine))
    engine:create_sounds(sfx.get_sounds(self, engine))

    self:create_ship(engine)

    while #self.asteroids < INIT_ASTEROID_COUNT do
        self:try_spawn_asteroid(engine)
    end

    self.sockets.speed   = engine:create_socket { colors = { Palette.White } }
    self.sockets.turn    = engine:create_socket { colors = { Palette.White } }
    self.sockets.bullets = engine:create_socket { colors = { Palette.White, Palette.Red } }

    gfx.draw_dmd(engine.dmd, self)
end

---@param engine engine
function game:can_start_turn(engine)
    local s = self.sockets
    return not (s.speed.is_empty or s.turn.is_empty or s.bullets.is_empty)
end

---@param engine engine
function game:on_turn_start(engine)
    self.bulletTime           = 0

    local ship                = self.ship
    ship.linearVelocityTarget = self.sockets.speed.die_value * 30

    local dirs                = { -135, -90, -45, 45, 90, 135 }
    ship.turnStepsTotal       = dirs[self.sockets.turn.die_value] / 45
    ship.turnStepsDone        = 0

    self.bulletsLeft          = self.sockets.bullets.die_value

    if engine:get_hand(self.sockets).value == "ThreeOfAKind" then
        ship:set_invulnerable(true)
        self.powerup = true
        gfx.draw_dmd(engine.dmd, self)
    end
end

---@param engine engine
---@param deltaTime number
function game:on_turn_update(engine, deltaTime, turnTime)
    if turnTime >= DURATION then return GameStatus.TurnEnded end

    self:try_spawn_bullet(engine, deltaTime)

    for i = #self.asteroids, 1, -1 do
        self.asteroids[i]:update(i, deltaTime)
    end
    for i = #self.explosions, 1, -1 do
        self.explosions[i]:update(i, deltaTime)
    end
    for i = #self.bullets, 1, -1 do
        self.bullets[i]:update(i, deltaTime)
    end
    self.ship:update(deltaTime, turnTime)

    engine.ssd = tostring(#self.asteroids)
    if self.ship.health == 0 then return GameStatus.GameOver end
    return GameStatus.Running
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
    self.powerup = false
    self.ship:set_invulnerable(false)
    self:try_spawn_asteroid(engine)
    gfx.draw_dmd(engine.dmd, self)
end

---@param engine engine
function game:on_teardown(engine)
    gfx.draw_game_over(engine.dmd, self)
end

------
------

function game:update_entity(e, deltaTime)
    local rad         = math.rad((e.direction or 0) - 90)
    local vx          = math.cos(rad) * e.linearVelocity / 1000
    local vy          = math.sin(rad) * e.linearVelocity / 1000

    local pos         = e.sprite.position
    e.sprite.position = { x = (pos.x + vx * deltaTime) % ScreenSize.width, y = (pos.y + vy * deltaTime) % ScreenSize.height }
end

---@param engine engine
function game:try_spawn_asteroid(engine)
    local count = #self.asteroids
    if count >= INIT_ASTEROID_COUNT * 2 then return end

    local x, y
    local maxX, maxY = ScreenSize.width - gfx.largeAsteroidSize, ScreenSize.height - gfx.largeAsteroidSize

    local edge = engine:irnd(1, 4)
    if edge == 1 then
        x = engine:irnd(0, maxX)
        y = 0
    elseif edge == 2 then
        x = maxX
        y = engine:irnd(0, maxY)
    elseif edge == 3 then
        x = engine:irnd(0, maxX)
        y = maxY
    elseif edge == 4 then
        x = 0
        y = engine:irnd(0, maxY)
    end

    local sx, sy = self.ship.sprite.position.x, self.ship.sprite.position.y
    local dx, dy = sx - x, sy - y
    if dx * dx + dy * dy < MIN_SPAWN_DISTANCE_SQ then return end

    self:create_asteroid(engine, "large", x, y)
    engine.ssd = tostring(#self.asteroids)
end

---@param engine engine
function game:create_asteroid(engine, size, x, y)
    local asteroid                      = {
        direction      = engine:rnd(0, 359),
        linearVelocity = engine:rnd(15, 30),
        size           = size,

        type           = "asteroid",
        markedForDeath = false,

        spriteInit     = {
            position = { x = x, y = y },
            texture  = self.textures.asteroid[size],
        },

        update         = function(a, i, deltaTime)
            if a.markedForDeath then
                if a.size == "small" then
                    self:create_explosion(engine, a)
                else
                    local newSize = a.size == "medium" and "small" or "medium"
                    local b       = a.sprite.bounds
                    local cx      = b.x + b.width * 0.5
                    local cy      = b.y + b.height * 0.5
                    local offset  = 8
                    self:create_asteroid(engine, newSize, cx - offset, cy - offset)
                    self:create_asteroid(engine, newSize, cx + offset, cy + offset)
                    engine:play_sound(self.sounds.explosion)
                end

                engine:remove_sprite(a.sprite)
                table.remove(self.asteroids, i)

                engine:give_score(ASTEROID_SCORES[a.size])
                engine.ssd = tostring(#self.asteroids)
            else
                self:update_entity(a, deltaTime)
            end
        end,

        collide        = function(a, b)
            if b.type == "bullet" or b.type == "ship" then
                a.markedForDeath = true
            end
        end
    }
    asteroid.sprite                     = engine:create_sprite(asteroid)
    self.asteroids[#self.asteroids + 1] = asteroid
end

---@param engine engine
function game:try_spawn_bullet(engine, deltaTime)
    if self.bulletsLeft <= 0 then return end
    self.bulletTime = self.bulletTime - deltaTime
    if self.bulletTime > 0 then return end

    self:create_bullet(engine)
end

---@param engine engine
function game:create_bullet(engine)
    local ship                      = self.ship

    local bullet                    = {
        direction      = ship.direction,
        linearVelocity = math.max(MIN_BULLET_SPEED, ship.linearVelocity * 1.5),
        type           = "bullet",
        lifetime       = 0,
        spriteInit     = {
            texture = self.textures.bullet,
        },

        update         = function(b, i, deltaTime)
            if b.lifetime < DURATION then
                self:update_entity(b, deltaTime)
            else
                engine:remove_sprite(b.sprite)
                table.remove(self.bullets, i)
            end
            b.lifetime = b.lifetime + deltaTime
        end,
        collide        = function(bullet, b)
            if b.type == "asteroid" then
                bullet.lifetime = DURATION - 1
            end
        end
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

    self.bulletTime                 = HALF_DURATION / self.sockets.bullets.die_value
    self.bulletsLeft                = self.bulletsLeft - 1
    engine:play_sound(self.sounds.bullet)
end

---@param engine engine
function game:create_explosion(engine, asteroid)
    local explosion                       = {
        lifetime   = 0,
        spriteInit = {
            position   = asteroid.sprite.position,
            texture    = self.textures.explosion,
            collidable = false,
        },

        update     = function(e, i, deltaTime)
            e.lifetime = e.lifetime + deltaTime
            if e.lifetime >= EXPLOSION_DURATION then
                engine:remove_sprite(e.sprite)
                table.remove(self.explosions, i)
            end
        end
    }

    explosion.sprite                      = engine:create_sprite(explosion)
    self.explosions[#self.explosions + 1] = explosion
    engine:play_sound(self.sounds.explosion)
end

---@param engine engine
function game:create_ship(engine)
    local ship = {
        direction            = 0,
        linearVelocity       = 0,
        linearVelocityTarget = 0,
        sprite               = nil, ---@type sprite

        type                 = "ship",
        health               = 5,
        invulnerable         = false,

        spriteInit           = {
            texture = 0,
            position = { x = ScreenSize.width / 2 - 12, y = ScreenSize.height / 2 - 12 }
        },

        set_invulnerable     = function(ship, val)
            ship.invulnerable   = val
            ship.sprite.texture = val and self.textures.hurtShip[ship.direction] or self.textures.ship[ship.direction]
        end,

        update               = function(ship, deltaTime, updateTime)
            local factor        = updateTime < HALF_DURATION
                and updateTime / HALF_DURATION
                or 1 - ((updateTime - HALF_DURATION) / HALF_DURATION)

            ship.linearVelocity = ship.linearVelocityTarget * factor

            local stepsToApply  = math.floor(math.abs(ship.turnStepsTotal) * factor + 0.5) - ship.turnStepsDone
            if stepsToApply > 0 then
                local sign         = ship.turnStepsTotal > 0 and 1 or -1
                ship.direction     = ship.direction + sign * 45 * stepsToApply
                ship.turnStepsDone = ship.turnStepsDone + stepsToApply
            end

            ship.direction = ship.direction % 360
            ship.sprite.texture = ship.invulnerable and self.textures.hurtShip[ship.direction] or self.textures.ship[ship.direction]
            self:update_entity(ship, deltaTime)
        end,

        collide              = function(ship, b)
            if ship.invulnerable then return end
            if b.type == "asteroid" then
                ship.health = ship.health - 1
                ship:set_invulnerable(true)
                gfx.draw_dmd(engine.dmd, self)
            end
        end
    }
    ship.sprite = engine:create_sprite(ship)
    self.ship = ship
end

return game
