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

    sockets     = nil,
    socketCount = 0,

    finished    = false,

    init        = function(event)
        event.sockets = {}

        assert(event.socketCount <= 5)
        for i = 1, event.socketCount do
            event.sockets[#event.sockets + 1] = socket.new { colors = { Palette.White } }
        end

        if event.on_init then
            event:on_init()
        end
    end,

    turn_start  = function(event)
        local s, h      = socket.get_value(event.sockets)
        event.value     = event.value + (s + h * 5)
        event.turnsLeft = event.turnsLeft - 1

        if event.turnsLeft == 0 then
            event:resolve()
        end
    end,

    update      = function(event, deltaTime, turnTime)
        if event.on_update then
            event:on_update(deltaTime, turnTime)
        end
    end,

    resolve     = function(event)
        if event.on_resolve then
            local factor = event.value / event.target
            event:on_resolve(factor)
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

---@param engine engine
local function start_event(game, engine)
    return event_base:create({
        title       = "START YOUR ENGINE!",
        target      = 22,
        turnsLeft   = 1,

        socketCount = 2,

        on_resolve  = function(event, factor)
            game.car:set_target_speed(factor * 100)
        end
    })
end

--TODO events:
-- shift gears -> trigger: target speed reaced -> speed up on success
-- speed trap  -> trigger: speed above 180     -> speed down on failure
-- maintenance -> speed down/lose fuel on failure
-- gas station -> trigger: below 10% fuel
---@param engine engine
local function traffic_event(game, engine)
    local traffic_1 = event_base:create({
        title       = "TRAFFIC AHEAD!",
        target      = 22,
        turnsLeft   = 3,

        socketCount = 3,

        on_init     = function(event)
        end,

        on_update   = function(event, deltaTime, turnTime)
            if engine:irnd(0, 1000) == 1 then
                --game:try_spawn_opponent(engine)
            end
        end,

        on_resolve  = function(event, factor)
        end
    })

    return traffic_1
end

local events = {
    get_start = function(self, game, engine)
        return start_event(game, engine)
    end,

    get_next = function(self, game, engine) ---@return event_base
        return traffic_event(game, engine)
    end
}

return events
