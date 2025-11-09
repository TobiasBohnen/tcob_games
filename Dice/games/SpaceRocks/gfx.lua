-- Copyright (c) 2025 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local gfx = {
    shipTexture = 0, ---@type texture
    bulletTexture = 1, ---@type texture
    asteroidTextures = { small = 2, medium = 3, large = 4 } ---@type { [string]: texture }
}

---@param canvas canvas
function gfx.draw_background(engine, canvas, size)
    canvas:begin_path()
    canvas:rect({ x = 0, y = 0, width = size.width, height = size.height })
    canvas:fill_color("black")
    canvas:fill()
    -- Draw stars
    local starCount = 200 -- adjust number of stars
    for i = 1, starCount do
        local x = engine:random(0, 1) * size.width
        local y = engine:random(0, 1) * size.height
        local s = engine:random(0, 1) * 2 + 1 -- small square size
        canvas:begin_path()
        canvas:rect({ x = x, y = y, width = s, height = s })
        canvas:fill_color("white")
        canvas:fill()
    end
end

function gfx:get_textures(engine)
    return {
        [self.shipTexture] = {
            Size = { width = 32, height = 32 },
            Draw = function(canvas) ---@param canvas canvas
                canvas:path_2d("M16,3 L29,29 L16,24 L3,29 Z")
                canvas:stroke_color("red")
                canvas:stroke_width(2)
                canvas:stroke()
            end
        },

        [self.bulletTexture] = {
            Size = { width = 8, height = 8 },
            Draw = function(canvas) ---@param canvas canvas
                canvas:begin_path()
                canvas:rect({ x = 0, y = 0, width = 8, height = 8 })
                canvas:fill_color("red")
                canvas:fill()
            end
        },

        [self.asteroidTextures.small] = {
            Size = { width = 16, height = 16 },
            Draw = function(canvas) ---@param canvas canvas
                canvas:path_2d("M8 1 L14 4 L16 9 L12 15 L6 16 L1 11 L1 6 Z")
                canvas:stroke_color("red")
                canvas:stroke_width(2)
                canvas:stroke()
            end
        },

        [self.asteroidTextures.medium] = {
            Size = { width = 24, height = 24 },
            Draw = function(canvas) ---@param canvas canvas
                canvas:path_2d("M12 1 L20 4 L23 10 L22 18 L14 22 L6 20 L2 12 L4 4 Z")
                canvas:stroke_color("red")
                canvas:stroke_width(2)
                canvas:stroke()
            end
        },

        [self.asteroidTextures.large] = {
            Size = { width = 32, height = 32 },
            Draw = function(canvas) ---@param canvas canvas
                canvas:path_2d("M16 1 L26 4 L31 10 L30 20 L24 28 L14 31 L4 28 L1 18 L2 8 Z")
                canvas:stroke_color("red")
                canvas:stroke_width(2)
                canvas:stroke()
            end
        },
    }
end

return gfx
