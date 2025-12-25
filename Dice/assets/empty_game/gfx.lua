-- Copyright (c) 2025 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local gfx = {}

---@param engine engine
function gfx.get_background(game, engine)
    return {
        [0] = {
            bitmap = ""
        }
    }
end

---@param engine engine
function gfx.get_textures(game, engine)
    return {
    }
end

------

---@param dmd dmd
function gfx.draw_dmd(dmd, game)

end

return gfx
