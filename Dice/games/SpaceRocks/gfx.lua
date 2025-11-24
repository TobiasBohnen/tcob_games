-- Copyright (c) 2025 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local gfx = {}

---@param canvas canvas
---@param engine engine
function gfx.draw_background(game, engine, canvas, size)
    canvas:begin_path()
    canvas:rect({ x = 0, y = 0, width = size.width, height = size.height })
    canvas:fill_color(palette.Black)
    canvas:fill()

    local starCount = 75
    for i = 1, starCount do
        local x = engine:random(0, 1) * size.width
        local y = engine:random(0, 1) * size.height
        local s = engine:random(0, 1) * 2 + 1
        canvas:begin_path()
        canvas:rect({ x = x, y = y, width = s, height = s })
        canvas:fill_color(palette.White)
        canvas:fill()
    end
end

local function square(x)
    return { width = x, height = x }
end

function gfx.get_textures(game, engine)
    return {
        [game.ship.texture] = {
            size = square(32),
            draw = function(canvas) ---@param canvas canvas
                canvas:path_2d("M16,3 L29,29 L16,24 L3,29 Z")
                canvas:stroke_color(palette.Gray)
                canvas:stroke_width(2)
                canvas:stroke()
            end
        },
        [game.ship.hurtTexture] = {
            size = square(32),
            draw = function(canvas) ---@param canvas canvas
                canvas:path_2d("M16,3 L29,29 L16,24 L3,29 Z")
                canvas:fill_color(palette.Red)
                canvas:fill()
            end
        },
        [game.bulletTexture] = {
            size = square(8),
            draw = function(canvas) ---@param canvas canvas
                canvas:begin_path()
                canvas:rect({ x = 0, y = 0, width = 8, height = 8 })
                canvas:fill_color(palette.Orange)
                canvas:fill()
            end
        },

        [game.asteroidTextures.small] = {
            size = square(16),
            draw = function(canvas) ---@param canvas canvas
                canvas:path_2d("M8 1 L14 4 L16 9 L12 15 L6 16 L1 11 L1 6 Z")
                canvas:fill_color(palette.DarkBrown)
                canvas:fill()
            end
        },

        [game.asteroidTextures.medium] = {
            size = square(24),
            draw = function(canvas) ---@param canvas canvas
                canvas:path_2d("M12 1 L20 4 L23 10 L22 18 L14 22 L6 20 L2 12 L4 4 Z")
                canvas:fill_color(palette.DarkBrown)
                canvas:fill()
            end
        },

        [game.asteroidTextures.large] = {
            size = square(32),
            draw = function(canvas) ---@param canvas canvas
                canvas:path_2d("M16 1 L26 4 L31 10 L30 20 L24 28 L14 31 L4 28 L1 18 L2 8 Z")
                canvas:fill_color(palette.DarkBrown)
                canvas:fill()
            end
        },

        [game.explosionTexture] = {
            size = square(32),
            draw = function(canvas)
                canvas:begin_path()
                canvas:path_2d("M16 0 L20 8 L32 12 L22 18 L24 30 L16 25 L8 30 L10 18 L0 12 L12 8 Z")
                canvas:fill_color(palette.Orange)
                canvas:fill()

                canvas:stroke_color(palette.Red)
                canvas:stroke_width(2)
                canvas:stroke()
            end
        }
    }
end

function gfx.get_dmd(slot, value)
    local region
    local pattern

    if slot == "speed" then
        region = { x = 2, y = 20, width = 10, height = 10 }
        local patterns = {
            [1] = [[0000330000000033000000033330000000000000000000000000000000000000000000000000000000000000000000000000]],
            [2] = [[0000330000000033000000033330000033333300000000000000000000000000000000000000000000000000000000000000]],
            [3] = [[0000330000000033000000033330000033883300003888830000033330000000000000000000000000000000000000000000]],
            [4] = [[0003883000000388300000038830000033883300033888833003888888300333333330000000000000000000000000000000]],
            [5] = [[0003883000000388300000038830000033883300033888833003888888300388888830033888833000333333000000000000]],
            [6] = [[0003883000000388300000038830000033883300033888833033888888333888888883388888888333888888330333333330]],
        }
        pattern = patterns[value]
    elseif slot == "turn" then
        region = { x = 26, y = 20, width = 10, height = 10 }
        local patterns = {
            [1] = [[0000000000000000000000E00000000EE0000000EEEEEE00000EE000E00000E0000E0000000000E0000000000E000000000E]],
            [2] = [[0000000000000000000000000E00000000EE0000000EEEE0000000EE0E0000000E00E0000000000E000000000E000000000E]],
            [3] = [[000000000000000000000000000E00000000EE0000000EEEEE000000EE0E0000000E0E000000000E000000000E000000000E]],
            [4] = [[0000000000000000000000E000000000EE000000EEEEE00000E0EE000000E0E0000000E000000000E000000000E000000000]],
            [5] = [[000000000000000000000000E000000000EE0000000EEEE00000E0EE00000E00E00000E000000000E000000000E000000000]],
            [6] = [[0000000000000000000000E00000000EE0000000EEEEEE00000EE000E00000E0000E0000000000E0000000000E000000000E]],
        }
        pattern = patterns[value]
    elseif slot == "bullets" then
        region = { x = 50, y = 20, width = 3, height = 23 }
        local patterns = {
            [1] = [[000000000000000000000000000000080888080000000000000000000000000000000]],
            [2] = [[000000000000000000000080888080000000000080888080000000000000000000000]],
            [3] = [[000000000000080888080000000000080888080000000000080888080000000000000]],
            [4] = [[000080888080000000000080888080000000000080888080000000000080888080000]],
            [5] = [[000000080888080000080888080000080888080000080888080000080888080000000]],
            [6] = [[080888080000080888080000080888080000080888080000080888080000080888080]],
        }
        pattern = patterns[value]
    end

    if value == 0 then
        pattern = string.rep("0", region.width * region.height)
    end
    return region, pattern
end

return gfx
