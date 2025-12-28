-- Copyright (c) 2025 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local DURATION      = 2500
local HALF_DURATION = DURATION * 0.5
local CITY_COUNT    = 5

local gfx           = require('do.gfx')
local sfx           = require('do.sfx')

local game          = {
    cities         = {},
    cityTextures   = { normal = 0, damaged = 1, destroyed = 2 }, ---@type { [string]: texture }

    guns           = {},
    gunTextures    = { left = 10, right = 11 }, ---@type { [string]: texture }

    missiles       = {},
    missileTexture = 20,

    updateTime     = 0,
}

---@param engine engine
function game:on_setup(engine)
    engine:create_backgrounds(gfx.get_background(self, engine))
    engine:create_textures(gfx.get_textures(self, engine))
    engine:create_sounds(sfx.get_sounds(self, engine))

    for i = 1, CITY_COUNT do
        local city     = {
            type = "city",
            spriteInit = {
                position = {
                    x = ((ScreenSize.width / CITY_COUNT) * i) - 40,
                    y = ScreenSize.height / 5 * 4
                },
                texture = self.cityTextures.normal,
                wrappable = false,
                collidable = true,
            },
        }
        local sprite   = engine:create_sprite(city)
        city.sprite    = sprite
        city.center    = {
            x = sprite.position.x + sprite.size.width / 2,
            y = sprite.position.y + sprite.size.height / 2
        }
        self.cities[i] = city
    end
    self.guns = {
        left = {
            sprite = engine:create_sprite({
                spriteInit = {
                    texture = self.gunTextures.left,
                    position = { x = 0, y = ScreenSize.height / 3 * 2 },
                    wrappable = false,
                    collidable = false
                }
            })
        },
        right = {
            sprite = engine:create_sprite({
                spriteInit = {
                    texture = self.gunTextures.right,
                    position = { x = ScreenSize.width - 16, y = ScreenSize.height / 3 * 2 },
                    wrappable = false,
                    collidable = false
                }
            })
        }
    }

    gfx.draw_dmd(engine.dmd, self)
end

---@param engine engine
function game:can_start_turn(engine)
    return true
end

---@param engine engine
function game:on_turn_start(engine)
    self:try_spawn_missile(engine)
end

---@param engine engine
---@param deltaTime number
function game:on_turn_update(engine, deltaTime, updateTime)
    if updateTime >= DURATION then return GameStatus.TurnEnded end

    for i = #self.missiles, 1, -1 do
        local m = self.missiles[i]
        self:update_missile(m, deltaTime)
    end

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

    print(key)
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
    gfx.draw_dmd(engine.dmd, self)
end

---@param engine engine
function game:on_teardown(engine)
    gfx.draw_game_over(engine.dmd, self)
end

------
------

function game:update_missile(e, deltaTime)
    local rad         = e.direction
    local vx          = math.cos(rad) * e.linearVelocity / 1000
    local vy          = math.sin(rad) * e.linearVelocity / 1000

    local pos         = e.sprite.position
    e.sprite.position = { x = (pos.x + vx * deltaTime) % ScreenSize.width, y = (pos.y + vy * deltaTime) % ScreenSize.height }
end

---@param engine engine
function game:try_spawn_missile(engine)
    local count = #self.missiles
    if count >= CITY_COUNT * 2 then return end

    local missile     = {
        target = engine:irnd(1, #self.cities),
        linearVelocity = engine:rnd(15, 30),
        type = "missile",
        spriteInit = {
            position   = { x = engine:rnd(0, ScreenSize.width), y = 0 },
            texture    = self.missileTexture,
            wrappable  = false,
            collidable = true,
        },
    }
    local target      = self.cities[missile.target]

    missile.sprite    = engine:create_sprite(missile)
    missile.direction = math.atan(target.center.y - missile.sprite.position.y - 4, target.center.x - missile.sprite.position.x - 4)


    self.missiles[#self.missiles + 1] = missile
end

return game
