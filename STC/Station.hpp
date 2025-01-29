// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"
#include "Object.hpp"

////////////////////////////////////////////////////////////

class station : public object {
public:
    struct def {
    };

    enum class traffic_controller {
        Approach,
        Tower,
        Ground,
        Departure
    };
    enum class sensor {
        IR,
        Radar,
        Visual
    };
    enum class berth {
        Port,
        Bay,
        Pad
    };

    station(def const& def, std::shared_ptr<physics::body> body, std::shared_ptr<gfx::rect_shape> shape);

private:
    std::vector<traffic_controller> _controllers;
    std::vector<berth>              _berths;
    std::vector<sensor>             _sensors;
};
