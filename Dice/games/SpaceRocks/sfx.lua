-- Copyright (c) 2025 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local sfx = {
}
---@param engine engine
function sfx.get_sounds(game, engine)
    return {
        [0] = engine.sfx:explosion(96324)
    }
end

return sfx
