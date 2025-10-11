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

    auto operator==(die_face const& other) const -> bool = default;
};

template <>
struct std::hash<die_face> {
    auto operator()(die_face const& p) const -> std::size_t
    {
        std::size_t h1 = std::hash<u8> {}(p.Value);
        std::size_t h2 = std::hash<color_type> {}(p.Color);
        return tcob::helper::hash_combine(h1, h2);
    }
};