local img = app.image
if not img then
    return app.alert("No active image")
end
local w, h = img.width, img.height

local function toBase26Letters(n)
    local s = ""
    while n > 0 do
        local rem = (n - 1) % 26
        s = string.char(97 + rem) .. s -- 97 = 'a'
        n = math.floor((n - 1) / 26)
    end
    return s
end

local data = {}
for y = 0, h - 1 do
    for x = 0, w - 1 do
        data[#data + 1] = string.format("%X", img:getPixel(x, y) & 0xF)
    end
end

local out = {}
local i = 1
while i <= #data do
    local digit = data[i]
    local count = 1
    while (i + count <= #data) and (data[i + count] == digit) do
        count = count + 1
    end
    out[#out + 1] = digit .. toBase26Letters(count)
    i = i + count
end

app.clipboard.text = table.concat(out, "")
app.alert("Exported " .. w .. "x" .. h .. " RLE indices to clipboard.")
