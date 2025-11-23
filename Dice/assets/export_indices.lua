local spr = app.activeSprite
if not spr then return app.alert("No active sprite") end

local img = Image(spr.width, spr.height, spr.colorMode)

for _, cel in ipairs(spr.cels) do
    if cel.image then
        img:drawImage(cel.image, cel.position)
    end
end

local w, h = img.width, img.height
local lines = { "[[" }

for y = 0, h - 1 do
    local row = {}
    for x = 0, w - 1 do
        row[#row + 1] = string.format("%X", img:getPixel(x, y))
    end
    lines[#lines + 1] = table.concat(row, "")
end

lines[#lines + 1] = "]]"

app.clipboard.text = table.concat(lines, "\n")
app.alert("Copied indices (" .. w .. "x" .. h .. ") to clipboard!")
