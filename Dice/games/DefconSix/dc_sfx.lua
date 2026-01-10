-- Copyright (c) 2026 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local sfx  = {}

sfx.sounds = {
    missileExplosion = 1, ---@type sound
    cityExplosion    = 2, ---@type sound
    cannon           = 3, ---@type sound
}

---@param engine engine
function sfx.create_sounds(game, engine)
    engine:define_sound(sfx.sounds.missileExplosion, {
        random_seed      = 0,
        sample_rate      = 44100,
        wave_type        = "Noise",
        attack_time      = 0.0,
        sustain_time     = 0.295341,
        sustain_punch    = 0.753663,
        decay_time       = 0.380711,
        start_frequency  = 0.074670,
        min_frequency    = 0.0,
        slide            = 0.051724,
        delta_slide      = 0.0,
        vibrato_depth    = 0.0,
        vibrato_speed    = 0.0,
        change_amount    = 0.758172,
        change_speed     = 0.758144,
        square_duty      = 0.0,
        duty_sweep       = 0.0,
        repeat_speed     = 0.0,
        phaser_offset    = 0.162462,
        phaser_sweep     = -0.149039,
        lpf_cutoff       = 1.0,
        lpf_cutoff_sweep = 0.0,
        lpf_resonance    = 0.0,
        hpf_cutoff       = 0.0,
        hpf_cutoff_sweep = 0.0
    })
    engine:define_sound(sfx.sounds.cityExplosion, {
        random_seed      = 0,
        sample_rate      = 44100,
        wave_type        = "Noise",
        attack_time      = 0.0,
        sustain_time     = 0.288605,
        sustain_punch    = 0.767008,
        decay_time       = 0.484835,
        start_frequency  = 0.086480,
        min_frequency    = 0.0,
        slide            = -0.392799,
        delta_slide      = 0.0,
        vibrato_depth    = 0.261103,
        vibrato_speed    = 0.514203,
        change_amount    = -0.020564,
        change_speed     = 0.849670,
        square_duty      = 0.0,
        duty_sweep       = 0.0,
        repeat_speed     = 0.424327,
        phaser_offset    = 0.0,
        phaser_sweep     = 0.0,
        lpf_cutoff       = 1.0,
        lpf_cutoff_sweep = 0.0,
        lpf_resonance    = 0.0,
        hpf_cutoff       = 0.0,
        hpf_cutoff_sweep = 0.0
    })
    engine:define_sound(sfx.sounds.cannon, {
        random_seed      = 0,
        sample_rate      = 44100,
        wave_type        = "Sawtooth",
        attack_time      = 0.0,
        sustain_time     = 0.191656,
        sustain_punch    = 0.0,
        decay_time       = 0.343566,
        start_frequency  = 0.778364,
        min_frequency    = 0.2,
        slide            = 0.215233,
        delta_slide      = 0.0,
        vibrato_depth    = 0.0,
        vibrato_speed    = 0.0,
        change_amount    = 0.0,
        change_speed     = 0.0,
        square_duty      = 0.138784,
        duty_sweep       = 0.045617,
        repeat_speed     = 0.0,
        phaser_offset    = 0.0,
        phaser_sweep     = 0.0,
        lpf_cutoff       = 1.0,
        lpf_cutoff_sweep = 0.0,
        lpf_resonance    = 0.0,
        hpf_cutoff       = 0.0,
        hpf_cutoff_sweep = 0.0
    })
end

return sfx
