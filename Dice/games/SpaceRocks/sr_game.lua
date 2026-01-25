-- Copyright (c) 2026 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local DURATION              = 2500
local HALF_DURATION         = DURATION * 0.5
local INIT_ASTEROID_COUNT   = 5
local EXPLOSION_DURATION    = 750
local MIN_BULLET_SPEED      = 180
local MIN_SPAWN_DISTANCE    = 80
local MIN_SPAWN_DISTANCE_SQ = MIN_SPAWN_DISTANCE * MIN_SPAWN_DISTANCE
local ASTEROID_SCORES       = { small = 100, medium = 50, large = 20 }

local gfx                   = require('sr_gfx')
local sfx                   = require('sr_sfx')

local game                  = {
    ship       = {},

    bullets    = {},

    asteroids  = {},

    explosions = {},

    sockets    = {
        speed   = nil, ---@type socket
        turn    = nil, ---@type socket
        bullets = nil, ---@type socket
    },
}

---@param engine engine
function game:on_setup(engine)
    gfx.create_background(engine)
    gfx.create_textures(self, engine)
    sfx.create_sounds(self, engine)

    self:create_ship(engine)

    while #self.asteroids < INIT_ASTEROID_COUNT do
        self:try_spawn_asteroid(engine)
    end

    self.sockets.speed   = socket.new { colors = { Palette.White } }
    self.sockets.turn    = socket.new { colors = { Palette.White } }
    self.sockets.bullets = socket.new { colors = { Palette.White, Palette.Red } }
    engine:update_hud()
end

---@param engine engine
function game:on_turn_start(engine)
    self.bulletTime = 0

    local ship = self.ship
    ship:turn_start()

    ship.bulletsLeft = self.sockets.bullets.die_value

    if socket.get_hand(self.sockets).value == "ThreeOfAKind" then
        ship:set_shield(true)
    end
    engine:update_hud()
end

---@param engine engine
---@param deltaTime number
function game:on_turn_update(engine, deltaTime, turnTime)
    if turnTime > DURATION then return GameStatus.Waiting end

    self:try_spawn_bullet(engine, deltaTime)

    for i = #self.asteroids, 1, -1 do self.asteroids[i]:update(i, deltaTime) end
    for i = #self.explosions, 1, -1 do self.explosions[i]:update(i, deltaTime) end
    for i = #self.bullets, 1, -1 do self.bullets[i]:update(i, deltaTime) end
    self.ship:update(deltaTime, turnTime)

    if self.ship.health == 0 then return GameStatus.GameOver end
    return GameStatus.Running
end

---@param engine engine
function game:on_turn_finish(engine)
    self:try_spawn_asteroid(engine)
    engine:update_hud()
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
---@param hud tex
function game:on_draw_hud(engine, hud)
    if engine.is_game_over then
        gf.draw_game_over(hud)
    else
        gfx.draw_hud(hud, self)
    end
end

------
------

function game:update_entity(e, deltaTime)
    local rad         = math.rad((e.direction or 0) - 90)
    local vx          = math.cos(rad) * e.speed / 1000
    local vy          = math.sin(rad) * e.speed / 1000

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
        speed          = engine:rnd(15, 30),
        size           = size,

        type           = "asteroid",
        markedForDeath = false,

        spriteInit     = {
            position = { x = x, y = y },
            texture  = gfx.textures.asteroid[size],
        },

        update         = function(a, i, deltaTime)
            if a.markedForDeath then
                if a.size == "small" then
                    self:create_explosion(engine, a)
                else
                    local newSize = a.size == "medium" and "small" or "medium"
                    local b       = a.sprite.center
                    local offset  = 8
                    self:create_asteroid(engine, newSize, b.x - offset, b.y - offset)
                    self:create_asteroid(engine, newSize, b.x + offset, b.y + offset)
                    engine:play_sound(sfx.sounds.explosion)
                end

                a.sprite:remove()
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
    asteroid.sprite                     = sprite.new(asteroid)
    self.asteroids[#self.asteroids + 1] = asteroid
end

---@param engine engine
function game:try_spawn_bullet(engine, deltaTime)
    local ship = self.ship
    if ship.bulletsLeft <= 0 then return end
    ship.bulletTime = ship.bulletTime - deltaTime
    if ship.bulletTime > 0 then return end

    self:create_bullet(engine)
end

---@param engine engine
function game:create_bullet(engine)
    local ship                      = self.ship

    local bullet                    = {
        direction  = ship.direction,
        speed      = math.max(MIN_BULLET_SPEED, ship.speed * 1.5),
        type       = "bullet",
        lifetime   = 0,
        spriteInit = {
            texture = gfx.textures.bullet,
        },

        update     = function(b, i, deltaTime)
            if b.lifetime < DURATION then
                self:update_entity(b, deltaTime)
            else
                b.sprite:remove()
                table.remove(self.bullets, i)
            end
            b.lifetime = b.lifetime + deltaTime
        end,
        collide    = function(bullet, b)
            if b.type == "asteroid" then
                bullet.lifetime = DURATION - 1
            end
        end
    }

    bullet.sprite                   = sprite.new(bullet)
    self.bullets[#self.bullets + 1] = bullet

    local shipSprite                = ship.sprite
    local shipCenter                = shipSprite.center
    local shipSize                  = shipSprite.size
    local bulletSize                = bullet.sprite.size
    local rad                       = math.rad(ship.direction)

    local bx                        =
        shipCenter.x
        + math.sin(rad) * shipSize.width * 0.5
        - bulletSize.width * 0.5

    local by                        =
        shipCenter.y
        - math.cos(rad) * shipSize.height * 0.5
        - bulletSize.height * 0.5

    bullet.sprite.position          = { x = bx, y = by }

    ship.bulletTime                 = HALF_DURATION / self.sockets.bullets.die_value
    ship.bulletsLeft                = ship.bulletsLeft - 1
    engine:play_sound(sfx.sounds.bullet, 1, false)
end

---@param engine engine
function game:create_explosion(engine, asteroid)
    local explosion                       = {
        lifetime   = 0,
        spriteInit = {
            position   = asteroid.sprite.position,
            texture    = gfx.textures.explosion,
            collidable = false,
        },

        update     = function(e, i, deltaTime)
            e.lifetime = e.lifetime + deltaTime
            if e.lifetime >= EXPLOSION_DURATION then
                e.sprite:remove()
                table.remove(self.explosions, i)
            end
        end
    }

    explosion.sprite                      = sprite.new(explosion)
    self.explosions[#self.explosions + 1] = explosion
    engine:play_sound(sfx.sounds.explosion)
end

---@param engine engine
function game:create_ship(engine)
    local ship = {
        direction     = 0,
        speed         = 0,
        speedTarget   = 0,
        sprite        = nil, ---@type sprite

        bulletsLeft   = 0,
        bulletTime    = 0,

        type          = "ship",
        health        = 5,
        shieldsUp     = false,
        hitByAsteroid = false,
        engineStall   = false,

        spriteInit    = {
            texture = 0,
            position = { x = ScreenSize.width / 2 - 12, y = ScreenSize.height / 2 - 12 }
        },

        turn_start    = function(ship)
            ship.speedTarget = self.sockets.speed.die_value * 30
            if ship.speedTarget == 0 then
                if ship.engineStall then
                    ship.health = ship.health - 1
                end
                ship.engineStall = true
            else
                ship.engineStall = false
            end

            local dirs          = { [0] = 0, [1] = -135, [2] = -90, [3] = -45, [4] = 45, [5] = 90, [6] = 135 }
            ship.turnStepsTotal = dirs[self.sockets.turn.die_value] / 45
            ship.turnStepsDone  = 0

            ship:set_shield(false)
            ship.hitByAsteroid = false
        end,

        set_shield    = function(ship, val)
            ship.shieldsUp      = val
            ship.sprite.texture = val and gfx.textures.hurtShip[ship.direction] or gfx.textures.ship[ship.direction]
        end,

        update        = function(ship, deltaTime, turnTime)
            local factor       = turnTime < HALF_DURATION
                and turnTime / HALF_DURATION
                or 1 - ((turnTime - HALF_DURATION) / HALF_DURATION)

            ship.speed         = ship.speedTarget * factor

            local stepsToApply = math.floor(math.abs(ship.turnStepsTotal) * factor + 0.5) - ship.turnStepsDone
            if stepsToApply > 0 then
                local sign         = ship.turnStepsTotal > 0 and 1 or -1
                ship.direction     = ship.direction + sign * 45 * stepsToApply
                ship.turnStepsDone = ship.turnStepsDone + stepsToApply
            end

            ship.direction      = ship.direction % 360
            ship.sprite.texture = ship.shieldsUp and gfx.textures.hurtShip[ship.direction] or gfx.textures.ship[ship.direction]
            self:update_entity(ship, deltaTime)
        end,

        collide       = function(ship, b)
            if ship.shieldsUp then return end
            if b.type == "asteroid" then
                ship.health        = ship.health - 1
                ship.hitByAsteroid = true
                ship:set_shield(true)
                engine:update_hud()
            end
        end
    }
    ship.sprite = sprite.new(ship)
    self.ship = ship
end

return game
