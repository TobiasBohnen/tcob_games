local img = app.image
if not img then
    return app.alert("No active image")
end

local w, h = img.width, img.height

local function toBase26Letters(n)
    local s = ""
    while n > 0 do
        local rem = (n - 1) % 26
        s         = string.char(97 + rem) .. s
        n         = math.floor((n - 1) / 26)
    end
    return s
end

local data = {}
for y = 0, h - 1 do
    for x = 0, w - 1 do
        data[#data + 1] = string.format("%X", img:getPixel(x, y) & 0xF)
    end
end

local n        = #data
local finalOut = {}
local rleLen   = 0
local rawLen   = n

local rle      = {}
local i        = 1
while i <= n do
    local digit = data[i]
    local count = 1
    while (i + count <= n) and (data[i + count] == digit) do
        count = count + 1
    end
    local rleCount = toBase26Letters(count)
    rleLen         = rleLen + 1 + #rleCount
    rle[#rle + 1]  = digit .. rleCount
    i              = i + count
end

local usedRLE = rleLen < rawLen

app.clipboard.text = table.concat(usedRLE and rle or data)
app.alert("Exported " .. w .. "x" .. h .. " indices to clipboard. RLE used: " .. tostring(usedRLE))
