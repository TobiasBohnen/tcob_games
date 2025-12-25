// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Common.hpp" // IWYU pragma: keep

////////////////////////////////////////////////////////////

enum class value_category : u8 {
    None,
    OnePair,
    TwoPair,
    ThreeOfAKind,
    FullHouse,
    Straight,
    FourOfAKind,
    FiveOfAKind,
};

enum class color_category : u8 {
    None,
    Flush,
    Rainbow,
};

////////////////////////////////////////////////////////////

struct hand {
    value_category Value {value_category::None};
    color_category Color {color_category::None};

    std::vector<slot*> Slots;

    static auto constexpr Members()
    {
        return std::tuple {
            member<&hand::Value> {"value"},
            member<&hand::Color> {"color"},
            member<&hand::Slots> {"slots"}};
    }
};

////////////////////////////////////////////////////////////

enum class slot_state : u8 {
    Idle   = 0,
    Hover  = 1,
    Accept = 2,
    Reject = 3
};

enum class op : u8 {
    Equal    = 0,
    NotEqual = 1,
    Greater  = 2,
    Less     = 3
};

struct slot_face {
    std::optional<u8> Value;
    std::optional<u8> Color;
    std::optional<op> Op;

    auto operator==(slot_face const& other) const -> bool = default;

    static auto constexpr Members()
    {
        return std::tuple {member<&slot_face::Value, std::nullopt> {"value"},
                           member<&slot_face::Color, std::nullopt> {"color"},
                           member<&slot_face::Op, std::nullopt> {"op"}};
    }
};

////////////////////////////////////////////////////////////

class slot {
    friend class slots;

public:
    explicit slot(slot_face face);

    scripting::table Owner;

    auto is_empty() const -> bool;
    auto current_die() const -> die*;

    auto can_insert_die(die_face dieFace) const -> bool;
    void insert_die(die* die);
    auto can_remove_die(die* die) const -> bool;
    void remove_die();

    auto bounds() const -> rect_f const&;
    void move_to(point_f pos);

    auto state() const -> slot_state;

private:
    rect_f _bounds {};
    die*   _die {nullptr};

    slot_face _face;

    slot_state _state {slot_state::Idle};
};

////////////////////////////////////////////////////////////

class slots {
public:
    explicit slots(size_f scale);

    void lock();
    void unlock();

    auto add_slot(slot_face face) -> slot*;
    void remove_slot(slot* slot);
    auto count() const -> usize;

    auto try_insert_die(die* die) -> slot*;
    auto try_remove_die(die* die) -> slot*;

    void reset();

    void on_hover(point_f mp);
    void on_drag(die* draggedDie);

private:
    void hover(rect_f const& rect);
    auto are_filled() const -> bool;

    std::vector<std::unique_ptr<slot>> _slots {};

    size_f _scale;

    bool _locked {false};

    slot* _hoverSlot {nullptr};
};

auto get_hand(std::span<slot* const> slots) -> hand;