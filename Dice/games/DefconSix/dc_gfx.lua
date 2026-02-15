-- Copyright (c) 2026 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local gfx = {}

---@param engine engine
function gfx.create_background(engine)
    local bg    = engine.bg
    local size  = bg.size
    local w, h  = size.width, size.height
    local n     = w * h
    local buf   = {}

    -- generate stars
    local stars = {}
    for i = 1, 35 do
        local x = math.floor(engine:rnd(0, 1) * w)
        local y = math.floor(engine:rnd(0, 1) * h * 2 / 3)
        local s = math.floor(engine:rnd(0, 1) * 2 + 1)
        table.insert(stars, { x = x, y = y, s = s })
    end

    for y = 0, h - 1 do
        for x = 0, w - 1 do
            local idx = y * w + x + 1
            -- ground
            if y >= h * 2 / 3 then
                buf[idx] = "A"
            else
                buf[idx] = "C"
            end
        end
    end

    -- draw stars
    for _, star in ipairs(stars) do
        for dy = 0, star.s - 1 do
            for dx = 0, star.s - 1 do
                local xx, yy = star.x + dx, star.y + dy
                if xx >= 0 and xx < w and yy >= 0 and yy < h * 2 / 3 then
                    buf[yy * w + xx + 1] = "2"
                end
            end
        end
    end

    bg:blit(bg.bounds, table.concat(buf))
end

gfx.textures                    = {
    city = { undamaged = 0, light_damage = 1, heavy_damage = 2, destroyed = 3 }, ---@type { [string]: texture }
    cannon = { left = 10, right = 11, center = 12 }, ---@type { [string]: texture }
    missile = 20, ---@type texture
}

local city_texture_undamaged    =
[[!0bd1d0g1c0o10b1d0e1g0b1c0g1c018b10e181818101e0f18101d0e1g0181810f1c018b10e181c8101e0d101c01d010c1g0181c0b1c018101d010c1c818101e0181b01c018b1d0b1g01818101d01c01g0b181e01e01b810181018b1b810b1e81018181f01c01g0b1g01g81b01c01g0b181r81018b1b810b1v01g0b1ad0]]
local city_texture_light_damage =
[[!0cy10c1b0x1810c1c0f10p1c0c1810f1c0l1c8101e0d101c0l1e0181c0b1c0181010j1818101e0181b01c010i101d01818101d01c01b0g18101c01e01b810181018b1b0d1e81018181f01c01d010c1g01g81b01c01g0b181r81018b1b810b1v01g0b1ad0]]
local city_texture_heavy_damage =
[[!0gm10e10y1c0c10n10j1810c1c0c1010f10i101d01810c1c0b1c01b0g18101c01e01b810181018010e1e81018181f01c01d0e1g01g81b01c01d0e181r81018b1b0d1v01g0b1ad0]]
local city_texture_destroyed    =
[[!0ny1c0d10h10c1e0e1f0b1c01c01d01g0b1i01l01g0b1ad0]]
local cannon_texture            =
[[!0arCc0jCe30hCfDb0bC0bCfDc0dCd9CbDb0gCb9CcDE0iCb0bCbDE0iCb0CbDE0lCbDE0kCdDE0g121h210c12121f212101af]]
local cannon_center_texture     =
[[!0t30c30c30gC0cC0cC0fCbD0CbD0CbD0eCbD0CbD0CbD0eCbD0CbD0CbD0eCbD0CbD0CbD0eCbD0CbD0CbD0eCbD0CbD0CbD0eCbD0CbD0CbD0eCbD0CbD0CbD0dCl0d121h210c12121f212101af]]
local missile_texture           =
[[!0b3d0e3b0c303d03d0b3f0b3d03d030c3b0e3d0b]]

gfx.sizes                       = {
    city = { width = 32, height = 16 },
    cannon = { width = 16, height = 16 },
    missile = { width = 8, height = 8 }
}

---@param engine engine
function gfx.create_textures(game, engine)
    local spr       = engine.spr
    local pen       = { x = 1, y = 1 }
    local rowHeight = 0
    local padding   = 2

    local function create(id, size, tex, flipH)
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
                transparent = 0,
                flip_h = flipH
            }
        )

        engine:define_texture(id, { x = pen.x, y = pen.y, width = width, height = height })

        pen.x = pen.x + width + padding
        rowHeight = math.max(rowHeight, height)
    end

    create(gfx.textures.city.undamaged, gfx.sizes.city, city_texture_undamaged)
    create(gfx.textures.city.light_damage, gfx.sizes.city, city_texture_light_damage)
    create(gfx.textures.city.heavy_damage, gfx.sizes.city, city_texture_heavy_damage)
    create(gfx.textures.city.destroyed, gfx.sizes.city, city_texture_destroyed)
    create(gfx.textures.cannon.left, gfx.sizes.cannon, cannon_texture)
    create(gfx.textures.cannon.right, gfx.sizes.cannon, cannon_texture, true)
    create(gfx.textures.cannon.center, gfx.sizes.cannon, cannon_center_texture)
    create(gfx.textures.missile, gfx.sizes.missile, missile_texture)
end

------

local cannon_pattern  = {
    left   = [[!0f20e20e20e20e20e20e2g]],
    right  = [[!20g20g20g20g20g202g]],
    center = [[!0c20f20f20f20f20f20c2g]],
}

local charge_pattern  = [[!3b4i3d4i3q4i3d4i3d4i3q4i3d4i3d4i3q4i3d4i3b]]
local cooling_pattern = [[!0eD0D0fD0D0bD0bD0D0cD0bDc0bD0eD0bD0bD0cD0cD0D0D0cD0D0cDc0cD0Df0Df0D0cDc0cD0D0cD0D0D0cD0cD0bD0bD0eD0bDc0bD0cD0D0bD0bD0D0fD0D0e]]

---@param hud tex
function gfx.draw_hud(hud, game, hudInfo)
    local sockets = game.sockets

    hud:clear()
    local offset = 18

    local function draw_cannon(type, x)
        local y = 4

        hud:blit({ x = x + 3, y = y, width = 7, height = 7 }, cannon_pattern[type])
        y = y + 12

        hud:blit({ x = 2, y = y, width = 13, height = 13 }, charge_pattern)
        sockets[type].chargeRate.position = { x = x - 1, y = y - 2 }
        hud:socket(sockets[type].chargeRate)

        y = y + offset

        hud:blit({ x = 2, y = y, width = 13, height = 13 }, cooling_pattern)
        sockets[type].coolRate.position = { x = x - 1, y = y - 2 }
        hud:socket(sockets[type].coolRate)

        local barHeight = 10
        local barWidth  = 5
        y               = y + offset + barHeight

        local nrgy      = math.floor(hudInfo[type].chargeRel * barHeight)
        hud:rect({ x = x, y = y - barHeight, width = barWidth, height = barHeight - nrgy }, Palette.DarkBrown, true)
        hud:rect({ x = x, y = y - nrgy, width = barWidth, height = nrgy }, Palette.Red, true)

        local heat = math.floor(hudInfo[type].heatRel * barHeight)
        hud:rect({ x = x + barWidth + 2, y = y - barHeight, width = barWidth, height = barHeight - heat }, Palette.DarkBlue, true)
        hud:rect({ x = x + barWidth + 2, y = y - heat, width = barWidth, height = heat }, Palette.Blue, true)
    end

    draw_cannon("left", 22)
    draw_cannon("center", 43)
    draw_cannon("right", 64)

    sockets.energyRestore.position = { x = 21, y = 72 }
    hud:socket(sockets.energyRestore)

    local reserveBarWidth  = 20
    local reserveBarHeight = 10
    local reserveBarLeft   = 40
    local reserveBarTop    = 75
    hud:rect({ x = reserveBarLeft, y = reserveBarTop, width = reserveBarWidth, height = reserveBarHeight }, Palette.Brown, true)

    local energyWidth  = math.ceil(hudInfo.energyReserveRel * reserveBarWidth) - 2
    local energyHeight = reserveBarHeight - 2
    local energyLeft   = reserveBarLeft + 1
    local energyTop    = reserveBarTop + 1
    hud:rect({ x = energyLeft, y = energyTop, width = energyWidth, height = energyHeight }, Palette.Yellow, true)
end

---@param hud tex
function gfx.draw_game_over(hud)
    hud:clear()
    hud:print({ x = 36, y = 12 }, "GAME", Palette.Red)
    hud:print({ x = 36, y = 20 }, "OVER", Palette.Red)
end

return gfx
