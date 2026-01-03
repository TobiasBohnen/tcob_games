-- Copyright (c) 2026 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local sfx = {
}
---@param engine engine
function sfx.get_sounds(game, engine)
    return {
        [game.sounds.missileExplosion] = {
            random_seed = 0,
            sample_rate = 44100,
            wave_type = "Noise",
            attack_time = 0.000000,
            sustain_time = 0.295341,
            sustain_punch = 0.753663,
            decay_time = 0.380711,
            start_frequency = 0.074670,
            min_frequency = 0.000000,
            slide = 0.051724,
            delta_slide = 0.000000,
            vibrato_depth = 0.000000,
            vibrato_speed = 0.000000,
            change_amount = 0.758172,
            change_speed = 0.758144,
            square_duty = 0.000000,
            duty_sweep = 0.000000,
            repeat_speed = 0.000000,
            phaser_offset = 0.162462,
            phaser_sweep = -0.149039,
            lpf_cutoff = 1.000000,
            lpf_cutoff_sweep = 0.000000,
            lpf_resonance = 0.000000,
            hpf_cutoff = 0.000000,
            hpf_cutoff_sweep = 0.000000
        },
        [game.sounds.cityExplosion] = {
            random_seed = 0,
            sample_rate = 44100,
            wave_type = "Noise",
            attack_time = 0.000000,
            sustain_time = 0.288605,
            sustain_punch = 0.767008,
            decay_time = 0.484835,
            start_frequency = 0.086480,
            min_frequency = 0.000000,
            slide = -0.392799,
            delta_slide = 0.000000,
            vibrato_depth = 0.261103,
            vibrato_speed = 0.514203,
            change_amount = -0.020564,
            change_speed = 0.849670,
            square_duty = 0.000000,
            duty_sweep = 0.000000,
            repeat_speed = 0.424327,
            phaser_offset = 0.000000,
            phaser_sweep = 0.000000,
            lpf_cutoff = 1.000000,
            lpf_cutoff_sweep = 0.000000,
            lpf_resonance = 0.000000,
            hpf_cutoff = 0.000000,
            hpf_cutoff_sweep = 0.000000
        },
        [game.sounds.cannon] = {
            random_seed = 0,
            sample_rate = 44100,
            wave_type = "Sawtooth",
            attack_time = 0.000000,
            sustain_time = 0.191656,
            sustain_punch = 0.000000,
            decay_time = 0.343566,
            start_frequency = 0.778364,
            min_frequency = 0.200000,
            slide = 0.215233,
            delta_slide = 0.000000,
            vibrato_depth = 0.000000,
            vibrato_speed = 0.000000,
            change_amount = 0.000000,
            change_speed = 0.000000,
            square_duty = 0.138784,
            duty_sweep = 0.045617,
            repeat_speed = 0.000000,
            phaser_offset = 0.000000,
            phaser_sweep = 0.000000,
            lpf_cutoff = 1.000000,
            lpf_cutoff_sweep = 0.000000,
            lpf_resonance = 0.000000,
            hpf_cutoff = 0.000000,
            hpf_cutoff_sweep = 0.000000
        }
    }
end

return sfx
