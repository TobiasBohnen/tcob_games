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
};

class slots {
public:
    slots(gfx::shape_batch& batch, asset_ptr<gfx::material> const& material);

    void reset();

    void hover(rect_f const& rect, color color);

    void drop_die(die* die);
    void take_die(die* die);

    auto check() -> hand;

private:
    auto get_faces() const -> std::optional<std::array<die_face, 5>>;

    std::array<slot, 5> _slots {};
    slot*               _hoverSlot {nullptr};
};