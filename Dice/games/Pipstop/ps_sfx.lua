-- Copyright (c) 2026 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local sfx = {
}
---@param engine engine
function sfx.get_sounds(game, engine)
    return {
        [game.sounds.car] = {
            random_seed      = 0,
            sample_rate      = 44100,
            wave_type        = "Square",
            attack_time      = 0.3,
            sustain_time     = 1.0,
            sustain_punch    = 0.0,
            decay_time       = 0.2,
            start_frequency  = 0.13,
            min_frequency    = 0.0,
            slide            = 0.576643,
            delta_slide      = -0.203503,
            vibrato_depth    = 0.006862,
            vibrato_speed    = 0.028243,
            change_amount    = 0.72,
            change_speed     = 0.46,
            square_duty      = 0.68,
            duty_sweep       = 0.19,
            repeat_speed     = 1.0,
            phaser_offset    = -0.67,
            phaser_sweep     = 0.743782,
            lpf_cutoff       = 0.83,
            lpf_cutoff_sweep = 0.03,
            lpf_resonance    = 0.699081,
            hpf_cutoff       = 0.08,
            hpf_cutoff_sweep = 0.032644
        }
    }
end

return sfx
