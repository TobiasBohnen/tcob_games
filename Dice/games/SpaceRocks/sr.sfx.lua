-- Copyright (c) 2025 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local sfx = {
}
---@param engine engine
function sfx.get_sounds(game, engine)
    return {
        [game.explosionSound] = engine.sfxr:explosion(96324),
        [game.bulletSound] = engine.sfxr:laser_shoot(2478)
    }
end

return sfx
