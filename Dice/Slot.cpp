// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <algorithm>

#include "Die.hpp"
#include "Slot.hpp"

////////////////////////////////////////////////////////////

slot::slot(std::span<u8 const> values, std::span<color_type const> colors)
    : _allowedValues {values.begin(), values.end()}
    , _allowedColors {colors.begin(), colors.end()}
{
}

auto slot::current_die() const -> die*
{
    return _die;
}

auto slot::can_drop(die_face face) const -> bool
{
    return _allowedValues.contains(face.Value) && _allowedColors.contains(face.Color);
}

void slot::drop(die* die)
{
    _die = die;
    if (_die) { _die->lock(); }
}

void slot::take()
{
    if (_die) { _die->unlock(); }
    _die = nullptr;
}

////////////////////////////////////////////////////////////

slots::slots(gfx::shape_batch& batch, asset_ptr<gfx::material> const& material)
    : _batch {batch}
    , _material {material}
{
}

void slots::add_slot(point_f pos, std::span<u8 const> values, std::span<color_type const> colors)
{
    auto& slot {_slots.emplace_back(values, colors)};
    slot.Shape                = &_batch.create_shape<gfx::rect_shape>();
    slot.Shape->Bounds        = {pos, DICE_SIZE};
    slot.Shape->Material      = _material;
    slot.Shape->TextureRegion = "empty";
}

auto slots::hover_slot(rect_f const& rect) -> slot*
{
    auto const find {[&](rect_f const& rect) -> slot* {
        slot* bestSlot {nullptr};
        f32   maxArea {0.0f};

        for (auto& s : _slots) {
            if (s.current_die()) { continue; }

            auto const& slotRect {*s.Shape->Bounds};
            auto const  interSect {rect.as_intersection_with(slotRect)};
            if (interSect.Size.area() > maxArea) {
                maxArea  = interSect.Size.area();
                bestSlot = &s;
            }
        }

        return bestSlot;
    }};

    _hoverSlot = find(rect);
    return _hoverSlot;
}

void slots::drop_die(die* die)
{
    if (!die || !_hoverSlot || !_hoverSlot->can_drop(die->current_face())) { return; }

    if (!_hoverSlot->current_die()) {
        _hoverSlot->drop(die);
        die->Shape->Bounds = *_hoverSlot->Shape->Bounds;
    }
}

void slots::take_die(die* die)
{
    if (!die) { return; }

    for (auto& slot : _slots) {
        if (slot.current_die() == die) {
            slot.take();
            break;
        }
    }
}

auto slots::check() -> hand
{
    assert(_slots.size() <= 5);

    std::vector<die_face> faces;
    faces.resize(_slots.size());
    for (usize i {0}; i < _slots.size(); ++i) {
        auto* die {_slots[i].current_die()};
        if (!die) { return {}; }
        faces[i] = die->current_face();
    }
    std::ranges::stable_sort(faces, {}, &die_face::Value);

    // value
    value_category valueCat {value_category::None};

    if ((faces.back().Value - faces.front().Value == 4) // Straight
        && (std::ranges::adjacent_find(faces, {}, &die_face::Value) == faces.end())) {
        valueCat = value_category::Straight;
    } else {
        std::array<i8, 6> counts {};
        std::array<i8, 4> groups {};
        for (auto const& die : faces) {
            i8 const count {++counts[die.Value - 1]};
            if (count >= 2 && count <= 5) {
                ++groups[count - 2];
                if (count > 2) { --groups[count - 3]; }
            }
        }
        if (groups[3] == 1) {                          // FiveOfAKind
            valueCat = value_category::FiveOfAKind;
        } else if (groups[2] == 1) {                   // FourOfAKind
            valueCat = value_category::FourOfAKind;
        } else if (groups[0] == 1 && groups[1] == 1) { // FullHouse
            valueCat = value_category::FullHouse;
        } else if (groups[1] == 1) {                   // ThreeOfAKind
            valueCat = value_category::ThreeOfAKind;
        } else if (groups[0] == 2) {                   // TwoPair
            valueCat = value_category::TwoPair;
        } else if (groups[0] == 1) {                   // OnePair
            valueCat = value_category::OnePair;
        }
    }

    // color
    color_category colorCat {color_category::None};

    std::unordered_set<color_type> uniqueColors;
    for (auto const& die : faces) { uniqueColors.insert(die.Color); }
    switch (uniqueColors.size()) {
    case 1:  colorCat = color_category::Flush; break;   // Flush
    case 5:  colorCat = color_category::Rainbow; break; // Rainbow
    default: break;
    }

    return {.Value = valueCat, .Color = colorCat};
}

void slots::reset_shapes()
{
    for (auto& slot : _slots) {
        if (_hoverSlot == &slot) { continue; }
        slot.Shape->Color = colors::White;
    }
}
