local spr = app.activeSprite
if not spr then
    return app.alert("No active sprite")
end

local img = Image(spr.width, spr.height, ColorMode.INDEXED)

for _, cel in ipairs(spr.cels) do
    if cel.image then
        img:drawImage(cel.image, cel.position)
    end
end

local w, h = img.width, img.height
local buf = {}

for y = 0, h - 1 do
    for x = 0, w - 1 do
        buf[#buf + 1] = string.format("%X", img:getPixel(x, y))
    end
end

local out = "[[" .. table.concat(buf, "") .. "]]"

app.clipboard.text = out
app.alert("Exported " .. w .. "x" .. h .. " indices to clipboard.")
