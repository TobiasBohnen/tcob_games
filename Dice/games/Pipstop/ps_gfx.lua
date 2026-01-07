-- Copyright (c) 2026 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local gfx            = {}

local HORIZON_HEIGHT = 100
local SUN_SPEED      = 0.5
local CITY_SPEED     = 0.8
local CLOUD_SPEED    = 1.5

local THEMES         = {
    day = {
        sky          = Palette.Blue,
        horizonLine1 = Palette.LightBlue,
        horizonLine2 = Palette.White,
        sun          = Palette.Yellow,
        cloud        = Palette.White,
        city         = Palette.Gray,
        window       = Palette.DarkBlue,
        ground       = Palette.Green,
        road         = Palette.Gray,
        centerLine   = Palette.White,
        curb1        = Palette.Red,
        curb2        = Palette.White,

        sunScale     = 1
    },
    dusk = {
        sky          = Palette.BlueGray,
        horizonLine1 = Palette.DarkBlue,
        horizonLine2 = Palette.Blue,
        sun          = Palette.Red,
        cloud        = Palette.Gray,
        city         = Palette.Black,
        window       = Palette.LightBlue,
        ground       = Palette.DarkGreen,
        road         = Palette.DarkBlue,
        centerLine   = Palette.Gray,
        curb1        = Palette.Brown,
        curb2        = Palette.Gray,

        sunScale     = 1
    },
    night = {
        sky          = Palette.Black,
        horizonLine1 = Palette.DarkBlue,
        horizonLine2 = Palette.BlueGray,
        sun          = Palette.White,
        cloud        = Palette.DarkBlue,
        city         = Palette.Black,
        window       = Palette.Yellow,
        ground       = Palette.DarkGreen,
        road         = Palette.Black,
        centerLine   = Palette.Yellow,
        curb1        = Palette.Brown,
        curb2        = Palette.Gray
    },
    fall = {
        sky          = Palette.Gray,
        horizonLine1 = Palette.White,
        horizonLine2 = Palette.Yellow,
        sun          = Palette.Orange,
        cloud        = Palette.White,
        city         = Palette.BlueGray,
        window       = Palette.Black,
        ground       = Palette.DarkBrown,
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
        window       = Palette.Orange,
        ground       = Palette.White,
        road         = Palette.Gray,
        centerLine   = Palette.White,
        curb1        = Palette.Red,
        curb2        = Palette.White
    },
    desert = {
        sky          = Palette.Blue,
        horizonLine1 = Palette.Yellow,
        horizonLine2 = Palette.Orange,
        sun          = Palette.Yellow,
        ground       = Palette.Yellow,
        road         = Palette.Gray,
        centerLine   = Palette.White,
        curb1        = Palette.Red,
        curb2        = Palette.Yellow
    },
}

gfx.textures         = {
    car = {
        straight = 1,
        left     = 2,
        right    = 3
    }
}

local worldRotation  = 0

local city           = {}
local clouds         = {}

local function create_objects(engine, worldWidth)
    if #clouds == 0 then
        for i = 1, 25 do
            clouds[#clouds + 1] = {
                x = engine:rnd(0, 1) * (worldWidth * CLOUD_SPEED),
                y = math.floor(engine:rnd(0, 1) * HORIZON_HEIGHT / 4 * 3),
                w = engine:irnd(12, 22),
                h = engine:irnd(3, 6)
            }
        end
    end

    if #city == 0 then
        for i = 1, 15 do
            city[#city + 1] = {
                x = engine:rnd(0, 1) * 120,
                w = engine:rnd(3, 10),
                h = engine:rnd(12, 20)
            }
        end
    end
end

---@param engine engine
function gfx.create_background(engine, curveAmount, trackOffset, biome)
    local bg           = engine.bg
    local size         = bg.size
    local w, h         = size.width, size.height
    local worldWidth   = w * 4
    local currentTheme = THEMES[biome]

    worldRotation      = (worldRotation + curveAmount) % worldWidth

    create_objects(engine, worldWidth)

    local function get_draw_x(objectX, speedMult)
        local span = worldWidth * speedMult
        local offset = worldRotation * speedMult
        return ((objectX - offset + span / 2) % span) - span / 2
    end

    -- sky
    bg:rect({ x = 0, y = 0, width = w, height = HORIZON_HEIGHT }, currentTheme.sky, true)
    bg:rect({ x = 0, y = HORIZON_HEIGHT - 3, width = w, height = 2 }, currentTheme.horizonLine1, true)
    bg:rect({ x = 0, y = HORIZON_HEIGHT - 1, width = w, height = 1 }, currentTheme.horizonLine2, true)

    -- sun
    local sunX    = get_draw_x(30, SUN_SPEED)
    local sunSpan = worldWidth * SUN_SPEED
    local sunWrap = (sunX > 0) and (sunX - sunSpan) or (sunX + sunSpan)

    bg:circle({ x = sunX, y = 20 }, 10, currentTheme.sun, true)
    bg:circle({ x = sunWrap, y = 20 }, 10, currentTheme.sun, true)

    -- city
    if currentTheme.city then
        local citySpan = worldWidth * CITY_SPEED
        for _, b in ipairs(city) do
            local drawX = get_draw_x(b.x, CITY_SPEED)
            local wrapX = (drawX > 0) and (drawX - citySpan) or (drawX + citySpan)
            local yPos  = HORIZON_HEIGHT - b.h + 1

            -- buildings
            bg:rect({ x = drawX, y = yPos, width = b.w, height = b.h }, currentTheme.city, true)
            bg:rect({ x = wrapX, y = yPos, width = b.w, height = b.h }, currentTheme.city, true)

            -- windows
            for wy = yPos + 2, yPos + b.h - 2, 3 do
                for wx = 1, b.w - 2, 2 do
                    bg:pixel({ x = drawX + wx, y = wy }, currentTheme.window)
                    bg:pixel({ x = wrapX + wx, y = wy }, currentTheme.window)
                end
            end
        end
    end

    -- clouds
    if currentTheme.cloud then
        local cloudSpan = worldWidth * CLOUD_SPEED
        for idx, cloud in ipairs(clouds) do
            local drawX = get_draw_x(cloud.x, CLOUD_SPEED)
            local wrapX = (drawX > 0) and (drawX - cloudSpan) or (drawX + cloudSpan)

            bg:rect({ x = drawX, y = cloud.y, width = cloud.w, height = cloud.h }, currentTheme.cloud, true)
            bg:rect({ x = wrapX, y = cloud.y, width = cloud.w, height = cloud.h }, currentTheme.cloud, true)
        end
    end


    -- ground
    bg:rect({ x = 0, y = HORIZON_HEIGHT, width = w, height = h - HORIZON_HEIGHT }, currentTheme.ground, true)

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

        bg:line({ x = leftEdge, y = y }, { x = rightEdge, y = y }, currentTheme.road)

        -- stripes
        local stripeWidth = math.max(0.5, roadWidth / 50)

        local distance    = (((math.log(1 + t * 10) / math.log(11)) - trackOffset) * 100) % 100

        if distance % 30 < 20 then
            bg:line({ x = math.ceil(centerX - stripeWidth), y = y }, { x = math.floor(centerX + stripeWidth), y = y }, currentTheme.centerLine)
        end
        if distance % 20 < 10 then
            bg:line({ x = leftEdge - stripeWidth, y = y }, { x = leftEdge + stripeWidth, y = y }, currentTheme.curb1)
            bg:line({ x = rightEdge - stripeWidth, y = y }, { x = rightEdge + stripeWidth, y = y }, currentTheme.curb1)
        else
            bg:line({ x = leftEdge - stripeWidth, y = y }, { x = leftEdge + stripeWidth, y = y }, currentTheme.curb2)
            bg:line({ x = rightEdge - stripeWidth, y = y }, { x = rightEdge + stripeWidth, y = y }, currentTheme.curb2)
        end
    end
end

local car_texture =
[[2bd0h2aaCc2b0a3a0aDb0a3a0a2bCc2uCe2a0a3a0aDb0a3a0a2aCe2tCc1aCa2a0a3a0aCb0a3a0a2aCa1aCc2tCc0d3a0aCb0a3a0dCc2tCc1a0a3a0cCb0c3a0a1aCc2t0c3c0aCf0a3c0c2t0a3e0aCf0a3e0a2q0h3a0aCf0a3a0h2n0aDf0cCf0cDf0a2n0aDf0bCh0bDf0a2kCc0bDv0bCc2gCe0cDr0cCe2eCh0aDr0aCh2dCf1aCa0iDb0iCa1aCf2dCf0kDb0kCf2dCf0kDb0kCf2dCf1aCa0a2c0a3a0h3a0a2c0aCa1aCf2dCh0e3a0h3a0eCh2eCg2d0c2f0c2dCg2fCf2vCf2c]]
local car_turn_right =
[[2bg0h2aaCc2b0a3a0aDb0a3a0a2bCd2tCe2a0a3a0aDb0a3a0a2aCf2sCb1bCa2a0a3a0d3a0a2aCc1bCa2rCc0d3a0bCa0a3a0cCc0a1aCa2rCc1a0a3a0cCb0c3a0aCc1aCa2s0c3c0aCf0a3c0c2t0a3e0aCf0a3e0a2p0h3a0aCf0a3a0h2n0aDf0cCf0cDf0a2n0aDf0bCh0bDf0a2jCd0bDv0bCb2gCf0cDr0cCd2eCh2a0aDr0aCg2dCd1bCb2a0iDb0iCd1bCa2dCd0mDb0iCe0a1aCa2dCd0mDb0iCe0a1aCa2dCd1bCb0a2c0a3a0h3a0a2c0aCe1bCa2dCh0e3a0h3a0eCh2eCg2d0c2f0c2dCg2fCf2vCf2c]]

gfx.sizes = {
    car = { width = 40, height = 22 },
}

---@param engine engine
function gfx.create_textures(game, engine)
    local spr       = engine.spr

    local pen       = { x = 1, y = 1 }
    local rowHeight = 0
    local padding   = 2

    ---@param size size
    local function make_texture(id, size, tex, tp, flipH)
        local width  = size.width
        local height = size.height

        if pen.x + width > spr.size.width then
            pen.x     = 1
            pen.y     = pen.y + rowHeight + padding
            rowHeight = 0
        end

        spr:blit(
            { x = pen.x, y = pen.y, width = width, height = height },
            tex,
            {
                transparent = tp,
                flip_h = flipH
            }
        )

        engine:create_texture(id, { x = pen.x, y = pen.y, width = width, height = height })

        pen.x = pen.x + width + padding
        rowHeight = math.max(rowHeight, height)
    end

    make_texture(gfx.textures.car.straight, gfx.sizes.car, car_texture, Palette.White, false)
    make_texture(gfx.textures.car.right, gfx.sizes.car, car_turn_right, Palette.White, false)
    make_texture(gfx.textures.car.left, gfx.sizes.car, car_turn_right, Palette.White, true)
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
