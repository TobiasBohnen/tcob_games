// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"
#include "Object.hpp"

////////////////////////////////////////////////////////////

class flight_plan {
public:
private:
};

////////////////////////////////////////////////////////////

class spacecraft : public object {
public:
    struct def {
        rect_f   Bounds {};
        degree_f Rotation {};

        f32      MaxSpeed {};
        f32      MaxAcceleration {};
        degree_f MaxTurnrate {};
    };

    spacecraft(def const& def, std::shared_ptr<physics::body> body, std::shared_ptr<gfx::rect_shape> shape);

protected:
    void on_update(milliseconds deltaTime) override;

private:
    void setup();

    std::unique_ptr<flight_plan> _flightPlan;
    def                          _def;
};