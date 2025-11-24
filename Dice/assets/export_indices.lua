local img = app.image
if not img then
    return app.alert("No active image")
end
local w, h = img.width, img.height
local out = {}

for y = 0, h - 1 do
    for x = 0, w - 1 do
        out[#out + 1] = string.format("%X", img:getPixel(x, y))
    end
end

app.clipboard.text = table.concat(out, "")
app.alert("Exported " .. w .. "x" .. h .. " indices to clipboard.")
