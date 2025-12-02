-- Copyright (c) 2025 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local gfx = {}

---@param engine engine
function gfx.get_background(game, engine)
    local size = ScreenSize
    local w, h = size.width, size.height
    local n    = w * h

    local buf  = {}
    for i = 1, n do
        buf[i] = "0"
    end

    local starCount = 75
    for i = 1, starCount do
        local x = math.floor(engine:random(0, 1) * w)
        local y = math.floor(engine:random(0, 1) * h)
        local s = math.floor(engine:random(0, 1) * 2 + 1)

        for dy = 0, s - 1 do
            for dx = 0, s - 1 do
                local xx = x + dx
                local yy = y + dy
                if xx >= 0 and xx < w and yy >= 0 and yy < h then
                    buf[yy * w + xx + 1] = "2"
                end
            end
        end
    end

    return table.concat(buf)
end

local function square(x)
    return { width = x, height = x }
end

---@param engine engine
function gfx.get_textures(game, engine)
    return {
        [game.ship.texture] = {
            size        = square(32),
            bitmap      =
            [[0o2b0ad2b0ac2a0b2a0ab2a0b2a0aa2a0d2a0z2a0d2a0y2a0f2a0x2a0f2a0w2a0h2a0v2a0h2a0u2a0j2a0t2a0j2a0s2a0l2a0r2a0l2a0q2a0n2a0p2a0n2a0o2a0p2a0n2a0p2a0m2a0r2a0l2a0r2a0k2a0t2a0j2a0t2a0i2a0v2a0h2a0j2b0j2a0g2a0i2b0b2b0i2a0f2a0g2b0f2b0g2a0e2a0f2b0j2b0f2a0d2a0d2b0n2b0d2a0c2a0c2b0r2b0c2a0b2a0a2b0v2b0a2a0a2c0z2d0ad2a]],
            transparent = 0,
        },
        [game.ship.hurtTexture] = {
            size        = square(32),
            bitmap      =
            [[0o2b0ad2b0ac2a3b2a0ab2a3b2a0aa2a3d2a0z2a3d2a0y2a3f2a0x2a3f2a0w2a3h2a0v2a3h2a0u2a3j2a0t2a3j2a0s2a3l2a0r2a3l2a0q2a3n2a0p2a3n2a0o2a3p2a0n2a3p2a0m2a3r2a0l2a3r2a0k2a3t2a0j2a3t2a0i2a3v2a0h2a3j2b3j2a0g2a3i2b0b2b3i2a0f2a3g2b0f2b3g2a0e2a3f2b0j2b3f2a0d2a3d2b0n2b3d2a0c2a3c2b0r2b3c2a0b2a3a2b0v2b3a2a0a2c0z2d0ad2a]],
            transparent = 0,
        },
        [game.bulletTexture] = {
            size   = square(8),
            bitmap = [[8aFf8aFa7fFb7fFb7fFb7fFb7fFb7fFa8aFf8a]]
        },
        [game.asteroidTextures.small] = {
            size        = square(16),
            bitmap      = [[0v6b0m6g0g6k0d6l0c6m0c6n0c6m0c6m0c6l0e6k0e6j0f6j0g6h0h6h0t]],
            transparent = 0,
        },
        [game.asteroidTextures.medium] = {
            size        = square(24),
            bitmap      = [[0be6c0t6i0n6l0j6o0h6q0f6r0e6s0e6t0e6s0e6s0e6s0e6r0g6q0g6p0i6o0i6n0j6n0k6l0l6l0m6j0bc]],
            transparent = 0,
        },
        [game.asteroidTextures.large] = {
            size        = square(32),
            bitmap      = [[0dd6d0aa6k0t6n0q6q0m6s0l6u0j6w0h6x0g6y0g6z0g6y0g6y0g6y0g6y0g6x0i6w0i6v0k6u0k6t0m6s0m6r0n6r0o6p0p6p0q6n0s6l0db]],
            transparent = 0,
        },
        [game.explosionTexture] = {
            size        = square(32),
            bitmap      =
            [[0p3a0ae3b0ac3a7a3a0ac3a7b3a0aa3a7c3a0aa3a7d3a0y3a7e3a0y3a7f3a0v3b7g3b0r3c7k3c0l3c7q3c0f3c7w3c0a3b7ac3a0a3b7z3b0d3b7v3b0h3a7t3a0k3b7p3b0n3b7l3b0r3a7j3a0t3a7j3a0t3a7k3a0s3a7k3a0s3a7k3a0s3a7k3a0s3a7l3a0q3a7f3a7f3a0q3a7d3b0a3b7d3a0q3a7c3a0e3b7b3a0q3a7b3a0h3a7b3a0p3c0j3c0p3a0n3a0am]],
            transparent = 0,
        }
    }
end

------

local speed_patterns   = {
    [1] = [[0000330000000033000000033330000000000000000000000000000000000000000000000000000000000000000000000000]],
    [2] = [[0000330000000033000000033330000033333300000000000000000000000000000000000000000000000000000000000000]],
    [3] = [[0000330000000033000000033330000033883300003888830000033330000000000000000000000000000000000000000000]],
    [4] = [[0003883000000388300000038830000033883300033888833003888888300333333330000000000000000000000000000000]],
    [5] = [[0003883000000388300000038830000033883300033888833003888888300388888830033888833000333333000000000000]],
    [6] = [[0003883000000388300000038830000033883300033888833033888888333888888883388888888333888888330333333330]],
}
local turn_patterns    = {
    [1] = [[0000000000000000000000E00000000EE0000000EEEEEE00000EE000E00000E0000E0000000000E0000000000E000000000E]],
    [2] = [[0000000000000000000000000E00000000EE0000000EEEE0000000EE0E0000000E00E0000000000E000000000E000000000E]],
    [3] = [[000000000000000000000000000E00000000EE0000000EEEEE000000EE0E0000000E0E000000000E000000000E000000000E]],
    [4] = [[0000000000000000000000E000000000EE000000EEEEE00000E0EE000000E0E0000000E000000000E000000000E000000000]],
    [5] = [[000000000000000000000000E000000000EE0000000EEEE00000E0EE00000E00E00000E000000000E000000000E000000000]],
    [6] = [[000000000000000000000000000E000000000EE00000EEEEEE000E000EE000E0000E000E00000000E000000000E000000000]],
}
local bullets_patterns = {
    [1] = [[0000000000000088880000000000000000]],
    [2] = [[0000008888000000000000008888000000]],
    [3] = [[0088880000000088880000000088880000]],
    [4] = [[0088880000888800008888000088880000]],
    [5] = [[0088880088880088880088880088880000]],
    [6] = [[8888008888008888008888008888008888]],
}
local heart_pattern    = [[0b3a0d3a0c3c0b3c0a3ad0a3h0b3h0c3f0e3d0g3b0d]]
local ship_pattern     = [[0i2b0r2b0q2d0p2d0p2d0o2f0n2f0m2h0l2h0l2h0k2j0j2j0j2j0i2l0h2l0h2l0g2f0b2f0f2e0d2e0e2e0f2e0d2d0h2d0d2c0j2c0c2c0l2c0b2b0n2b0a]]

---@param dmd dmd
function gfx.draw_dmd(dmd, game)
    local slots        = game.slots
    local bulletsValue = slots.bullets.dieValue
    local speedValue   = slots.speed.dieValue
    local turnValue    = slots.turn.dieValue

    dmd:clear()

    for i = 1, game.ship.health do
        dmd:blit({ x = 0, y = (i - 1) * 11, width = 10, height = 10 }, heart_pattern)
    end

    dmd:blit({ x = 40, y = 23, width = 20, height = 23 }, ship_pattern)

    if bulletsValue > 0 then
        dmd:blit({ x = 49, y = 5, width = 2, height = 17 }, bullets_patterns[bulletsValue])
    end
    if speedValue > 0 then
        dmd:blit({ x = 45, y = 43, width = 10, height = 10 }, speed_patterns[speedValue])
    end
    if turnValue > 0 then
        local region
        if turnValue >= 1 and turnValue <= 3 then
            region = { x = 30, y = 25, width = 10, height = 10 }
        else
            region = { x = 60, y = 25, width = 10, height = 10 }
        end
        dmd:blit(region, turn_patterns[turnValue])
    end

    dmd:blit({ x = slots.bullets.position.x, y = slots.bullets.position.y, width = 13, height = 13 }, string.rep("3", 13 * 13))
    dmd:blit({ x = slots.turn.position.x, y = slots.turn.position.y, width = 13, height = 13 }, string.rep("3", 13 * 13))
    dmd:blit({ x = slots.speed.position.x, y = slots.speed.position.y, width = 13, height = 13 }, string.rep("3", 13 * 13))
    dmd:blit({ x = 0, y = 0, width = DMDSize.width, height = 1 }, string.rep("6", DMDSize.width))
    dmd:blit({ x = 0, y = DMDSize.height - 2, width = DMDSize.width, height = 1 }, string.rep("3", DMDSize.width))
    dmd:blit({ x = 0, y = DMDSize.height - 1, width = DMDSize.width, height = 1 }, string.rep("5", DMDSize.width))
end

return gfx
