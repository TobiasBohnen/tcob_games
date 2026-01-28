-- Copyright (c) 2026 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local gfx = {}

---@param engine engine
function gfx.create_background(engine)
    local bg   = engine.bg
    local size = bg.size
    local w, h = size.width, size.height
    local n    = w * h

    local buf  = {}
    for i = 1, n do
        buf[i] = "0"
    end

    local starCount = 75
    for i = 1, starCount do
        local x = math.floor(engine:rnd(0, 1) * w)
        local y = math.floor(engine:rnd(0, 1) * h)
        local s = math.floor(engine:rnd(0, 1) * 2 + 1)

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

    engine.bg:blit(engine.bg.bounds, table.concat(buf))
end

gfx.textures                  = {
    ship      = { [0] = 0, [45] = 1, [90] = 2, [135] = 3, [180] = 4, [225] = 5, [270] = 6, [315] = 7 }, ---@type { [number]: texture }
    hurtShip  = { [0] = 10, [45] = 11, [90] = 12, [135] = 13, [180] = 14, [225] = 15, [270] = 16, [315] = 17 }, ---@type { [number]: texture }
    bullet    = 20, ---@type texture
    asteroid  = { small = 30, medium = 31, large = 32 }, ---@type { [string]: texture }
    explosion = 40, ---@type texture
}

local ship_texture_0          =
[[!0am1b0m1d0l1Eb10l1Eb10k1bEb1b0k1d0l1d0k1f0i1h0g1bDb1bDb1b0e1bD0bDb0bD1b0dDb0hDb0ah]]
local ship_texture_45         =
[[!0be1c0l1e0j1bEb1b0c1iEb10dDc1i0gDb1f0iD1d0kDb1c0mDb10nD10nD10nD10w]]
local hurt_texture_0          =
[[!0v3d0k3b1b3b0j31d30j31Eb130i3b1Eb13b0h31bEb1b30h3b1d3b0h3b1d3b0g3b1f3b0e3b1h3b0c3b1bDb1bDb1b3b0b31bD3bDb3bD1b30b3Db3hDb30b3d0f3d0q]]
local hurt_texture_45         =
[[!0an3e0j3b1c3b0h3b1e30b3g1bEb1b30b31hEb13b0b3Db1i30c3cDb1f3b0e3bD1d3c0g3Db1c30i3cDb130k3bD130l3D130l3D130l3d0f]]
local bullet_texture          =
[[0FF0F88FF88F0FF0]]
local asteroid_small_texture  =
[[!0f5c10i656c1b51c0d5b6e5c1b0c1e6d51d01g6g101f6h501b5b6d1b576b505b156c1c5b760b51c561d5c60b51i5c6b051i5d605b1h5d60b51g56b56b0c616b1d6d0e6d1b6b50j5c60f]]
local asteroid_medium_texture =
[[!0i5d1b0r5c1c0n65b6b1j0h5656b1h6b1b0g5c6b1b5b1c6d10e1b5d1b5f16c1b0b1515d1d5f6c1b0b5f1d6b15c16c1b0b5e1d6d5d6c1b0b5f6b16e5c6c1b0b5f6h5d61c0b5151b5b6d1b615c1c0c1f56c1d6151e0b51g6c1c61g0b5b1d516c1h51c05b1c5c16c1b6b1g05c1b5c1b6f1d51b05c1b5c1c6156c1b5c0d51b5d1c5c61b5d0f1b5b1c5d16b5d0f616d5d6c5d0h6e15b16b5d0m5d6b0b5b0n5d6b0i]]
local asteroid_large_texture  =
[[!0l5h0t1e5h0r1g5b16b5c0o1k6d5f0j1f6c1d6d5f0i161b5b6b1f6d5f0e1c6c5c6b1b5d16e5e0c1c6d5d1b5g6d5e0c1c6c5d1d5g6c15d0c15b165d1f5g61c5c0c5h1e6c15d161d5b0c5g1e6f5d6b1c5c0b5b15e6b1b6f5e61c5b0c51c5d6k5d61d5b0b51c5e6j5e1d5b0b51c51b5b6f1b6b15c1e5c051c5b1b5b6c1e6b15b1d5d051c5b1d6d1e61h5c051d5b1d6d1k515c05b1b5c15b16d1j5f05d1515d16d1b6c1d5f05d1c5d1b6h1e5e05e1b5d1c6h1d5d0b5f15e1c615b6c1c5d0f5c15b65b1d5d61c5e0f5e6c1d5e1b6b5e0g5d6e5g6b5f0i5d6e5e6b5f0l5c6h5h0p5b6g5f0s5b6d5d0v5e6c0l]]
local explosion_texture       =
[[!0f3d0i63h1b0d53c7d3c10c13b7h3b1013b7c8d7c3b013b7b8f7b3b03b7b8h7b3d7b8h7b3d7b8h7b3d7b8h7b3b53b7b8f7b3b053b7c8d7c3b0b53b7h3b60c63c7d3c60e63h0j3d0f]]

gfx.largeAsteroidSize         = 32

---@param engine engine
function gfx.create_textures(game, engine)
    local spr       = engine.spr
    local pen       = { x = 1, y = 1 }
    local rowHeight = 0
    local padding   = 2

    local function create(id, size, tex, rot)
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
                rotation    = rot
            }
        )

        engine:define_texture(id, { x = pen.x, y = pen.y, width = width, height = height })

        pen.x = pen.x + width + padding
        rowHeight = math.max(rowHeight, height)
    end

    create(gfx.textures.ship[0], 16, ship_texture_0, Rot.R0)
    create(gfx.textures.ship[90], 16, ship_texture_0, Rot.R90)
    create(gfx.textures.ship[180], 16, ship_texture_0, Rot.R180)
    create(gfx.textures.ship[270], 16, ship_texture_0, Rot.R270)
    create(gfx.textures.ship[45], 16, ship_texture_45, Rot.R0)
    create(gfx.textures.ship[135], 16, ship_texture_45, Rot.R90)
    create(gfx.textures.ship[225], 16, ship_texture_45, Rot.R180)
    create(gfx.textures.ship[315], 16, ship_texture_45, Rot.R270)

    create(gfx.textures.hurtShip[0], 16, hurt_texture_0, Rot.R0)
    create(gfx.textures.hurtShip[90], 16, hurt_texture_0, Rot.R90)
    create(gfx.textures.hurtShip[180], 16, hurt_texture_0, Rot.R180)
    create(gfx.textures.hurtShip[270], 16, hurt_texture_0, Rot.R270)
    create(gfx.textures.hurtShip[45], 16, hurt_texture_45, Rot.R0)
    create(gfx.textures.hurtShip[135], 16, hurt_texture_45, Rot.R90)
    create(gfx.textures.hurtShip[225], 16, hurt_texture_45, Rot.R180)
    create(gfx.textures.hurtShip[315], 16, hurt_texture_45, Rot.R270)

    create(gfx.textures.asteroid.small, gfx.largeAsteroidSize / 2, asteroid_small_texture, Rot.R0)
    create(gfx.textures.asteroid.medium, gfx.largeAsteroidSize / 4 * 3, asteroid_medium_texture, Rot.R0)
    create(gfx.textures.asteroid.large, gfx.largeAsteroidSize, asteroid_large_texture, Rot.R0)

    create(gfx.textures.bullet, 4, bullet_texture, Rot.R0)
    create(gfx.textures.explosion, 16, explosion_texture, Rot.R0)
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
local heart_pattern    = [[!0b30d30c3c0b3c03ad03h0b3h0c3f0e3d0g3b0d]]
local ship_pattern     = [[!0i2b0r2b0q2d0p2d0p2d0o2f0n2f0m2h0l2h0l2h0k2j0j2j0j2j0i2l0h2l0h2l0g2f0b2f0f2e0d2e0e2e0f2e0d2d0h2d0d2c0j2c0c2c0l2c0b2b0n2b0]]

---@param hud tex
function gfx.draw_hud(hud, game)
    local sockets      = game.sockets ---@type {[string]: socket})
    local bulletsValue = sockets.bullets.die_value
    local speedValue   = sockets.speed.die_value
    local turnValue    = sockets.turn.die_value

    hud:clear()

    for i = 1, game.ship.health do
        hud:blit({ x = 2, y = (i - 1) * 13 + 2, width = 10, height = 10 }, heart_pattern)
    end

    hud:blit({ x = 30, y = 20, width = 20, height = 23 }, ship_pattern)

    if bulletsValue > 0 then
        hud:blit({ x = 39, y = 2, width = 2, height = 17 }, bullets_patterns[bulletsValue])
    end
    if turnValue > 0 then
        hud:blit({ x = 20, y = 12, width = 10, height = 10 }, turn_patterns[turnValue])
    end
    if speedValue > 0 then
        hud:blit({ x = 35, y = 40, width = 10, height = 10 }, speed_patterns[speedValue])
    end

    sockets.bullets.position = { x = 54, y = 4 }
    hud:socket(sockets.bullets)

    sockets.turn.position = { x = 54, y = 24 }
    hud:socket(sockets.turn)

    sockets.speed.position = { x = 54, y = 44 }
    hud:socket(sockets.speed)

    if game.ship.hitByAsteroid then
        hud:print({ x = 1, y = 60 }, "HIT!", Palette.Red)
    end
    if game.ship.shieldsUp then
        hud:print({ x = 1, y = 70 }, "SHIELDS UP!", Palette.Blue)
    end
    if game.ship.engineStall then
        hud:print({ x = 1, y = 80 }, "ENGINE STALL!", Palette.Red)
    end
end

---@param hud tex
function gfx.draw_game_over(hud)
    hud:clear()
    hud:print({ x = 12, y = 12 }, "GAME", Palette.LightBlue)
    hud:print({ x = 36, y = 20 }, "OVER", Palette.LightBlue)
end

return gfx
