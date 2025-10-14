// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "Hand.hpp"

////////////////////////////////////////////////////////////

class slot {
public:
    slot(std::span<u8 const> values, std::span<color_type const> colors);

    gfx::rect_shape* Shape {nullptr};

    auto current_die() const -> die*;

    auto can_drop(die_face face) const -> bool;
    void drop(die* die);
    void take();

private:
    die* _die {nullptr};

    std::unordered_set<u8>         _allowedValues;
    std::unordered_set<color_type> _allowedColors;
};

////////////////////////////////////////////////////////////

class slots {
public:
    slots(gfx::shape_batch& batch, asset_ptr<gfx::material> const& material);

    void add_slot(point_f pos, std::span<u8 const> values, std::span<color_type const> colors);

    auto hover_slot(rect_f const& rect) -> slot*;

    void drop_die(die* die);
    void take_die(die* die);

    auto get_hand() const -> hand;
    auto get_sum() const -> i32;
    auto is_complete() const -> bool;

    void reset_shapes();

private:
    std::vector<slot> _slots {};
    slot*             _hoverSlot {nullptr};

    gfx::shape_batch&        _batch;
    asset_ptr<gfx::material> _material;
};