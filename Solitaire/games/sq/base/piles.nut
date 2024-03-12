// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

local rules = dofile("./base/rules.nut", true)

local initial_tab = {
    top_face_up = function(size) {
        local retValue = []
        for (local i = 0; i < size - 1; i += 1) {
            retValue.append(false)
        }
        retValue.append(true)
        return retValue
    },
    face_up = function(size) {
        local retValue = []
        for (local i = 0; i < size; i += 1) {
            retValue.append(true)
        }
        return retValue
    },
    face_down = function(size) {
        local retValue = []
        for (local i = 0; i < size; i += 1) {
            retValue.append(false)
        }
        return retValue
    },
    alternate = function(size, first) {
        local retValue = []
        for (local i = 0; i < size; i += 1) {
            retValue.append(first)
            first = !first
        }
        return retValue
    }
}

return {
    ace_upsuit_top = {
        Rule = {
            Build = rules.Build.UpInSuit,
            Move = rules.Move.Top,
            Empty = rules.Empty.Ace
        }
    },
    Initial = initial_tab
}