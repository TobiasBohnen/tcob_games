-- Copyright (c) 2026 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local sfx = {
}
---@param engine engine
function sfx.get_sounds(game, engine)
    return {
        [game.sounds.explosion] = {
            random_seed      = 96324,
            sample_rate      = 44100,
            wave_type        = "Noise",
            attack_time      = 0.0,
            sustain_time     = 0.269872,
            sustain_punch    = 0.774597,
            decay_time       = 0.158956,
            start_frequency  = 0.265540,
            min_frequency    = 0.0,
            slide            = 0.0,
            delta_slide      = 0.0,
            vibrato_depth    = 0.029852,
            vibrato_speed    = 0.122890,
            change_amount    = 0.0,
            change_speed     = 0.0,
            square_duty      = 0.0,
            duty_sweep       = 0.0,
            repeat_speed     = 0.0,
            phaser_offset    = 0.433521,
            phaser_sweep     = -0.052679,
            lpf_cutoff       = 1.0,
            lpf_cutoff_sweep = 0.0,
            lpf_resonance    = 0.0,
            hpf_cutoff       = 0.0,
            hpf_cutoff_sweep = 0.0
        },
        [game.sounds.bullet] = {
            random_seed      = 2478,
            sample_rate      = 44100,
            wave_type        = "Sawtooth",
            attack_time      = 0.0,
            sustain_time     = 0.150909,
            sustain_punch    = 0.0,
            decay_time       = 0.275454,
            start_frequency  = 0.874541,
            min_frequency    = 0.225085,
            slide            = 0.286607,
            delta_slide      = 0.0,
            vibrato_depth    = 0.0,
            vibrato_speed    = 0.0,
            change_amount    = 0.0,
            change_speed     = 0.0,
            square_duty      = 0.292426,
            duty_sweep       = 0.008263,
            repeat_speed     = 0.0,
            phaser_offset    = 0.071596,
            phaser_sweep     = -0.082881,
            lpf_cutoff       = 1.0,
            lpf_cutoff_sweep = 0.0,
            lpf_resonance    = 0.0,
            hpf_cutoff       = 0.105782,
            hpf_cutoff_sweep = 0.0
        }
    }
end

return sfx
