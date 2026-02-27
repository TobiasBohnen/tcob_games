-- Copyright (c) 2026 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local gfx           = {}

gfx.horizonHeight   = 100

local SUN_SPEED     = 0.5
local CITY_SPEED    = 0.8
local CLOUD_SPEED   = 1.5

local THEMES        = {
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

local worldRotation = 0

local city          = {}
local clouds        = {}

local function create_objects(engine, worldWidth)
    if #clouds == 0 then
        for i = 1, 25 do
            clouds[#clouds + 1] = {
                x = engine:rnd(0, 1) * (worldWidth * CLOUD_SPEED),
                y = math.floor(engine:rnd(0, 1) * gfx.horizonHeight / 4 * 3),
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

local roadData = {}

function gfx.get_road_at_y(y)
    return roadData[math.floor(y)] or roadData[gfx.horizonHeight]
end

---@param engine engine
function gfx.create_background(engine, curveAmount, speedFactor, trackOffset, theme)
    local bg           = engine.bg
    local size         = bg.size
    local w, h         = size.width, size.height
    local worldWidth   = w * 4
    local currentTheme = THEMES[theme]

    worldRotation      = (worldRotation + (curveAmount * speedFactor)) % worldWidth

    create_objects(engine, worldWidth)

    local function get_draw_x(objectX, speedMult)
        local span   = worldWidth * speedMult
        local offset = worldRotation * speedMult
        return ((objectX - offset + span / 2) % span) - span / 2
    end

    -- sky
    bg:rect({ x = 0, y = 0, width = w, height = gfx.horizonHeight }, currentTheme.sky, true)
    bg:rect({ x = 0, y = gfx.horizonHeight - 3, width = w, height = 2 }, currentTheme.horizonLine1, true)
    bg:rect({ x = 0, y = gfx.horizonHeight - 1, width = w, height = 1 }, currentTheme.horizonLine2, true)

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
            local yPos  = gfx.horizonHeight - b.h + 1

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
    bg:rect({ x = 0, y = gfx.horizonHeight, width = w, height = h - gfx.horizonHeight }, currentTheme.ground, true)

    -- road
    for y = gfx.horizonHeight, h - 1 do
        local t             = (y - gfx.horizonHeight) / (h - gfx.horizonHeight)
        local roadWidth     = math.floor(20 + t * (w / 3 * 2))

        local oneMinusT     = 1.0 - t
        local curveStrength = oneMinusT * oneMinusT * oneMinusT
        local curveOffset   = math.floor(curveAmount * curveStrength * (w / 2))

        local centerX       = math.floor(w / 2) + curveOffset
        local leftEdge      = centerX - math.floor(roadWidth / 2)
        local rightEdge     = centerX + math.floor(roadWidth / 2)

        roadData[y]         = {
            leftEdge  = leftEdge,
            rightEdge = rightEdge,
            centerX   = centerX,
            roadWidth = roadWidth
        }

        bg:line({ x = leftEdge, y = y }, { x = rightEdge, y = y }, currentTheme.road)

        -- stripes
        local stripeWidth = math.max(0.5, roadWidth / 50)
        local distance    = (math.sqrt(t) - trackOffset) * 100 % 100
        if distance % 20 < 15 then
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

local car_texture     =
[[!2i0p2q0b9n0b2o0b9p0b2m0b9r0b2k0c9r0c2i0z2g0ab2e0ad2cC0c3d0p3d0cC2Cb0c3d0p3d0cCd0adCd0k1h0kCd0k1h0kCd0fDb0vCd0fDb0vCd0adCh0vCf2Ce2vCe2]]
local opp_car_texture =
[[!2i7p2q7b5n7b2o7b5p7b2m7b5r7b2k07b5r7b02i07x02h7z2g7ab2dCb7c3b7r3b7cCb2Cb7c3d7p3d7cCd7d3b7r3b7dCd7k1h7kCe7j1h7jCf7abCh7b0c7sCj7c07tCk2b7r2bCf2Ce2vCe2]]

gfx.textures          = {
    car = 1,
    opp_car = {
        { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 },
        { 20, 21, 22, 23, 24, 25, 26, 27, 28, 29 },
        { 30, 31, 32, 33, 34, 35, 36, 37, 38, 39 },
        { 40, 41, 42, 43, 44, 45, 46, 47, 48, 49 },
        { 50, 51, 52, 53, 54, 55, 56, 57, 58, 59 },
    }
}

gfx.sizes             = {
    car = { width = 34, height = 18 }
}

---@param engine engine
function gfx.create_textures(game, engine)
    local spr       = engine.spr

    local pen       = { x = 1, y = 1 }
    local rowHeight = 0
    local padding   = 2

    ---@param size size
    local function create(id, size, tex, tp, scale, swap)
        local width  = size.width * scale
        local height = size.height * scale

        if pen.x + width >= spr.size.width then
            pen.x     = 1
            pen.y     = pen.y + rowHeight + padding
            rowHeight = 0
        end

        spr:blit(
            { x = pen.x, y = pen.y, width = size.width, height = size.height },
            tex,
            {
                transparent = tp,
                scale       = scale,
                swap        = swap
            }
        )

        engine:define_texture(id, { x = pen.x, y = pen.y, width = width, height = height })

        pen.x = pen.x + width + padding
        rowHeight = math.max(rowHeight, height)
    end

    create(gfx.textures.car, gfx.sizes.car, car_texture, Palette.White, 1)

    for i = 1, 10 do
        local scale = math.sqrt(i / 10)
        create(gfx.textures.opp_car[1][i], gfx.sizes.car, opp_car_texture, Palette.White, scale)
        create(gfx.textures.opp_car[2][i], gfx.sizes.car, opp_car_texture, Palette.White, scale, { [Palette.Orange] = Palette.Pink })
        create(gfx.textures.opp_car[3][i], gfx.sizes.car, opp_car_texture, Palette.White, scale, { [Palette.Orange] = Palette.Yellow })
        create(gfx.textures.opp_car[4][i], gfx.sizes.car, opp_car_texture, Palette.White, scale, { [Palette.Orange] = Palette.Green })
        create(gfx.textures.opp_car[5][i], gfx.sizes.car, opp_car_texture, Palette.White, scale, { [Palette.Orange] = Palette.Blue })
    end
end

------

local wheel_patterns = {
    [1] = [[0000000000000000000000E00000000EE0000000EEEEEE00000EE000E00000E0000E0000000000E0000000000E000000000E]],
    [2] = [[0000000000000000000000000E00000000EE0000000EEEE0000000EE0E0000000E00E0000000000E000000000E000000000E]],
    [3] = [[000000000000000000000000000E00000000EE0000000EEEEE000000EE0E0000000E0E000000000E000000000E000000000E]],
    [4] = [[0000000000000000000000E000000000EE000000EEEEE00000E0EE000000E0E0000000E000000000E000000000E000000000]],
    [5] = [[000000000000000000000000E000000000EE0000000EEEE00000E0EE00000E00E00000E000000000E000000000E000000000]],
    [6] = [[000000000000000000000000000E000000000EE00000EEEEEE000E000EE000E0000E000E00000000E000000000E000000000]],
}

---@param hud tex
function gfx.draw_hud(hud, game)
    hud:clear()

    local y          = 3
    local xStart     = (hud.size.width - (12 * 6)) / 2
    local x          = xStart

    -- speed
    local carTarget  = math.floor(game.car.speed.target / 100 * 12)
    local carTargetX = 0
    for i = 1, 12 do
        local carSlower = math.ceil(game.car.speed.current) < i * (100 / 12)
        local col
        if carSlower then
            col = Palette.DarkBrown
        else
            if i <= 4 then
                col = Palette.Green
            elseif i <= 8 then
                col = Palette.Yellow
            else
                col = Palette.Red
            end
        end

        local off = (12 - i) // 2
        hud:rect({ x = x, y = y + off, width = 5, height = 7 - off }, col, true)
        if i == carTarget then
            carTargetX = x
        end
        x = x + 6
    end
    if game.car.speed.target > 0 then
        hud:rect({ x = carTargetX + 2, y = y, width = 1, height = 7 }, Palette.Blue, true)
    end


    y = y + 8


    -- fuel
    x = xStart
    for i = 1, 12 do
        local col
        if i <= game.car.fuel / 100 * 12 then
            col = Palette.Brown
        else
            col = Palette.DarkBrown
        end

        hud:rect({ x = x, y = y, width = 5, height = 4 }, col, true)
        x = x + 6
    end
    y                = y + 6

    -- wheel
    x                = (hud.size.width - 16) / 2
    local wheel      = game.sockets.wheel
    local wheelValue = wheel.die_value
    wheel.position   = { x = x, y = y }
    hud:socket(wheel)
    if wheelValue > 0 then
        hud:blit({ x = wheelValue < 4 and x - 20 or x + 20, y = y, width = 10, height = 10 }, wheel_patterns[wheelValue])
    end

    y = y + 18

    gfx.draw_events(hud, game, y)
end

---@param hud tex
function gfx.draw_events(hud, game, y)
    local xStart = 1
    local offset = 18

    for _, event in ipairs(game.eventQueue) do
        hud:rect({ x = 0, y = y, width = hud.size.width, height = offset + 15 }, Palette.DarkBlue, true)
        y              = y + 1

        local x        = xStart
        local texColor = event.finished and Palette.Red or Palette.LightBlue

        hud:print({ x = x, y = y }, event.title, texColor, "Font3x5")
        y = y + 7

        hud:print({ x = x + 55, y = y }, event.value .. " / " .. event.target, texColor, "Font3x5")

        hud:print({ x = x, y = y }, "turns = " .. event.turnsLeft, texColor, "Font3x5")
        y = y + 7

        for _, socket in ipairs(event.sockets or {}) do
            socket.position = { x = x + 1, y = y }
            hud:socket(socket)
            x = x + offset
        end

        y = y + offset + 1
    end
end

---@param hud tex
function gfx.draw_game_over(hud)
    hud:clear()
    hud:print({ x = 12, y = 12 }, "DNF", Palette.Red)
end

return gfx
