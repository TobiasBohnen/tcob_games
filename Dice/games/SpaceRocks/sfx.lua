local sfx = {
}
---@param engine engine
function sfx.get_sounds(game, engine)
    return {
        [0] = engine.sfx:explosion(96324),
        [1] = engine.sfx:laser_shoot(2478)
    }
end

return sfx
