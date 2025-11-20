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
    Normal,
    Hovered,
    Accept,
    Reject
};

class slot {
    friend class slots;

public:
    slot(gfx::rect_shape* shape, slot_face face);

    gfx::rect_shape* Shape {nullptr};
    scripting::table Owner;

    auto is_empty() const -> bool;
    auto current_die() const -> die*;

    auto can_insert(die_face dieFace) const -> bool;
    void insert(die* die);
    auto can_remove(die* die) const -> bool;
    void remove();

    void update(milliseconds deltaTime) const;

private:
    die* _die {nullptr};

    bool _locked {false};

    slot_face  _face;
    slot_state _colorState {slot_state::Normal};
};

////////////////////////////////////////////////////////////

class slots {
public:
    slots(gfx::shape_batch& batch, asset_ptr<gfx::font_family> font);

    slot* HoverSlot {nullptr};

    void lock();
    void unlock();

    auto add_slot(point_f pos, slot_face face) -> slot*;

    void hover_slot(rect_f const& rect, die* die, bool isButtonDown);

    auto try_insert(die* die) -> bool;
    auto try_remove(die* die) -> slot*;

    void reset(std::span<slot* const> slots);

    auto get_hand() const -> hand;
    auto get_sum() const -> i32;
    auto are_filled() const -> bool;

    void update(milliseconds deltaTime);

private:
    std::vector<std::unique_ptr<slot>> _slots {};

    gfx::shape_batch& _batch;
    slot_painter      _painter;

    bool _locked {false};
};