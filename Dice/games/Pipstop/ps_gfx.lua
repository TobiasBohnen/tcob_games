-- Copyright (c) 2026 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local gfx              = {}

local HORIZON_HEIGHT   = 100

local SKY_COLOR        = Palette.Blue
local GROUND_COLOR     = Palette.Green
local ROAD_COLOR       = Palette.Gray
local CENTERLINE_COLOR = Palette.White
local CURB_COLOR1      = Palette.Red
local CURB_COLOR2      = Palette.White

---@param engine engine
function gfx.create_background(engine, curveAmount, trackOffset)
    local bg   = engine.bg
    local size = bg.size
    local w, h = size.width, size.height

    -- sky
    bg:rect({ x = 0, y = 0, width = w, height = HORIZON_HEIGHT }, SKY_COLOR, true)

    -- ground
    bg:rect({ x = 0, y = HORIZON_HEIGHT, width = w, height = h - HORIZON_HEIGHT }, GROUND_COLOR, true)

    -- road
    for y = HORIZON_HEIGHT, h - 1 do
        local t             = (y - HORIZON_HEIGHT) / (h - HORIZON_HEIGHT)
        local roadWidth     = math.floor(20 + t * (w / 3 * 2))

        local oneMinusT     = 1.0 - t
        local curveStrength = oneMinusT * oneMinusT * oneMinusT
        local curveOffset   = math.floor(curveAmount * curveStrength * (w / 2))

        local centerX       = math.floor(w / 2) + curveOffset
        local leftEdge      = centerX - math.floor(roadWidth / 2)
        local rightEdge     = centerX + math.floor(roadWidth / 2)

        bg:line({ x = leftEdge, y = y }, { x = rightEdge, y = y }, ROAD_COLOR)

        -- stripes
        local stripeWidth = math.max(0.5, roadWidth / 50)

        local distance    = (((math.log(1 + t * 10) / math.log(11)) - trackOffset) * 100) % 100

        if distance % 30 < 20 then
            bg:line({ x = math.ceil(centerX - stripeWidth), y = y }, { x = math.floor(centerX + stripeWidth), y = y }, CENTERLINE_COLOR)
        end
        if distance % 20 < 10 then
            bg:line({ x = leftEdge - stripeWidth, y = y }, { x = leftEdge + stripeWidth, y = y }, CURB_COLOR1)
            bg:line({ x = rightEdge - stripeWidth, y = y }, { x = rightEdge + stripeWidth, y = y }, CURB_COLOR1)
        else
            bg:line({ x = leftEdge - stripeWidth, y = y }, { x = leftEdge + stripeWidth, y = y }, CURB_COLOR2)
            bg:line({ x = rightEdge - stripeWidth, y = y }, { x = rightEdge + stripeWidth, y = y }, CURB_COLOR2)
        end
    end
end

---@param engine engine
function gfx.create_textures(game, engine)
    local spr       = engine.spr

    local pen       = { x = 1, y = 1 }
    local rowHeight = 0
    local padding   = 2

    local function make_texture(id, size, tex, rot)
        local width  = size
        local height = size

        if rot == Rot.R90 or rot == Rot.R270 then
            width, height = height, width
        end
        if pen.x + width > spr.size.width then
            pen.x     = 1
            pen.y     = pen.y + rowHeight + padding
            rowHeight = 0
        end

        spr:blit(
            { x = pen.x, y = pen.y, width = size, height = size },
            tex,
            {
                transparent = 0,
                rotation = rot
            }
        )

        engine:create_texture(id, { x = pen.x, y = pen.y, width = width, height = height })

        pen.x = pen.x + width + padding
        rowHeight = math.max(rowHeight, height)
    end
end

------

---@param dmd dmd
function gfx.draw_dmd(dmd, game)
    local sockets = game.sockets ---@type {[string]: socket})
end

---@param dmd dmd
function gfx.draw_game_over(dmd, game)
    dmd:clear()
    dmd:print({ x = 12, y = 12 }, "DNF", Palette.Red)
end

return gfx
