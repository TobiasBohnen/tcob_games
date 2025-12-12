// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "Hand.hpp"
#include "Painter.hpp"

////////////////////////////////////////////////////////////

enum class slot_state : u8 {
    Idle   = 0,
    Hover  = 1,
    Accept = 2,
    Reject = 3
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

    auto operator==(slot_face const& other) const -> bool = default;
};

////////////////////////////////////////////////////////////

class slot {
    friend class slots;

public:
    explicit slot(slot_face face);

    scripting::table Owner;

    auto is_empty() const -> bool;
    auto current_die() const -> die*;

    auto can_insert_die(die_face dieFace) const -> bool;
    void insert_die(die* die);
    auto can_remove_die(die* die) const -> bool;
    void remove_die();

    auto bounds() const -> rect_f const&;
    void move_to(point_f pos);

    auto state() const -> slot_state;

private:
    rect_f _bounds {};
    die*   _die {nullptr};

    slot_face _face;

    slot_state _state {slot_state::Idle};
};

////////////////////////////////////////////////////////////

class slots {
public:
    explicit slots(size_f scale);

    void lock();
    void unlock();

    auto add_slot(slot_face face) -> slot*;
    auto count() const -> usize;

    auto try_insert_die(die* die) -> slot*;
    auto try_remove_die(die* die) -> slot*;

    void reset(std::span<slot* const> slots);
    auto get_hand(std::span<slot* const> slots) const -> hand;

    void on_hover(point_f mp);
    void on_drag(point_f mp, die* draggedDie);

private:
    auto hover(rect_f const& rect) -> slot*;
    auto are_filled() const -> bool;

    std::vector<std::unique_ptr<slot>> _slots {};

    size_f _scale;

    bool _locked {false};

    slot* _hoverSlot {nullptr};
};