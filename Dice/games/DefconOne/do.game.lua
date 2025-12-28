-- Copyright (c) 2025 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local DURATION      = 2500
local HALF_DURATION = DURATION * 0.5
local CITY_COUNT    = 5

local gfx           = require('do.gfx')
local sfx           = require('do.sfx')

local game          = {
    cities       = {},
    cityTextures = { normal = 0, damaged = 1, destroyed = 2 }, ---@type { [string]: texture }

    guns         = {},
    gunTextures  = { left = 10, right = 11 }, ---@type { [string]: texture }
}

---@param engine engine
function game:on_setup(engine)
    engine:create_backgrounds(gfx.get_background(self, engine))
    engine:create_textures(gfx.get_textures(self, engine))
    engine:create_sounds(sfx.get_sounds(self, engine))

    for i = 1, CITY_COUNT do
        local sprite    = engine:create_sprite({ texture = self.cityTextures.normal, wrappable = false, collidable = true })
        sprite.position = {
            x = ((ScreenSize.width / CITY_COUNT) * i) - 40,
            y = ScreenSize.height / 5 * 4
        }
        self.cities[i]  = { sprite = sprite }
    end
    self.guns = {
        left = {
            sprite = engine:create_sprite({ texture = self.gunTextures.left, wrappable = false, collidable = false })
        },
        right = {
            sprite = engine:create_sprite({ texture = self.gunTextures.right, wrappable = false, collidable = false })
        }
    }
    self.guns.left.sprite.position = {
        x = 0,
        y = ScreenSize.height / 3 * 2
    }
    self.guns.right.sprite.position = {
        x = ScreenSize.width - 16,
        y = ScreenSize.height / 3 * 2
    }

    gfx.draw_dmd(engine.dmd, self)
end

---@param engine engine
function game:can_start_turn(engine)
    return true
end

---@param engine engine
function game:on_turn_start(engine)
end

---@param engine engine
---@param deltaTime number
function game:on_turn_update(engine, deltaTime)
    return GameStatus.Running
end

---@param engine engine
---@param spriteA sprite
---@param spriteB sprite
function game:on_collision(engine, spriteA, spriteB)
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

return game
