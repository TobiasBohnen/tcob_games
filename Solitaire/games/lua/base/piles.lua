-- Copyright (c) 2024 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local initial = {
    top_face_up = function(size)
        local retValue = {}
        for i = 1, size - 1 do
            retValue[i] = false
        end
        retValue[size] = true
        return retValue
    end,
    face_up = function(size)
        local retValue = {}
        for i = 1, size do
            retValue[i] = true
        end
        return retValue
    end,
    face_down = function(size)
        local retValue = {}
        for i = 1, size do
            retValue[i] = false
        end
        return retValue
    end,
    alternate = function(size, first)
        local retValue = {}
        for i = 1, size do
            retValue[i] = first
            first = not first
        end
        return retValue
    end
}

return {
    ace_upsuit_top = function() return { Rule = { Build = "UpInSuit", Move = "Top", Empty = "Ace" } } end,
    initial = initial
}
