// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <tcob/tcob.hpp>

using namespace tcob;
using namespace std::chrono_literals;

class slot;
class slots;
class die;
class dice;
class base_game;

constexpr size_f DICE_SIZE {62, 62};
constexpr f32    DICE_OFFSET {72.f};

////////////////////////////////////////////////////////////

struct die_face {
    u8    Value {0};
    color Color {};

    auto texture_region() const -> string
    {
        return std::format("d{}-{}", Value, Color.value());
    }

    auto operator==(die_face const& other) const -> bool = default;
};

template <>
struct std::hash<die_face> {
    auto operator()(die_face const& p) const -> std::size_t
    {
        std::size_t h1 = std::hash<u8> {}(p.Value);
        std::size_t h2 = std::hash<u32> {}(p.Color.value());
        return tcob::helper::hash_combine(h1, h2);
    }
};

enum class op : u8 {
    Equal    = 0,
    NotEqual = 1,
    Greater  = 2,
    Less     = 3
};

struct slot_face {
    u8    Value {0};
    color Color {colors::Transparent};
    op    Op {op::Equal};

    auto texture_region() const -> string
    {
        string op;
        switch (Op) {
        case op::Equal:    op = "="; break;
        case op::NotEqual: op = "!"; break;
        case op::Greater:  op = "+"; break;
        case op::Less:     op = "-"; break;
        }

        return std::format("d{}-{}-{}", Value, Color.value(), op);
    }

    auto operator==(slot_face const& other) const -> bool = default;
};