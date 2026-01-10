-- Copyright (c) 2026 Tobias Bohnen
--
-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

---@class event_base
local event_base = {
    title       = "",
    value       = 0,
    target      = 0,
    outcome     = "",
    turnsLeft   = 0,

    sockets     = {},
    socketCount = 0,

    finished    = false,

    ---@param engine engine
    init        = function(event, game, engine)
        assert(event.socketCount <= 5)
        for i = 1, event.socketCount do
            event.sockets[#event.sockets + 1] = socket.new { colors = { Palette.White } }
        end

        if event.on_init then
            event:on_init()
        end
    end,

    ---@param engine engine
    turn_start  = function(event, game, engine)
        event.value     = event.value + get_value(event.sockets, 5)
        event.turnsLeft = event.turnsLeft - 1

        if event.turnsLeft == 0 then
            event:resolve(game, engine)
        end
    end,

    ---@param engine engine
    update      = function(event, game, engine, deltaTime, turnTime)
        if event.on_update then
            event:on_update(deltaTime, turnTime)
        end
    end,

    ---@param engine engine
    resolve     = function(event, game, engine)
        if event.on_resolve then
            event:on_resolve()
        end

        for key, value in pairs(event.sockets) do
            value:remove()
        end
        event.sockets  = nil
        event.finished = true
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

local events = {
    get_start = function(self, game, engine)
        return event_base:create({
            title       = "START YOUR ENGINE!",
            target      = 22,
            turnsLeft   = 1,

            socketCount = 2,

            on_resolve  = function(event)
                local value = event.value
                local speed = 10
                if     value >= 22 then
                    speed = 100
                elseif value >= 18 then
                    speed = 80
                elseif value >= 12 then
                    speed = 60
                elseif value >= 10 then
                    speed = 30
                elseif value >= 3 then
                    speed = 20
                end
                game.car:set_target_speed(speed)
            end
        })
    end,

    get_next = function(self, game, engine) ---@return event_base
        return event_base:create({
            name        = "Sharp Curve",
            turnsLeft   = 2,
            socketCount = 1,

        })
    end
}

return events
