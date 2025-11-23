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
    canvas:fill_color("black")
    canvas:fill()

    local starCount = 75
    for i = 1, starCount do
        local x = engine:random(0, 1) * size.width
        local y = engine:random(0, 1) * size.height
        local s = engine:random(0, 1) * 2 + 1
        canvas:begin_path()
        canvas:rect({ x = x, y = y, width = s, height = s })
        canvas:fill_color("white")
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
                canvas:stroke_color("lightgray")
                canvas:stroke_width(2)
                canvas:stroke()
            end
        },
        [game.ship.hurtTexture] = {
            size = square(32),
            draw = function(canvas) ---@param canvas canvas
                canvas:path_2d("M16,3 L29,29 L16,24 L3,29 Z")
                canvas:fill_color("red")
                canvas:fill()
            end
        },
        [game.bulletTexture] = {
            size = square(8),
            draw = function(canvas) ---@param canvas canvas
                canvas:begin_path()
                canvas:rect({ x = 0, y = 0, width = 8, height = 8 })
                canvas:fill_color("gold")
                canvas:fill()
            end
        },

        [game.asteroidTextures.small] = {
            size = square(16),
            draw = function(canvas) ---@param canvas canvas
                canvas:path_2d("M8 1 L14 4 L16 9 L12 15 L6 16 L1 11 L1 6 Z")
                canvas:fill_color("brown")
                canvas:fill()
            end
        },

        [game.asteroidTextures.medium] = {
            size = square(24),
            draw = function(canvas) ---@param canvas canvas
                canvas:path_2d("M12 1 L20 4 L23 10 L22 18 L14 22 L6 20 L2 12 L4 4 Z")
                canvas:fill_color("brown")
                canvas:fill()
            end
        },

        [game.asteroidTextures.large] = {
            size = square(32),
            draw = function(canvas) ---@param canvas canvas
                canvas:path_2d("M16 1 L26 4 L31 10 L30 20 L24 28 L14 31 L4 28 L1 18 L2 8 Z")
                canvas:fill_color("brown")
                canvas:fill()
            end
        },

        [game.explosionTexture] = {
            size = square(32),
            draw = function(canvas)
                canvas:begin_path()
                canvas:path_2d("M16 0 L20 8 L32 12 L22 18 L24 30 L16 25 L8 30 L10 18 L0 12 L12 8 Z")
                canvas:fill_color("orange")
                canvas:fill()

                canvas:stroke_color("red")
                canvas:stroke_width(2)
                canvas:stroke()
            end
        }
    }
end

function gfx.get_dmd(slot, value)
    local region
    local pattern

    if value == 0 then
        pattern = string.rep("0", 100)
    end

    if slot == "speed" then
        region = { x = 2, y = 20, width = 10, height = 10 }
        local patterns = {
            [1] = [[
0000990000
0000000000
0000000000
0000000000
0000000000
0000000000
0000000000
0000000000
0000000000
0000000000
]],
            [2] = [[
0000990000
0000990000
0000000000
0000000000
0000000000
0000000000
0000000000
0000000000
0000000000
0000000000
]],
            [3] = [[
0000990000
0000990000
0009999000
0009999000
0000000000
0000000000
0000000000
0000000000
0000000000
0000000000
]],
            [4] = [[
0000990000
0000990000
0009999000
0009999000
0099999900
0099999900
0000000000
0000000000
0000000000
0000000000
]],
            [5] = [[
0000990000
0000990000
0009999000
0009999000
0099999900
0099999900
0999999990
0999999990
0000000000
0000000000
]],
            [6] = [[
0000990000
0000990000
0009999000
0009999000
0099999900
0099999900
0999999990
0999999990
9999999999
9999999999
]],
        }
        pattern = patterns[value] or pattern
    elseif slot == "turn" then
        region = { x = 26, y = 20, width = 10, height = 10 }
        local patterns = {
            [1] = [[
0000000000
0000000000
0000000000
00D0000000
0D00000000
DDDDDDDDDD
0D00000000
00D0000000
0000000000
0000000000
]],
            [2] = [[
DDDD000000
DD00000000
D0D0000000
D00D000000
0000D00000
00000D0000
000000D000
0000000D00
00000000D0
000000000D
]],
            [3] = [[
00D0000000
00DD000000
0DDDD00000
000D000000
000D000000
0000D00000
0000D00000
0000D00000
00000D0000
00000D0000
]],
            [4] = [[
0000000D00
000000DD00
00000DDDD0
000000D000
000000D000
00000D0000
00000D0000
00000D0000
0000D00000
0000D00000
]],
            [5] = [[
000000DDDD
00000000DD
0000000D0D
000000D00D
00000D0000
0000D00000
000D000000
000D000000
00D0000000
0D00000000
]],
            [6] = [[
0000000000
0000000000
0000000000
0000000D00
0000000DD0
DDDDDDDDDD
0000000DD0
0000000D00
0000000000
0000000000
]],
        }
        pattern = patterns[value] or pattern
    elseif slot == "bullets" then
        region = { x = 50, y = 20, width = 10, height = 10 }
        local patterns = {
            [1] = [[
0000000000
0000000000
0000000000
0000000000
0000550000
0000000000
0000000000
0000000000
0000000000
0000000000
]],
            [2] = [[
0000000000
0000000000
0000000000
0000000000
0000550000
0000000000
0000550000
0000000000
0000000000
0000000000
]],
            [3] = [[
0000000000
0000000000
0000550000
0000000000
0000550000
0000000000
0000550000
0000000000
0000000000
0000000000
]],
            [4] = [[
0000000000
0000000000
0000550000
0000000000
0000550000
0000000000
0000550000
0000000000
0000550000
0000000000
]],
            [5] = [[
0000550000
0000000000
0000550000
0000000000
0000550000
0000000000
0000550000
0000000000
0000550000
0000000000
]],
            [6] = [[
0000550000
0000550000
0000550000
0000550000
0000550000
0000550000
0000550000
0000550000
0000550000
0000550000
]],
        }
        pattern = patterns[value] or pattern
    end

    return region, pattern
end

return gfx
