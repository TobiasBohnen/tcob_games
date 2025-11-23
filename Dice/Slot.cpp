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

auto slot::is_empty() const -> bool
{
    return _die == nullptr;
}

auto slot::current_die() const -> die*
{
    return _die;
}

auto slot::can_insert(die_face dieFace) const -> bool
{
    if (_die || _locked) { return false; }

    if (_face.Value != 0) {
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
    }

    if (_face.Color == colors::Transparent) { return true; }
    return _face.Color == dieFace.Color;
}

void slot::insert(die* die)
{
    _colorState = slot_state::Normal;

    _die = die;
    if (_die) { _die->freeze(); }
}

auto slot::can_remove(die* die) const -> bool
{
    return !_locked && _die && die == _die;
}

void slot::remove()
{
    if (_die) { _die->unfreeze(); }
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

////////////////////////////////////////////////////////////

slots::slots(gfx::shape_batch& batch, asset_ptr<gfx::font_family> font, size_f scale)
    : _batch {batch}
    , _painter {{10, 50}, std::move(font)}
    , _scale {scale}
{
}

void slots::lock() { _locked = true; }

void slots::unlock() { _locked = false; }

auto slots::add_slot(slot_face face, point_f pos) -> slot*
{
    auto* shape {&_batch.create_shape<gfx::rect_shape>()};
    shape->Bounds        = {pos, DICE_SIZE * _scale};
    shape->Material      = _painter.material();
    shape->TextureRegion = face.texture_region();

    auto& retValue {_slots.emplace_back(std::make_unique<slot>(shape, face))};
    _painter.make_slot(face);

    return retValue.get();
}

auto slots::try_insert(die* hoverDie) -> bool
{
    if (_locked || !hoverDie || !HoverSlot || !HoverSlot->can_insert(hoverDie->current_face())) { return false; }

    HoverSlot->insert(hoverDie);
    hoverDie->_colorState    = die_state::Hovered;
    hoverDie->_shape->Bounds = *HoverSlot->_shape->Bounds;
    _batch.send_to_back(*hoverDie->_shape);
    return true;
}

void slots::hover(rect_f const& rect, die* die, bool isButtonDown)
{
    if (_locked) { return; }

    auto const find {[&](rect_f const& rect) -> slot* {
        slot* bestSlot {nullptr};
        f32   maxArea {0.0f};

        for (auto& s : _slots) {
            auto const& slotRect {*s->_shape->Bounds};
            auto const  interSect {rect.as_intersection_with(slotRect)};
            if (interSect.Size.area() > maxArea) {
                maxArea  = interSect.Size.area();
                bestSlot = s.get();
            }
        }

        return bestSlot;
    }};

    auto* slot {find(rect)};
    if (slot) {
        if (!die) {
            slot->_colorState = slot_state::Hovered;
        } else if (isButtonDown) {
            slot->_colorState = slot->can_insert(die->current_face()) ? slot_state::Accept : slot_state::Reject;
        }
    }
    if (slot != HoverSlot && HoverSlot) { HoverSlot->_colorState = slot_state::Normal; }

    HoverSlot = slot;
}

auto slots::try_remove(die* die) -> slot*
{
    if (_locked || !die) { return nullptr; }

    for (auto& slot : _slots) {
        if (slot->can_remove(die)) {
            slot->remove();
            return slot.get();
        }
    }

    return nullptr;
}

void slots::reset(std::span<slot* const> slots)
{
    for (auto* slot : slots) {
        auto* die {slot->current_die()};
        if (!die) { continue; }
        slot->remove();
        die->move_by({0, (*slot->_shape->Bounds).height()});
    }
}

auto slots::get_hand() const -> hand
{
    if (!are_filled() || _slots.size() > 5) { return {}; }

    struct indexed_face {
        die_face Face;
        usize    Index;
    };

    static auto const func {[](auto const& f) { return f.Face.Value; }};

    std::vector<indexed_face> faces;
    faces.reserve(_slots.size());
    for (usize i {0}; i < _slots.size(); ++i) {
        faces.push_back({.Face = _slots[i]->current_die()->current_face(), .Index = i});
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
    if (!are_filled()) { return -1; }

    i32 retValue {0};
    for (auto const& slot : _slots) {
        retValue += slot->current_die()->current_face().Value;
    }
    return retValue;
}

auto slots::are_filled() const -> bool
{
    return std::ranges::all_of(_slots, [](auto const& slot) { return !slot->is_empty(); });
}

void slots::update(milliseconds deltaTime)
{
    for (auto& slot : _slots) {
        slot->update(deltaTime);
    }
}
auto slots::count() const -> usize
{
    return _slots.size();
}
