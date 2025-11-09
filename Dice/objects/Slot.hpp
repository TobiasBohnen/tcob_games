// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp" // IWYU pragma: keep

#include "../Hand.hpp"
#include "../Painter.hpp"

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

    auto empty() const -> bool;
    auto current_die() const -> die*;

    void lock();
    void unlock();

    auto can_drop(die_face dieFace) const -> bool;
    void drop(die* die);
    auto can_take(die* die) const -> bool;
    void take();

    void update(milliseconds deltaTime) const;

    auto bounds() const -> rect_f const&;

private:
    die*             _die {nullptr};
    gfx::rect_shape* _shape {nullptr};

    bool _locked {false};

    slot_face  _face;
    slot_state _colorState {slot_state::Normal};
};

////////////////////////////////////////////////////////////

class slots {
public:
    slots(gfx::shape_batch& batch, asset_ptr<gfx::font_family> font);

    void lock();
    void unlock();
    auto are_locked() const -> bool;

    void add_slot(point_f pos, slot_face face);
    auto get_slot(usize idx) -> slot*;

    auto hover_slot(rect_f const& rect, die* die, bool isButtonDown) -> slot*;

    void take_die(die* die);

    auto get_hand() const -> hand;
    auto get_sum() const -> i32;
    auto is_complete() const -> bool;

    void update(milliseconds deltaTime);

private:
    std::vector<slot> _slots {};
    slot*             _hoverSlot {nullptr};

    gfx::shape_batch& _batch;
    slot_painter      _painter;

    bool _locked {false};
};