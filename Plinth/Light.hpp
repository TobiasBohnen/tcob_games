// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp"

struct dynamic_light {
    point_d Position {};
    f64     Z {0.5};
    f64     Range {8.0};
    color   Color {colors::White};
    f64     Intensity {1.0};
    u32     Layers {LIGHT_LAYER_WORLD};
};
