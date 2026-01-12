-- Copyright (c) 2026 Tobias Bohnen
-- MIT License
-- https://opensource.org/licenses/MIT

local sfx  = {}

sfx.sounds = {
    car_speed_1  = 1,
    car_speed_2  = 2,
    car_speed_3  = 3,
    car_speed_4  = 4,
    car_speed_5  = 5,
    car_speed_6  = 6,
    car_speed_7  = 7,
    car_speed_8  = 8,
    car_speed_9  = 9,
    car_speed_10 = 10,
}

---@param engine engine
function sfx.create_sounds(game, engine)
    for speed = 1, 10 do
        local tab = {
            random_seed      = 0,
            sample_rate      = 44100,
            wave_type        = "Square",
            attack_time      = 0.0256,
            sustain_time     = 0.0515,
            sustain_punch    = 0.3,
            decay_time       = 0.0256,
            start_frequency  = 0.008 + (speed - 1) * 0.015,
            min_frequency    = 0.0,
            slide            = 0.15 + (speed - 1) * 0.05,
            delta_slide      = -0.08 - (speed - 1) * 0.015,
            vibrato_depth    = 0.002 + (speed - 1) * 0.003,
            vibrato_speed    = 0.3 + (speed - 1) * 0.15,
            change_amount    = 0.3 + (speed - 1) * 0.08,
            change_speed     = 0.5 + (speed - 1) * 0.08,
            square_duty      = 0.75 - (speed - 1) * 0.05,
            duty_sweep       = 0.1 + (speed - 1) * 0.04,
            repeat_speed     = 0.0,
            phaser_offset    = -0.5 - (speed - 1) * 0.03,
            phaser_sweep     = 0.5 + (speed - 1) * 0.05,
            lpf_cutoff       = 0.6 + (speed - 1) * 0.04,
            lpf_cutoff_sweep = 0.01 + (speed - 1) * 0.01,
            lpf_resonance    = 0.5 + (speed - 1) * 0.04,
            hpf_cutoff       = 0.03 + (speed - 1) * 0.025,
            hpf_cutoff_sweep = 0.01 + (speed - 1) * 0.006
        }
        engine:define_sound(sfx.sounds["car_speed_" .. speed], tab)
    end
end

return sfx
