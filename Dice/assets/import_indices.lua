local img = app.image
if not img then
    return app.alert("No active image")
end
local w, h = img.width, img.height
local expected = w * h

local content = app.clipboard.text
if not content or #content == 0 then
    return app.alert("Clipboard is empty")
end

content = content:gsub("%s+", "")

local use_rle = content:find("%x[a-z]") ~= nil

local function fromBase26Letters(s)
    local n = 0
    for i = 1, #s do
        local c = s:sub(i, i)
        if c < 'a' or c > 'z' then
            error("Invalid run-length letter: " .. c)
        end
        n = n * 26 + (c:byte() - 96)
    end
    return n
end

local flat = {}

if use_rle then
    local i = 1
    local len = #content
    while i <= len do
        local digit = content:sub(i, i)
        if not digit:match("[0-9A-F]") then
            return app.alert("Invalid HEX digit at pos " .. i .. ": " .. digit)
        end
        i = i + 1

        local start = i
        while i <= len and content:sub(i, i):match("[a-z]") do
            i = i + 1
        end
        if start == i then return app.alert("Missing run-length after " .. digit .. " at pos " .. (i - 1)) end

        local run = fromBase26Letters(content:sub(start, i - 1))
        local val = tonumber(digit, 16)
        for _ = 1, run do flat[#flat + 1] = val end
    end
else
    for i = 1, #content do
        local c = content:sub(i, i)
        local val = tonumber(c, 16)
        if not val then return app.alert("Invalid HEX digit at pos " .. i .. ": " .. c) end
        flat[#flat + 1] = val
    end
end

if #flat ~= expected then
    return app.alert("Error: decompressed size " .. #flat .. " != expected " .. expected)
end

local idx = 1
for y = 0, h - 1 do
    for x = 0, w - 1 do
        img:putPixel(x, y, flat[idx])
        idx = idx + 1
    end
end

app.refresh()
app.alert("Imported " .. w .. "x" .. h .. " indices from clipboard. RLE=" .. tostring(use_rle))
