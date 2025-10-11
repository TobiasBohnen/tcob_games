// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

#include "Hand.hpp"

class slot {
public:
    gfx::rect_shape* Shape {nullptr};
    die*             Die {nullptr};

    std::unordered_set<die_face> AcceptedFaces;
};

////////////////////////////////////////////////////////////

class slots {
public:
    slots(gfx::shape_batch& batch);

    void add_slot(std::span<die_face const> faces, asset_ptr<gfx::material> const& material);

    void reset();

    auto hover_slot(rect_f const& rect) -> slot*;

    void drop_die(die* die);
    void take_die(die* die);

    auto check() -> hand;

private:
    std::vector<slot> _slots {};
    slot*             _hoverSlot {nullptr};

    gfx::shape_batch& _batch;
};