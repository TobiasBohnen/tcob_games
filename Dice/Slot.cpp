// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <algorithm>

#include "Die.hpp"
#include "Slot.hpp"

slots::slots(gfx::shape_batch& batch)
    : _batch {batch}
{
}

void slots::add_slot(std::span<die_face const> faces, asset_ptr<gfx::material> const& material)
{
    auto& slot {_slots.emplace_back()};
    slot.Shape                = &_batch.create_shape<gfx::rect_shape>();
    slot.Shape->Bounds        = {{_slots.size() * DICE_OFFSET, 10}, DICE_SIZE};
    slot.Shape->Material      = material;
    slot.Shape->TextureRegion = "empty";
    slot.AcceptedFaces        = {faces.begin(), faces.end()};
}

void slots::reset()
{
    for (auto& slot : _slots) {
        if (_hoverSlot == &slot) { continue; }
        slot.Shape->Color = colors::White;
    }
}

auto slots::hover_slot(rect_f const& rect) -> slot*
{
    auto const find {[&](rect_f const& rect) -> slot* {
        slot* bestSlot {nullptr};
        f32   maxArea {0.0f};

        for (auto& s : _slots) {
            if (s.Die) { continue; }

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
    if (!die || !_hoverSlot || !_hoverSlot->AcceptedFaces.contains(die->Face)) { return; }

    if (!_hoverSlot->Die) {
        _hoverSlot->Die = die;
        die->lock();
        die->Shape->Bounds = *_hoverSlot->Shape->Bounds;
    }
}

void slots::take_die(die* die)
{
    if (!die) { return; }

    for (auto& slot : _slots) {
        if (slot.Die == die) {
            die->unlock();
            slot.Die = nullptr;
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
        auto* die {_slots[i].Die};
        if (!die) { return {}; }
        faces[i] = die->Face;
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
