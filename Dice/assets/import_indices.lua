local spr = app.activeSprite
if not spr then
    return app.alert("No active sprite to import into")
end

local w = spr.width
local h = spr.height

local content = app.clipboard.text
if not content or #content == 0 then
    return app.alert("Clipboard is empty")
end

content = content:gsub("%s+", "")

local expected = w * h
local actual = #content

if actual ~= expected then
    return app.alert(
        "Pixel count mismatch!\n" ..
        "Expected " .. expected .. " hex digits, got " .. actual
    )
end

if spr.colorMode ~= ColorMode.INDEXED then
    return app.alert("Sprite must be in indexed mode")
end

local img = spr.cels[1].image
local idx = 1

for y = 0, h - 1 do
    for x = 0, w - 1 do
        local ch = content:sub(idx, idx)
        idx = idx + 1

        local val = tonumber(ch, 16)
        if not val then
            return app.alert("Invalid hex digit: " .. ch)
        end

        img:putPixel(x, y, val)
    end
end

app.refresh()
app.alert("Imported " .. w .. "x" .. h .. " indices from clipboard.")
