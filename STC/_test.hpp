// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Mission.hpp"

class test_mission : public mission {
public:
    test_mission()
    {
        set_station({});
    }

    void on_mouse_button_down(input::mouse::button_event const& ev) override
    {
        size_f const size {25, 50};
        auto         ship {add_ship({
                    .Bounds   = {point_f {ev.Position} - point_f {size.Width / 2, size.Height / 2}, size},
                    .Rotation = degree_f {_rng(0.f, 360.f)},

                    .MaxSpeed        = 3,
                    .MaxAcceleration = 250.f,
                    .MaxTurnrate     = degree_f {_rng(45.f, 90.f)},
        })};
    }

private:
    rng _rng;
};
