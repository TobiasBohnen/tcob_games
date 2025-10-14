// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <algorithm>

#include "Die.hpp"
#include "Slot.hpp"

////////////////////////////////////////////////////////////

slot::slot(gfx::rect_shape* shape, std::span<u8 const> values, std::span<color_type const> colors)
    : _shape {shape}
    , _allowedValues {values.begin(), values.end()}
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

void slot::update(milliseconds deltaTime) const
{
    switch (State) {
    case slot_state::Normal:
        _shape->Color = colors::White;
        break;
    case slot_state::Hovered:
        _shape->Color = colors::Gray;
        break;
    case slot_state::Accept:
        _shape->Color = colors::Green;
        break;
    case slot_state::Reject:
        _shape->Color = colors::Red;
        break;
    case slot_state::PartOfHand:
        _shape->Color = colors::PaleVioletRed;
        break;
    }
}

auto slot::bounds() const -> rect_f const&
{
    return *_shape->Bounds;
}

////////////////////////////////////////////////////////////

slots::slots(gfx::shape_batch& batch, asset_ptr<gfx::material> const& material)
    : _batch {batch}
    , _material {material}
{
}

void slots::add_slot(point_f pos, std::span<u8 const> values, std::span<color_type const> colors)
{
    auto* shape {&_batch.create_shape<gfx::rect_shape>()};
    shape->Bounds        = {pos, DICE_SIZE};
    shape->Material      = _material;
    shape->TextureRegion = "empty";
    _slots.emplace_back(shape, values, colors);
}

auto slots::get_slot(usize idx) -> slot*
{
    return &_slots[idx];
}

auto slots::hover_slot(rect_f const& rect) -> slot*
{
    auto const find {[&](rect_f const& rect) -> slot* {
        slot* bestSlot {nullptr};
        f32   maxArea {0.0f};

        for (auto& s : _slots) {
            if (s.current_die()) { continue; }

            auto const& slotRect {s.bounds()};
            auto const  interSect {rect.as_intersection_with(slotRect)};
            if (interSect.Size.area() > maxArea) {
                maxArea  = interSect.Size.area();
                bestSlot = &s;
            }
        }

        return bestSlot;
    }};

    auto* slot {find(rect)};

    if (_hoverSlot && slot != _hoverSlot) {
        _hoverSlot->State = slot_state::Normal;
    }

    _hoverSlot = slot;
    return _hoverSlot;
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

auto slots::get_hand() const -> hand
{
    if (!is_complete() || _slots.size() > 5) { return {}; }

    struct indexed_face {
        die_face Face;
        usize    Index;
    };

    static auto const func {[](auto const& f) { return f.Face.Value; }};

    std::vector<indexed_face> faces;
    faces.reserve(_slots.size());
    for (usize i {0}; i < _slots.size(); ++i) {
        faces.push_back({.Face = _slots[i].current_die()->current_face(), .Index = i});
    }
    std::ranges::stable_sort(faces, {}, func);

    hand result {};

    // value
    if ((faces.back().Face.Value - faces.front().Face.Value == faces.size() - 1)
        && (std::ranges::adjacent_find(faces, {}, func) == faces.end())) {
        result.Value = value_category::Straight;
        for (auto const& f : faces) { result.Slots.push_back(f.Index); }
    } else {
        std::array<i8, 6> counts {};
        std::array<i8, 4> groups {};
        for (auto const& f : faces) {
            i8 const count {++counts[f.Face.Value - 1]};
            if (count >= 2 && count <= 5) {
                ++groups[count - 2];
                if (count > 2) { --groups[count - 3]; }
            }
        }

        auto collect {[&](u8 targetValue) {
            for (auto const& f : faces) {
                if (f.Face.Value == targetValue) { result.Slots.push_back(f.Index); }
            }
        }};
        auto collectAll {[&](u8 targetCount) {
            for (usize v {0}; v < 6; ++v) {
                if (counts[v] == targetCount) { collect(v + 1); }
            }
        }};

        if (groups[3] == 1) {
            result.Value = value_category::FiveOfAKind; // FiveOfAKind
            collect(faces[0].Face.Value);
        } else if (groups[2] == 1) {
            result.Value = value_category::FourOfAKind; // FourOfAKind
            collectAll(4);
        } else if (groups[0] == 1 && groups[1] == 1) {
            result.Value = value_category::FullHouse;   // FullHouse
            collect(faces.front().Face.Value);
            collect(faces.back().Face.Value);
        } else if (groups[1] == 1) {
            result.Value = value_category::ThreeOfAKind; // ThreeOfAKind
            collectAll(3);
        } else if (groups[0] == 2) {
            result.Value = value_category::TwoPair;      // TwoPair
            collectAll(2);
        } else if (groups[0] == 1) {
            result.Value = value_category::OnePair;      // OnePair
            collectAll(2);
        }
    }

    // color
    std::unordered_set<color_type> uniqueColors;
    for (auto const& f : faces) { uniqueColors.insert(f.Face.Color); }

    switch (uniqueColors.size()) {
    case 1:
        result.Color = color_category::Flush;
        break;
    case 5:
        result.Color = color_category::Rainbow;
        break;
    default: break;
    }

    return result;
}

auto slots::get_sum() const -> i32
{
    if (!is_complete()) { return -1; }

    i32 retValue {0};
    for (auto const& slot : _slots) {
        retValue += slot.current_die()->current_face().Value;
    }
    return retValue;
}

auto slots::is_complete() const -> bool
{
    return std::ranges::all_of(_slots, [](auto const& slot) { return slot.current_die(); });
}

void slots::update(milliseconds deltaTime)
{
    for (auto& slot : _slots) {
        slot.update(deltaTime);
    }
}
