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

    return {
        [0] = {
            bitmap = table.concat(buf)
        }
    }
end

local ship_texture_0          =
[[0am1b0m1d0l1aEb1a0l1aEb1a0k1bEb1b0k1d0l1d0k1f0i1h0g1bDb1bDb1b0e1bDa0bDb0bDa1b0dDb0hDb0ah]]
local ship_texture_45         =
[[0be1c0l1e0j1bEb1b0c1iEb1a0dDc1i0gDb1f0iDa1d0kDb1c0mDb1a0nDa1a0nDa1a0nDa1a0w]]
local hurt_texture_0          =
[[0v3d0k3b1b3b0j3a1d3a0j3a1aEb1a3a0i3b1aEb1a3b0h3a1bEb1b3a0h3b1d3b0h3b1d3b0g3b1f3b0e3b1h3b0c3b1bDb1bDb1b3b0b3a1bDa3bDb3bDa1b3a0b3aDb3hDb3a0b3d0f3d0q]]
local hurt_texture_45         =
[[0an3e0j3b1c3b0h3b1e3a0b3g1bEb1b3a0b3a1hEb1a3b0b3aDb1i3a0c3cDb1f3b0e3bDa1d3c0g3aDb1c3a0i3cDb1a3a0k3bDa1a3a0l3aDa1a3a0l3aDa1a3a0l3d0f]]
local bullet_texture          =
[[0aFb0aFa8bFb8bFa0aFb0a]]
local asteroid_small_texture  =
[[0f5c1a0i6a5a6c1b5a1c0d5b6e5c1b0c1e6d5a1d0a1g6g1a0a1f6h5a0a1b5b6d1b5a7a6b5a0a5b1a5a6c1c5b7a6a0b5a1c5a6a1d5c6a0b5a1i5c6b0a5a1i5d6a0a5b1h5d6a0b5a1g5a6b5a6b0c6a1a6b1d6d0e6d1b6b5a0j5c6a0f]]
local asteroid_medium_texture =
[[0i5d1b0r5c1c0n6a5b6b1j0h5a6a5a6b1h6b1b0g5c6b1b5b1c6d1a0e1b5d1b5f1a6c1b0b1a5a1a5d1d5f6c1b0b5f1d6b1a5c1a6c1b0b5e1d6d5d6c1b0b5f6b1a6e5c6c1b0b5f6h5d6a1c0b5a1a5a1b5b6d1b6a1a5c1c0c1f5a6c1d6a1a5a1e0b5a1g6c1c6a1g0b5b1d5a1a6c1h5a1c0a5b1c5c1a6c1b6b1g0a5c1b5c1b6f1d5a1b0a5c1b5c1c6a1a5a6c1b5c0d5a1b5d1c5c6a1b5d0f1b5b1c5d1a6b5d0f6a1a6d5d6c5d0h6e1a5b1a6b5d0m5d6b0b5b0n5d6b0i]]
local asteroid_large_texture  =
[[0l5h0t1e5h0r1g5b1a6b5c0o1k6d5f0j1f6c1d6d5f0i1a6a1b5b6b1f6d5f0e1c6c5c6b1b5d1a6e5e0c1c6d5d1b5g6d5e0c1c6c5d1d5g6c1a5d0c1a5b1a6a5d1f5g6a1c5c0c5h1e6c1a5d1a6a1d5b0c5g1e6f5d6b1c5c0b5b1a5e6b1b6f5e6a1c5b0c5a1c5d6k5d6a1d5b0b5a1c5e6j5e1d5b0b5a1c5a1b5b6f1b6b1a5c1e5c0a5a1c5b1b5b6c1e6b1a5b1d5d0a5a1c5b1d6d1e6a1h5c0a5a1d5b1d6d1k5a1a5c0a5b1b5c1a5b1a6d1j5f0a5d1a5a1a5d1a6d1b6c1d5f0a5d1c5d1b6h1e5e0a5e1b5d1c6h1d5d0b5f1a5e1c6a1a5b6c1c5d0f5c1a5b6a5b1d5d6a1c5e0f5e6c1d5e1b6b5e0g5d6e5g6b5f0i5d6e5e6b5f0l5c6h5h0p5b6g5f0s5b6d5d0v5e6c0l]]
local explosion_texture       =
[[0f3d0i6a3h1b0d5a3c7d3c1a0c1a3b7h3b1a0a1a3b7c8d7c3b0a1a3b7b8f7b3b0a3b7b8h7b3d7b8h7b3d7b8h7b3d7b8h7b3b5a3b7b8f7b3b0a5a3b7c8d7c3b0b5a3b7h3b6a0c6a3c7d3c6a0e6a3h0j3d0f]]


local function make_texture(size, tex, r)
    local function square(x) return { width = x, height = x } end
    return {
        size        = square(size),
        bitmap      = tex,
        transparent = 0,
        rotation    = r
    }
end

---@param engine engine
function gfx.get_textures(game, engine)
    return {
        [game.shipTextures[0]]         = make_texture(16, ship_texture_0, Rot.R0),
        [game.shipTextures[90]]        = make_texture(16, ship_texture_0, Rot.R90),
        [game.shipTextures[180]]       = make_texture(16, ship_texture_0, Rot.R180),
        [game.shipTextures[270]]       = make_texture(16, ship_texture_0, Rot.R270),

        [game.shipTextures[45]]        = make_texture(16, ship_texture_45, Rot.R0),
        [game.shipTextures[135]]       = make_texture(16, ship_texture_45, Rot.R90),
        [game.shipTextures[225]]       = make_texture(16, ship_texture_45, Rot.R180),
        [game.shipTextures[315]]       = make_texture(16, ship_texture_45, Rot.R270),

        [game.shipHurtTextures[0]]     = make_texture(16, hurt_texture_0, Rot.R0),
        [game.shipHurtTextures[90]]    = make_texture(16, hurt_texture_0, Rot.R90),
        [game.shipHurtTextures[180]]   = make_texture(16, hurt_texture_0, Rot.R180),
        [game.shipHurtTextures[270]]   = make_texture(16, hurt_texture_0, Rot.R270),

        [game.shipHurtTextures[45]]    = make_texture(16, hurt_texture_45, Rot.R0),
        [game.shipHurtTextures[135]]   = make_texture(16, hurt_texture_45, Rot.R90),
        [game.shipHurtTextures[225]]   = make_texture(16, hurt_texture_45, Rot.R180),
        [game.shipHurtTextures[315]]   = make_texture(16, hurt_texture_45, Rot.R270),

        [game.bulletTexture]           = make_texture(4, bullet_texture, Rot.R0),

        [game.asteroidTextures.small]  = make_texture(16, asteroid_small_texture, Rot.R0),
        [game.asteroidTextures.medium] = make_texture(24, asteroid_medium_texture, Rot.R0),
        [game.asteroidTextures.large]  = make_texture(32, asteroid_large_texture, Rot.R0),

        [game.explosionTexture]        = make_texture(16, explosion_texture, Rot.R0),
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
    local slots        = game.slots ---@type {[string]: slot})
    local bulletsValue = slots.bullets.die_value
    local speedValue   = slots.speed.die_value
    local turnValue    = slots.turn.die_value

    dmd:clear()

    for i = 1, game.ship.health do
        dmd:blit({ x = 0, y = (i - 1) * 11, width = 10, height = 10 }, heart_pattern)
    end

    dmd:blit({ x = 30, y = 50, width = 20, height = 23 }, ship_pattern)

    if bulletsValue > 0 then
        dmd:blit({ x = 39, y = 32, width = 2, height = 17 }, bullets_patterns[bulletsValue])
    end
    if turnValue > 0 then
        dmd:blit({ x = 20, y = 42, width = 10, height = 10 }, turn_patterns[turnValue])
    end
    if speedValue > 0 then
        dmd:blit({ x = 35, y = 70, width = 10, height = 10 }, speed_patterns[speedValue])
    end


    local function draw_slot(pos, state)
        local col = state == SlotState.Accept and "3" or "9"
        dmd:blit({ x = pos.x, y = pos.y, width = 13, height = 13 }, string.rep(col, 13 * 13))
        dmd:blit({ x = pos.x + 1, y = pos.y + 1, width = 11, height = 11 }, string.rep("A", 11 * 11))
    end

    slots.bullets.position = { x = 55, y = 30 }
    draw_slot(slots.bullets.position, slots.bullets.state)

    slots.turn.position = { x = 55, y = 50 }
    draw_slot(slots.turn.position, slots.turn.state)

    slots.speed.position = { x = 55, y = 70 }
    draw_slot(slots.speed.position, slots.speed.state)

    if game.powerup then
        dmd:print({ x = 7, y = 90 }, "3 OF A KIND", Palette.Green)
    end
end

function gfx.draw_game_over(dmd, game)
    dmd:clear()
    dmd:print({ x = 12, y = 12 }, "GAME", Palette.DarkBlue)
    dmd:print({ x = 36, y = 20 }, "OVER", Palette.BlueGray)
end

return gfx
