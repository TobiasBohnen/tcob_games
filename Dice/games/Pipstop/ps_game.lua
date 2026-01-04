-- Copyright (c) 2026 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local DURATION = 5500

local gfx      = require('ps_gfx')
local sfx      = require('ps_sfx')

local game     = {
    sockets  = {}, ---@type { [string]: socket }

    textures = {
    },

    sounds   = {
    },
}

---@param engine engine
function game:on_setup(engine)
    gfx.create_background(engine, 0, 0)
    gfx.create_textures(self, engine)
    engine:create_sounds(sfx.get_sounds(self, engine))
end

---@param engine engine
function game:on_turn_start(engine)

end

---@param engine engine
---@param deltaTime number
function game:on_turn_update(engine, deltaTime, turnTime)
    if turnTime >= DURATION then return GameStatus.TurnEnded end
    gfx.create_background(engine, (turnTime / DURATION) * 2 - 1, ((turnTime / DURATION) * 3) % 1)
    --  if self.ship.health == 0 then return GameStatus.GameOver end
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
function game:on_turn_finish(engine)

end

---@param engine engine
function game:on_teardown(engine)
    gfx.draw_game_over(engine.dmd, self)
end

---@param engine engine
function game:on_draw_dmd(engine)
    gfx.draw_dmd(engine.dmd, self)
end

------
------


return game
