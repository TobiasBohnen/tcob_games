// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

struct monster {
    string  Symbol;
    color   Color {colors::White};
    point_i Position {};
};

}
