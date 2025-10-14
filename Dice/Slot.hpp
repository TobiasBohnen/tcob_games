// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "Hand.hpp"

////////////////////////////////////////////////////////////

enum class slot_state : u8 {
    Normal,
    Hovered,
    Accept,
    Reject,
    PartOfHand
};

class slot {
public:
    slot(gfx::rect_shape* shape, std::span<u8 const> values, std::span<color_type const> colors);

    slot_state State {slot_state::Normal};

    auto current_die() const -> die*;

    auto can_drop(die_face face) const -> bool;
    void drop(die* die);
    void take();

    void update(milliseconds deltaTime) const;

    auto bounds() const -> rect_f const&;

private:
    die*             _die {nullptr};
    gfx::rect_shape* _shape {nullptr};

    std::unordered_set<u8>         _allowedValues;
    std::unordered_set<color_type> _allowedColors;
};

////////////////////////////////////////////////////////////

class slots {
public:
    slots(gfx::shape_batch& batch, asset_ptr<gfx::material> const& material);

    void add_slot(point_f pos, std::span<u8 const> values, std::span<color_type const> colors);
    auto get_slot(usize idx) -> slot*;

    auto hover_slot(rect_f const& rect) -> slot*;

    void take_die(die* die);

    auto get_hand() const -> hand;
    auto get_sum() const -> i32;
    auto is_complete() const -> bool;

    void update(milliseconds deltaTime);

private:
    std::vector<slot> _slots {};
    slot*             _hoverSlot {nullptr};

    gfx::shape_batch&        _batch;
    asset_ptr<gfx::material> _material;
};