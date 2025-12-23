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

---@param engine engine
function gfx.get_textures(game, engine)
    return {
    }
end

------

---@param dmd dmd
function gfx.draw_dmd(dmd, game)

end

return gfx
