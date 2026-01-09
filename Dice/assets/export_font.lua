local spr = app.activeSprite
if not spr then
  return app.alert("No active sprite!")
end

local detectedWidth = spr.gridBounds.width
local detectedHeight = spr.gridBounds.height

local defaultWidth = (detectedWidth > 0) and detectedWidth or 5
local defaultHeight = (detectedHeight > 0) and detectedHeight or 5

local dlg = Dialog("Font Export Settings")
dlg:number {
  id = "glyphWidth",
  label = "Glyph Width:",
  text = tostring(defaultWidth),
  decimals = 0
}
dlg:number {
  id = "glyphHeight",
  label = "Glyph Height:",
  text = tostring(defaultHeight),
  decimals = 0
}
dlg:button { id = "ok", text = "Export" }
dlg:button { id = "cancel", text = "Cancel" }
dlg:show()

local data = dlg.data
if not data.ok then
  return
end

local glyphWidth  = math.floor(data.glyphWidth)
local glyphHeight = math.floor(data.glyphHeight)

if glyphWidth <= 0 or glyphHeight <= 0 then
  return app.alert("Glyph dimensions must be positive!")
end

local sheetWidth, sheetHeight = spr.width, spr.height
local columns                 = math.floor(sheetWidth / glyphWidth)
local rows                    = math.floor(sheetHeight / glyphHeight)

if columns == 0 or rows == 0 then
  return app.alert("Glyph size is larger than sprite dimensions!")
end

local outputLines = {}
for row = 0, rows - 1 do
  for col = 0, columns - 1 do
    local glyphBits = {}
    for y = 0, glyphHeight - 1 do
      local rowBits = 0
      for x = 0, glyphWidth - 1 do
        local pixel = spr.cels[1].image:getPixel(col * glyphWidth + x, row * glyphHeight + y)
        if pixel ~= 0 then
          rowBits = rowBits | (1 << (glyphWidth - 1 - x))
        end
      end
      table.insert(glyphBits, string.format("0x%02X", rowBits))
    end
    table.insert(outputLines, "{ " .. table.concat(glyphBits, ", ") .. " }")
  end
end

local clipboardText = table.concat(outputLines, ",\n")
app.clipboard.text = clipboardText
app.alert(string.format("Export complete! %d glyphs (%dx%d grid) copied to clipboard.",
  columns * rows, columns, rows))
