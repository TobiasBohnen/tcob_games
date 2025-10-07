// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <tcob/tcob.hpp>

using namespace tcob;
using namespace tcob::ui;
using namespace std::chrono_literals;

class slot;
class slots;
class die;
class dice;

constexpr size_f DICE_SIZE {64, 64};
constexpr f32    DICE_OFFSET {72.f};

////////////////////////////////////////////////////////////

enum class color_type : u8 {
    Red,
    Green,
    Yellow,
    Cyan,
    Blue
};

struct die_face {
    u8         Value {0};
    color_type Color {};
};