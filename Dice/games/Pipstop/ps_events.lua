-- Copyright (c) 2026 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local event_base = {
    name       = "",

    value      = 0,
    turnsLeft  = 0,

    sockets    = {},

    ---@param engine engine
    turn_start = function(event, engine, game)
        engine:get_hand(event.sockets)

        event.turnsLeft = event.turnsLeft - 1
        if event.turnsLeft == 0 then
            event:resolve(engine, game)
            return true
        end

        return false
    end,

    init       = function(event, engine, game)
    end,

    update     = function(event, engine, game)
    end,

    resolve    = function(event, engine, game)
    end,
}

function event_base:create(overrides)
    local event = {}

    for k, v in pairs(self) do
        if k ~= "create" then
            event[k] = v
        end
    end

    if overrides then
        for k, v in pairs(overrides) do
            event[k] = v
        end
    end

    return event
end

------
------

local events = {}

events.curve = event_base:create({
    name = "Sharp Curve",
    value = 100,
    turnsLeft = 2,
    slotCount = 1,

    init = function(event, engine, game)
        print("Curve appears in " .. event.turnsLeft .. " turns")
    end,

    resolve = function(event, engine, game)
        -- Check dice combo
        local combo = game:get_combo()
        if combo.score >= event.value then
            game.score = game.score + combo.score
        else
            game.health = game.health - 15
        end
    end
})

return events
