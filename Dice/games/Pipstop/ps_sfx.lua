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
function sfx.get_sounds(game, engine)
    local tab = {}
    for speed = 1, 10 do
        tab[sfx.sounds["car_speed_" .. speed]] = {
            random_seed      = 0,
            sample_rate      = 44100,
            wave_type        = "Square",

            attack_time      = 0.0256, -- 2ms
            sustain_time     = 0.0515, -- 6ms
            sustain_punch    = 0.3,
            decay_time       = 0.0256, -- 2ms

            start_frequency  = 0.08 + (speed - 1) * 0.06,
            min_frequency    = 0.0,
            slide            = 0.4 + (speed - 1) * 0.04,
            delta_slide      = -0.15 - (speed - 1) * 0.01,

            vibrato_depth    = 0.005 + (speed - 1) * 0.002,
            vibrato_speed    = 0.5 + (speed - 1) * 0.1,

            change_amount    = 0.6 + (speed - 1) * 0.05,
            change_speed     = 0.8 + (speed - 1) * 0.05,

            square_duty      = 0.68 - (speed - 1) * 0.03,
            duty_sweep       = 0.15 + (speed - 1) * 0.02,

            repeat_speed     = 0.0,

            phaser_offset    = -0.5 - (speed - 1) * 0.02,
            phaser_sweep     = 0.6 + (speed - 1) * 0.03,

            lpf_cutoff       = 0.75 + (speed - 1) * 0.02,
            lpf_cutoff_sweep = 0.02 + (speed - 1) * 0.005,
            lpf_resonance    = 0.6 + (speed - 1) * 0.02,
            hpf_cutoff       = 0.05 + (speed - 1) * 0.015,
            hpf_cutoff_sweep = 0.02 + (speed - 1) * 0.003
        }
    end

    return tab
end

return sfx
