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
[[0bd1d0g1c0o1a0b1d0e1g0b1c0g1c0a1a8b1a0e1a8a1a8a1a8a1a0a1e0f1a8a1a0a1d0e1g0a1a8a1a8a1a0f1c0a1a8b1a0e1a8a1c8a1a0a1e0d1a0a1c0a1d0a1a0c1g0a1a8a1c0b1c0a1a8a1a0a1d0a1a0c1c8a1a8a1a0a1e0a1a8a1b0a1c0a1a8b1d0b1g0a1a8a1a8a1a0a1d0a1c0a1g0b1a8a1e0a1e0a1b8a1a0a1a8a1a0a1a8b1b8a1a0b1e8a1a0a1a8a1a8a1f0a1c0a1g0b1g0a1g8a1b0a1c0a1g0b1a8a1r8a1a0a1a8b1b8a1a0b1v0a1g0b1ad0a]]
local city_texture_light_damage =
[[0cy1a0c1b0x1a8a1a0c1c0f1a0p1c0c1a8a1a0f1c0l1c8a1a0a1e0d1a0a1c0l1e0a1a8a1c0b1c0a1a8a1a0a1a0j1a8a1a8a1a0a1e0a1a8a1b0a1c0a1a0i1a0a1d0a1a8a1a8a1a0a1d0a1c0a1b0g1a8a1a0a1c0a1e0a1b8a1a0a1a8a1a0a1a8b1b0d1e8a1a0a1a8a1a8a1f0a1c0a1d0a1a0c1g0a1g8a1b0a1c0a1g0b1a8a1r8a1a0a1a8b1b8a1a0b1v0a1g0b1ad0a]]
local city_texture_heavy_damage =
[[0gm1a0e1a0y1c0c1a0n1a0j1a8a1a0c1c0c1a0a1a0f1a0i1a0a1d0a1a8a1a0c1c0b1c0a1b0g1a8a1a0a1c0a1e0a1b8a1a0a1a8a1a0a1a8a0a1a0e1e8a1a0a1a8a1a8a1f0a1c0a1d0e1g0a1g8a1b0a1c0a1d0e1a8a1r8a1a0a1a8b1b0d1v0a1g0b1ad0a]]
local city_texture_destroyed    =
[[0ny1c0d1a0h1a0c1e0e1f0b1c0a1c0a1d0a1g0b1i0a1l0a1g0b1ad0a]]
local cannon_texture            =
[[0arCc0jCe3a0hCfDb0bCa0bCfDc0dCd9aCbDb0gCb9aCcDaEa0iCb0bCbDaEa0iCb0aCbDaEa0lCbDaEa0kCdDaEa0g1a2a1h2a1a0c1a2a1a2a1f2a1a2a1a0a1af]]
local cannon_center_texture     =
[[0t3a0c3a0c3a0gCa0cCa0cCa0fCbDa0aCbDa0aCbDa0eCbDa0aCbDa0aCbDa0eCbDa0aCbDa0aCbDa0eCbDa0aCbDa0aCbDa0eCbDa0aCbDa0aCbDa0eCbDa0aCbDa0aCbDa0eCbDa0aCbDa0aCbDa0eCbDa0aCbDa0aCbDa0dCl0d1a2a1h2a1a0c1a2a1a2a1f2a1a2a1a0a1af]]
local missile_texture           =
[[0b3d0e3b0c3a0a3d0a3d0b3f0b3d0a3d0a3a0c3b0e3d0b]]

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

    local function make_texture(id, size, tex, flipH)
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

        engine:create_texture(id, { x = pen.x, y = pen.y, width = width, height = height })

        pen.x = pen.x + width + padding
        rowHeight = math.max(rowHeight, height)
    end

    make_texture(gfx.textures.city.undamaged, gfx.sizes.city, city_texture_undamaged)
    make_texture(gfx.textures.city.light_damage, gfx.sizes.city, city_texture_light_damage)
    make_texture(gfx.textures.city.heavy_damage, gfx.sizes.city, city_texture_heavy_damage)
    make_texture(gfx.textures.city.destroyed, gfx.sizes.city, city_texture_destroyed)
    make_texture(gfx.textures.cannon.left, gfx.sizes.cannon, cannon_texture)
    make_texture(gfx.textures.cannon.right, gfx.sizes.cannon, cannon_texture, true)
    make_texture(gfx.textures.cannon.center, gfx.sizes.cannon, cannon_center_texture)
    make_texture(gfx.textures.missile, gfx.sizes.missile, missile_texture)
end

------

local cannon_pattern = {
    left = [[0f2a0e2a0e2a0e2a0e2a0e2a0e2g]],
    right = [[2a0g2a0g2a0g2a0g2a0g2a0a2g]],
    center = [[0c2a0f2a0f2a0f2a0f2a0f2a0c2g]],
}

local charge_pattern = [[3b4i3d4i3q4i3d4i3d4i3q4i3d4i3d4i3q4i3d4i3b]]
local cooling_pattern = [[00000D0D000000D0D00D00D0D000D00DDD00D00000D00D00D000D000D0D0D000D0D000DDD000D0DDDDDD0DDDDDD0D000DDD000D0D000D0D0D000D000D00D00D00000D00DDD00D000D0D00D00D0D000000D0D00000]]

---@param dmd dmd
function gfx.draw_dmd(dmd, game, dmdInfo)
    local sockets = game.sockets

    dmd:clear()
    local offset = 18

    local function draw_cannon(type, x)
        local y = 4

        dmd:blit({ x = x + 3, y = y, width = 7, height = 7 }, cannon_pattern[type])
        y = y + 12

        dmd:blit({ x = 2, y = y, width = 13, height = 13 }, charge_pattern)
        sockets[type].chargeRate.position = { x = x, y = y }
        dmd:socket(sockets[type].chargeRate)

        y = y + offset

        dmd:blit({ x = 2, y = y, width = 13, height = 13 }, cooling_pattern)
        sockets[type].coolRate.position = { x = x, y = y }
        dmd:socket(sockets[type].coolRate)

        local barHeight = 10
        local barWidth  = 5
        y               = y + offset + barHeight

        local nrgy      = math.floor(dmdInfo[type].chargeRel * barHeight)
        dmd:rect({ x = x, y = y - barHeight, width = barWidth, height = barHeight - nrgy }, Palette.DarkBrown, true)
        dmd:rect({ x = x, y = y - nrgy, width = barWidth, height = nrgy }, Palette.Red, true)

        local heat = math.floor(dmdInfo[type].heatRel * barHeight)
        dmd:rect({ x = x + barWidth + 2, y = y - barHeight, width = barWidth, height = barHeight - heat }, Palette.DarkBlue, true)
        dmd:rect({ x = x + barWidth + 2, y = y - heat, width = barWidth, height = heat }, Palette.Blue, true)
    end

    draw_cannon("left", 22)
    draw_cannon("center", 43)
    draw_cannon("right", 64)

    sockets.energyRestore.position = { x = 22, y = 73 }
    dmd:socket(sockets.energyRestore)

    local reserveBarWidth  = 20
    local reserveBarHeight = 10
    local reserveBarLeft   = 40
    local reserveBarTop    = 75
    dmd:rect({ x = reserveBarLeft, y = reserveBarTop, width = reserveBarWidth, height = reserveBarHeight }, Palette.Brown, true)

    local energyWidth  = math.ceil(dmdInfo.energyReserveRel * reserveBarWidth) - 2
    local energyHeight = reserveBarHeight - 2
    local energyLeft   = reserveBarLeft + 1
    local energyTop    = reserveBarTop + 1
    dmd:rect({ x = energyLeft, y = energyTop, width = energyWidth, height = energyHeight }, Palette.Yellow, true)
end

---@param dmd dmd
function gfx.draw_game_over(dmd, game)
    dmd:clear()
    dmd:print({ x = 36, y = 12 }, "GAME", Palette.Red)
    dmd:print({ x = 36, y = 20 }, "OVER", Palette.Red)
end

return gfx
