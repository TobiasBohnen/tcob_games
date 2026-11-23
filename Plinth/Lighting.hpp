// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <span>
#include <vector>

#include "Common.hpp"

////////////////////////////////////////////////////////////

struct dynamic_light {
    point3_d Position {};
    f64      Range {8.0};
    color    Color {colors::White};
    f64      Intensity {1.0};
    u32      Layers {LIGHT_LAYER_WORLD};
};

////////////////////////////////////////////////////////////

class lighting {
public:
    lighting();

    void update(level const& lvl, usize losSamples);

    auto lights() const -> std::span<dynamic_light const>;
    auto lights_at(point_i map) const -> std::span<u32 const>; // indices into lights()

private:
    std::vector<dynamic_light>    _lights;
    std::vector<std::vector<u32>> _cellLights;
    usize                         _losSamples {0};

    task_manager& _taskManager;
};
