// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <algorithm>

#include "Die.hpp"
#include "Slot.hpp"

////////////////////////////////////////////////////////////

slot::slot(slot_face face)
    : _face {face}
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

auto slot::can_insert_die(die_face dieFace) const -> bool
{
    if (_die) { return false; }

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

void slot::insert_die(die* die)
{
    _die = die;
    if (_die) { _die->freeze(); }
}

auto slot::can_remove_die(die* die) const -> bool
{
    return _die && die == _die;
}

void slot::remove_die()
{
    if (_die) { _die->unfreeze(); }
    _die = nullptr;
}

auto slot::bounds() const -> rect_f const& { return _bounds; }
void slot::move_to(point_f pos) { _bounds = {pos, _bounds.Size}; }
auto slot::state() const -> slot_state { return _state; }

////////////////////////////////////////////////////////////

slots::slots(size_f scale)
    : _scale {scale}
{
}

void slots::lock() { _locked = true; }

void slots::unlock() { _locked = false; }

auto slots::add_slot(slot_face face) -> slot*
{
    auto& retValue {_slots.emplace_back(std::make_unique<slot>(face))};
    retValue->_bounds = {point_f::Zero, DICE_SIZE * _scale};
    return retValue.get();
}

auto slots::try_insert_die(die* hoverDie) -> slot*
{
    if (_locked || !hoverDie || !_hoverSlot || !_hoverSlot->can_insert_die(hoverDie->current_face())) { return nullptr; }

    _hoverSlot->insert_die(hoverDie);
    _hoverSlot->_state = slot_state::Idle;
    hoverDie->on_slotted(_hoverSlot->_bounds);
    return _hoverSlot;
}

auto slots::hover(rect_f const& rect) -> slot*
{
    if (_locked) { return nullptr; }

    auto const find {[&](rect_f const& rect) -> slot* {
        slot* bestSlot {nullptr};
        f32   maxArea {0.0f};

        for (auto& s : _slots) {
            auto const& slotRect {s->_bounds};
            auto const  interSect {rect.as_intersection_with(slotRect)};
            if (interSect.Size.area() > maxArea) {
                maxArea  = interSect.Size.area();
                bestSlot = s.get();
            }
        }

        return bestSlot;
    }};

    auto* slot {find(rect)};
    if (_hoverSlot != slot) {
        if (_hoverSlot) {
            _hoverSlot->_state = slot_state::Idle;
        }
        _hoverSlot = slot;
    }

    return _hoverSlot;
}

auto slots::try_remove_die(die* die) -> slot*
{
    if (_locked || !die || !_hoverSlot || !_hoverSlot->can_remove_die(die)) { return nullptr; }

    _hoverSlot->remove_die();
    _hoverSlot->_state = slot_state::Idle;
    return _hoverSlot;
}

void slots::reset(std::span<slot* const> slots)
{
    for (auto* slot : slots) {
        auto* die {slot->current_die()};
        if (!die) { continue; }
        slot->remove_die();
        die->move_by({0, slot->_bounds.height()}); // TODO: random die movement
    }
}

auto slots::get_hand(std::span<slot* const> slots) const -> hand
{
    if (!are_filled() || slots.size() > 5) { return {}; }

    struct indexed_face {
        die_face Face;
        slot*    Slot;
    };

    static auto const func {[](auto const& f) { return f.Face.Value; }};

    std::vector<indexed_face> faces;
    faces.reserve(slots.size());
    for (usize i {0}; i < slots.size(); ++i) {
        faces.push_back({.Face = slots[i]->current_die()->current_face(), .Slot = slots[i]});
    }
    std::ranges::stable_sort(faces, {}, func);

    hand result {};

    // value
    if ((faces.back().Face.Value - faces.front().Face.Value == faces.size() - 1)
        && (std::ranges::adjacent_find(faces, {}, func) == faces.end())) {
        result.Value = value_category::Straight;
        for (auto const& f : faces) { result.Slots.push_back(f.Slot); }
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
                if (f.Face.Value == targetValue) { result.Slots.push_back(f.Slot); }
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

void slots::on_hover(point_f mp)
{
    auto const getRect {[&] -> rect_f {
        return {mp, size_f::One};
    }};

    if (auto* hoverSlot {hover(getRect())}) {
        hoverSlot->_state = slot_state::Hover;
    }
}

void slots::on_drag(point_f mp, die* draggedDie)
{
    auto const getRect {[&] -> rect_f {
        rect_i const  bounds {draggedDie->bounds()};
        point_f const tl {bounds.top_left()};
        point_f const br {bounds.bottom_right()};
        return rect_f::FromLTRB(tl.X, tl.Y, br.X, br.Y);
    }};

    if (auto* hoverSlot {hover(getRect())}) {
        hoverSlot->_state = hoverSlot->can_insert_die(draggedDie->current_face()) ? slot_state::Accept : slot_state::Reject;
    }
}

auto slots::are_filled() const -> bool
{
    return std::ranges::all_of(_slots, [](auto const& slot) { return !slot->is_empty(); });
}

auto slots::count() const -> usize
{
    return _slots.size();
}
