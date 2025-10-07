// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <algorithm>

#include "Die.hpp"
#include "Slot.hpp"

slots::slots(gfx::shape_batch& batch, asset_ptr<gfx::material> const& material)
{
    for (i32 i {0}; i < 5; ++i) {
        auto& slot {_slots[i]};
        slot.Shape                = &batch.create_shape<gfx::rect_shape>();
        slot.Shape->Bounds        = {{i * DICE_OFFSET, 10}, DICE_SIZE};
        slot.Shape->Material      = material;
        slot.Shape->TextureRegion = "empty";
    }
}

void slots::reset()
{
    for (auto& slot : _slots) {
        if (_hoverSlot == &slot) { continue; }
        slot.Shape->Color = colors::White;
    }
}

void slots::hover(rect_f const& rect, color color)
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

    auto* slot {find(rect)};
    if (_hoverSlot == slot) { return; }

    _hoverSlot = slot;
    if (!_hoverSlot) { return; }

    _hoverSlot->Shape->Color = color;
}

void slots::drop_die(die* die)
{
    if (!die || !_hoverSlot) { return; }

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
    auto sf {get_faces()};
    if (!sf) { return {}; }
    std::array<die_face, 5> faces {*sf};
    std::ranges::stable_sort(faces, {}, &die_face::Value);

    // value
    value_category valueCat {value_category::None};

    if (faces[0].Value == faces[4].Value) {                                            // FiveOfAKind
        valueCat = value_category::FiveOfAKind;
    } else if (faces[0].Value == faces[3].Value || faces[1].Value == faces[4].Value) { // FourOfAKind
        valueCat = value_category::FourOfAKind;
    } else {
        if ((faces[4].Value - faces[0].Value == 4)                                     // Straight
            && (std::ranges::adjacent_find(faces, {}, &die_face::Value) == faces.end())) {
            valueCat = value_category::Straight;
        } else {
            std::array<i8, 6> counts {};
            for (auto const& die : faces) { ++counts[die.Value - 1]; }

            i8 countPairs {0};
            i8 countTriples {0};
            for (i8 count : counts) {
                if (count == 2) {
                    ++countPairs;
                } else if (count == 3) {
                    ++countTriples;
                }
            }
            if (countPairs == 1 && countTriples == 1) { // FullHouse,
                valueCat = value_category::FullHouse;
            } else if (countTriples == 1) {             // ThreeOfAKind
                valueCat = value_category::ThreeOfAKind;
            } else if (countPairs == 2) {               // TwoPair
                valueCat = value_category::TwoPair;
            } else if (countPairs == 1) {               // OnePair
                valueCat = value_category::OnePair;
            }
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

auto slots::get_faces() const -> std::optional<std::array<die_face, 5>>
{
    std::array<die_face, 5> retValue;
    for (usize i {0}; i < 5; ++i) {
        auto* die {_slots[i].Die};
        if (!die) { return std::nullopt; }
        retValue[i] = die->Face;
    }
    return retValue;
}
