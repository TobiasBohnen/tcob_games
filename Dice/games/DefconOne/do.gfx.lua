-- Copyright (c) 2025 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local gfx = {}

---@param engine engine
function gfx.get_background(game, engine)
    local w, h  = ScreenSize.width, ScreenSize.height
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

    return {
        [0] = { bitmap = table.concat(buf) }
    }
end

local function make_texture(w, h, tex, flip)
    return {
        size        = { width = w, height = h },
        bitmap      = tex,
        transparent = 0,
        flip_h      = flip
    }
end

local city_texture_normal =
[[0bd1d0g1c0o1a0b1d0e1g0b1c0g1c0a1a8b1a0e1a8a1a8a1a8a1a0a1e0f1a8a1a0a1d0e1g0a1a8a1a8a1a0f1c0a1a8b1a0e1a8a1c8a1a0a1e0d1a0a1c0a1d0a1a0c1g0a1a8a1c0b1c0a1a8a1a0a1d0a1a0c1c8a1a8a1a0a1e0a1a8a1b0a1c0a1a8b1d0b1g0a1a8a1a8a1a0a1d0a1c0a1g0b1a8a1e0a1e0a1b8a1a0a1a8a1a0a1a8b1b8a1a0b1e8a1a0a1a8a1a8a1f0a1c0a1g0b1g0a1g8a1b0a1c0a1g0b1a8a1r8a1a0a1a8b1b8a1a0b1v0a1g0b1ad0a]]
local city_texture_damaged =
[[0cy1a0c1b0x1a8a1a0c1c0f1a0p1c0c1a8a1a0f1c0l1c8a1a0a1e0d1a0a1c0l1e0a1a8a1c0b1c0a1a8a1a0a1a0j1a8a1a8a1a0a1e0a1a8a1b0a1c0a1a0i1a0a1d0a1a8a1a8a1a0a1d0a1c0a1b0g1a8a1a0a1c0a1e0a1b8a1a0a1a8a1a0a1a8b1b0d1e8a1a0a1a8a1a8a1f0a1c0a1d0a1a0c1g0a1g8a1b0a1c0a1g0b1a8a1r8a1a0a1a8b1b8a1a0b1v0a1g0b1ad0a]]
local city_texture_destroyed =
[[0ny1c0d1a0h1a0c1e0e1f0b1c0a1c0a1d0a1g0b1i0a1l0a1g0b1ad0a]]
local gun_texture =
[[0arCc0jCe3a0hCfDb0bCa0bCfDc0dCd9aCbDb0gCb9aCcDaEa0iCb0bCbDaEa0iCb0aCbDaEa0lCbDaEa0kCdDaEa0g1a2a1h2a1a0c1a2a1a2a1f2a1a2a1a0a1af]]
local missile_texture =
[[0b3d0e3b0c3a0a3d0a3d0b3f0b3d0a3d0a3a0c3b0e3d0b]]

---@param engine engine
function gfx.get_textures(game, engine)
    return {
        [game.cityTextures.normal]    = make_texture(32, 16, city_texture_normal),
        [game.cityTextures.damaged]   = make_texture(32, 16, city_texture_damaged),
        [game.cityTextures.destroyed] = make_texture(32, 16, city_texture_destroyed),
        [game.gunTextures.left]       = make_texture(16, 16, gun_texture),
        [game.gunTextures.right]      = make_texture(16, 16, gun_texture, true),
        [game.missileTexture]         = make_texture(8, 8, missile_texture),
    }
end

------

---@param dmd dmd
function gfx.draw_dmd(dmd, game)

end

return gfx
