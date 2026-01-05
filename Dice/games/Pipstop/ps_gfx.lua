-- Copyright (c) 2026 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local gfx            = {}

local HORIZON_HEIGHT = 100

local COLORS         = {
    normal = {
        sky          = Palette.Blue,
        horizonLine1 = Palette.LightBlue,
        horizonLine2 = Palette.White,
        sun          = Palette.Yellow,
        cloud        = Palette.White,
        city         = Palette.Gray,
        ground       = Palette.Green,
        road         = Palette.Gray,
        centerLine   = Palette.White,
        curb1        = Palette.Red,
        curb2        = Palette.White
    },
    winter = {
        sky          = Palette.LightBlue,
        horizonLine1 = Palette.White,
        horizonLine2 = Palette.White,
        sun          = Palette.Yellow,
        cloud        = Palette.White,
        city         = Palette.DarkBlue,
        ground       = Palette.White,
        road         = Palette.Gray,
        centerLine   = Palette.White,
        curb1        = Palette.Red,
        curb2        = Palette.White
    },
    dusk = {
        sky          = Palette.BlueGray,
        horizonLine1 = Palette.DarkBlue,
        horizonLine2 = Palette.Blue,
        sun          = Palette.Red,
        cloud        = Palette.Gray,
        city         = Palette.Black,
        ground       = Palette.DarkGreen,
        road         = Palette.BlueGray,
        centerLine   = Palette.Gray,
        curb1        = Palette.Brown,
        curb2        = Palette.Gray
    },
    fall = {
        sky          = Palette.DarkBlue,
        horizonLine1 = Palette.Blue,
        horizonLine2 = Palette.Yellow,
        sun          = Palette.Orange,
        cloud        = Palette.White,
        city         = Palette.BlueGray,
        ground       = Palette.DarkBrown,
        road         = Palette.Gray,
        centerLine   = Palette.White,
        curb1        = Palette.Red,
        curb2        = Palette.White
    }
}

local worldRotation  = 0

local city           = nil
local clouds         = nil

---@param engine engine
function gfx.create_background(engine, curveAmount, trackOffset, theme)
    local bg          = engine.bg
    local size        = bg.size
    local w, h        = size.width, size.height

    local themes      = { "normal", "dusk", "fall", "winter" }
    local colors      = COLORS[themes[theme]]

    local WORLD_WIDTH = w * 4
    local SUN_SPEED   = 0.5
    local CITY_SPEED  = 0.8
    local CLOUD_SPEED = 1.5

    worldRotation     = (worldRotation + curveAmount) % WORLD_WIDTH

    local function get_draw_x(objectX, speedMult)
        local span = WORLD_WIDTH * speedMult
        local offset = worldRotation * speedMult
        return ((objectX - offset + span / 2) % span) - span / 2
    end

    -- sky
    bg:rect({ x = 0, y = 0, width = w, height = HORIZON_HEIGHT }, colors.sky, true)
    bg:rect({ x = 0, y = HORIZON_HEIGHT - 3, width = w, height = 2 }, colors.horizonLine1, true)
    bg:rect({ x = 0, y = HORIZON_HEIGHT - 1, width = w, height = 1 }, colors.horizonLine2, true)

    -- sun
    local sunX    = get_draw_x(30, SUN_SPEED)
    local sunSpan = WORLD_WIDTH * SUN_SPEED
    local sunWrap = (sunX > 0) and (sunX - sunSpan) or (sunX + sunSpan)

    bg:circle({ x = sunX, y = 20 }, 10, colors.sun, true)
    bg:circle({ x = sunWrap, y = 20 }, 10, colors.sun, true)

    -- city
    if not city then
        city = {}
        for i = 1, 15 do
            local bw = engine:rnd(3, 10)
            local bh = engine:rnd(12, 20)
            table.insert(city, {
                x = engine:rnd(0, 1) * 120,
                w = bw,
                h = bh
            })
        end
    end

    local citySpan = WORLD_WIDTH * CITY_SPEED
    for _, b in ipairs(city) do
        local drawX = get_draw_x(b.x, CITY_SPEED)
        local wrapX = (drawX > 0) and (drawX - citySpan) or (drawX + citySpan)
        local yPos  = HORIZON_HEIGHT - b.h + 1
        bg:rect({ x = drawX, y = yPos, width = b.w, height = b.h }, colors.city, true)
        bg:rect({ x = wrapX, y = yPos, width = b.w, height = b.h }, colors.city, true)
    end

    -- clouds
    if not clouds then
        clouds = {}
        for i = 1, 25 do
            local cx = engine:rnd(0, 1) * (WORLD_WIDTH * CLOUD_SPEED)
            local cy = math.floor(engine:rnd(0, 1) * HORIZON_HEIGHT / 4 * 3)
            local cw = engine:irnd(12, 22)
            local ch = engine:irnd(3, 6)
            table.insert(clouds, { x = cx, y = cy, width = cw, height = ch })
        end
    end

    local cSpan = WORLD_WIDTH * CLOUD_SPEED
    for idx, cloud in ipairs(clouds) do
        local drawX = get_draw_x(cloud.x, CLOUD_SPEED)
        local wrapX = (drawX > 0) and (drawX - cSpan) or (drawX + cSpan)

        bg:rect({ x = drawX, y = cloud.y, width = cloud.width, height = cloud.height }, colors.cloud, true)
        bg:rect({ x = wrapX, y = cloud.y, width = cloud.width, height = cloud.height }, colors.cloud, true)
    end

    -- ground
    bg:rect({ x = 0, y = HORIZON_HEIGHT, width = w, height = h - HORIZON_HEIGHT }, colors.ground, true)

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

        bg:line({ x = leftEdge, y = y }, { x = rightEdge, y = y }, colors.road)

        -- stripes
        local stripeWidth = math.max(0.5, roadWidth / 50)

        local distance    = (((math.log(1 + t * 10) / math.log(11)) - trackOffset) * 100) % 100

        if distance % 30 < 20 then
            bg:line({ x = math.ceil(centerX - stripeWidth), y = y }, { x = math.floor(centerX + stripeWidth), y = y }, colors.centerLine)
        end
        if distance % 20 < 10 then
            bg:line({ x = leftEdge - stripeWidth, y = y }, { x = leftEdge + stripeWidth, y = y }, colors.curb1)
            bg:line({ x = rightEdge - stripeWidth, y = y }, { x = rightEdge + stripeWidth, y = y }, colors.curb1)
        else
            bg:line({ x = leftEdge - stripeWidth, y = y }, { x = leftEdge + stripeWidth, y = y }, colors.curb2)
            bg:line({ x = rightEdge - stripeWidth, y = y }, { x = rightEdge + stripeWidth, y = y }, colors.curb2)
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
