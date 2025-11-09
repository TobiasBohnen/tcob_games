// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <algorithm>
#include <utility>

#include "Die.hpp"
#include "Slot.hpp"

////////////////////////////////////////////////////////////

slot::slot(gfx::rect_shape* shape, slot_face face)
    : _shape {shape}
    , _face {face}
{
}

auto slot::empty() const -> bool
{
    return _die == nullptr;
}

auto slot::current_die() const -> die*
{
    return _die;
}

void slot::lock() { _locked = true; }

void slot::unlock() { _locked = false; }

auto slot::can_drop(die_face dieFace) const -> bool
{
    if (_die || _locked) { return false; }

    switch (_face.Op) {
    case op::Equal:
        if (dieFace.Value != _face.Value) { return false; }
        break;
    case op::NotEqual:
        if (dieFace.Value == _face.Value) { return false; }
        break;
    case op::Greater:
        if (dieFace.Value <= _face.Value) { return false; }
        break;
    case op::Less:
        if (dieFace.Value >= _face.Value) { return false; }
        break;
    }

    if (_face.Color == colors::Transparent) { return true; }
    return _face.Color == dieFace.Color;
}

void slot::drop(die* die)
{
    _colorState = slot_state::Normal;

    _die = die;
    if (_die) { _die->lock(); }
}

auto slot::can_take(die* die) const -> bool
{
    return !_locked && _die && die == _die;
}

void slot::take()
{
    if (_die) { _die->unlock(); }
    _die = nullptr;
}

void slot::update(milliseconds deltaTime) const
{
    // TODO: accept/reject texregion
    switch (_colorState) {
    case slot_state::Normal:  _shape->Color = colors::White; break;
    case slot_state::Hovered: _shape->Color = colors::Gray; break;
    case slot_state::Accept:  _shape->Color = colors::Gray; break;
    case slot_state::Reject:  _shape->Color = colors::White; break;
    }
}

auto slot::bounds() const -> rect_f const&
{
    return *_shape->Bounds;
}

////////////////////////////////////////////////////////////

slots::slots(gfx::shape_batch& batch, asset_ptr<gfx::font_family> font)
    : _batch {batch}
    , _painter {{10, 50}, std::move(font)}
{
}

void slots::lock() { _locked = true; }

void slots::unlock() { _locked = false; }

auto slots::are_locked() const -> bool { return _locked; }

void slots::add_slot(point_f pos, slot_face face)
{
    auto* shape {&_batch.create_shape<gfx::rect_shape>()};
    shape->Bounds        = {pos, DICE_SIZE};
    shape->Material      = _painter.material();
    shape->TextureRegion = face.texture_region();

    _slots.emplace_back(shape, face);
    _painter.make_slot(face);
}

auto slots::get_slot(usize idx) -> slot*
{
    return &_slots[idx];
}

auto slots::hover_slot(rect_f const& rect, die* die, bool isButtonDown) -> slot*
{
    if (_locked) { return nullptr; }

    auto const find {[&](rect_f const& rect) -> slot* {
        slot* bestSlot {nullptr};
        f32   maxArea {0.0f};

        for (auto& s : _slots) {
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
    if (slot) {
        if (!die) {
            slot->_colorState = slot_state::Hovered;
        } else if (isButtonDown) {
            slot->_colorState = slot->can_drop(die->current_face()) ? slot_state::Accept : slot_state::Reject;
        }
    }
    if (slot != _hoverSlot && _hoverSlot) { _hoverSlot->_colorState = slot_state::Normal; }

    _hoverSlot = slot;
    return _hoverSlot;
}

void slots::take_die(die* die)
{
    if (!die) { return; }

    for (auto& slot : _slots) {
        if (slot.can_take(die)) {
            slot.take();
            return;
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
    std::unordered_set<color> uniqueColors;
    for (auto const& f : faces) { uniqueColors.insert(f.Face.Color); }

    switch (uniqueColors.size()) {
    case 1:  result.Color = color_category::Flush; break;
    case 5:  result.Color = color_category::Rainbow; break;
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
    return std::ranges::all_of(_slots, [](auto const& slot) { return !slot.empty(); });
}

void slots::update(milliseconds deltaTime)
{
    for (auto& slot : _slots) {
        slot.update(deltaTime);
    }
}
