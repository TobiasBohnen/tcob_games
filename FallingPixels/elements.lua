-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local neighbors = { { -1, -1 }, { -1, 0 }, { -1, 1 }, { 1, -1 }, { 1, 0 }, { 1, 1 }, { 0, -1 }, { 0, 1 } }

----------------------
local empty = {
    Color          = "Transparent",
    ColorVariation = 0,
    SpawnCount     = 100,
    DefaultGravity = false,
    Density        = 100,
    Type           = "None",
}
register_element(0, "Empty", empty)

----------------------
local olive_oil = {
    Color          = "Olive",
    ColorVariation = 8,
    SpawnCount     = 1,
    DefaultGravity = true,
    Density        = 0.9,
    Type           = "Liquid",
}
register_element(1, "OliveOil", olive_oil)

----------------------
local sand = {
    Color          = "SandyBrown",
    ColorVariation = 8,
    SpawnCount     = 1,
    DefaultGravity = true,
    Density        = 2.5,
    Type           = "Powder",
}
register_element(2, "Sand", sand)

----------------------
local salt = {
    Color          = "#F3FAFD",
    ColorVariation = 24,
    SpawnCount     = 1,
    DefaultGravity = true,
    Density        = 2.19,
    Type           = "Powder",
    Update         = function(system, pos)
        for _, n in ipairs(neighbors) do
            local neighborPos = { x = pos.x + n[1], y = pos.y + n[2] }
            if system:name(neighborPos) == "Water" then
                system:clear(pos)
                break
            end
        end
    end
}
register_element(3, "Salt", salt)

----------------------
local sawdust = {
    Color          = "#F9EAD4",
    ColorVariation = 16,
    SpawnCount     = 1,
    DefaultGravity = true,
    Density        = 0.21,
    Type           = "Powder",
}
register_element(4, "Sawdust", sawdust)

----------------------
local water = {
    Color          = "Blue",
    ColorVariation = 16,
    SpawnCount     = 20,
    DefaultGravity = true,
    Density        = 1,
    Type           = "Liquid",
}
register_element(5, "Water", water)

----------------------
local wall = {
    Color          = "DarkGray",
    ColorVariation = 0,
    SpawnCount     = 100,
    DefaultGravity = false,
    Density        = 100,
    Type           = "Solid",
}
register_element(6, "Wall", wall)

----------------------
