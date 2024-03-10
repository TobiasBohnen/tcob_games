// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Cards.hpp"
#include "Common.hpp" // IWYU pragma: keep
#include "Rules.hpp"

#include <deque>

namespace solitaire {
////////////////////////////////////////////////////////////

enum class pile_type {
    Stock,
    Waste,
    Foundation,
    Tableau,
    Reserve,
    FreeCell
};

auto get_pile_type_name(pile_type s) -> std::string;

enum class layout_type {
    Squared,
    Column,
    Row,
    Fan
};

////////////////////////////////////////////////////////////

namespace initial {
    auto top_face_up(usize size) -> std::vector<bool>;
    auto face_up(usize size) -> std::vector<bool>;
    auto face_down(usize size) -> std::vector<bool>;
};

////////////////////////////////////////////////////////////

class pile {
public:
    std::shared_ptr<gfx::sprite> Marker;
    std::deque<card>             Cards;

    pile_type         Type {};
    rule              Rule {};
    layout_type       Layout {layout_type::Squared};
    std::vector<bool> Initial; // true -> faceup

    point_f Position {};

    bool HasMarker {true};

    auto is_active() const -> bool;
    void set_active(bool b, isize idx);

    auto is_playable() const -> bool;
    auto empty() const -> bool;

    void remove_tint();
    void tint_cards(color color, isize idx);

    void flip_up_cards();
    void flip_up_top_card();
    void flip_down_cards();
    void flip_down_top_card();

    auto get_description(i32 remainingRedeals) const -> hover_info;
    auto get_marker_texture_name() const -> std::string;

    void move_cards(pile& to, isize srcOffset, isize numCards, bool reverse);

    auto operator==(pile const& other) const -> bool
    {
        return this == &other;
    }
    auto operator<=>(pile const& other) const
    {
        return this <=> &other;
    }

private:
    bool _isActive {false};
};

////////////////////////////////////////////////////////////

struct stock : public pile {
    stock();
};

////////////////////////////////////////////////////////////

struct waste : public pile {
    waste();
};

////////////////////////////////////////////////////////////

struct tableau : public pile {
    tableau();
};

////////////////////////////////////////////////////////////

struct foundation : public pile {
    foundation();
};

////////////////////////////////////////////////////////////

struct reserve : public pile {
    reserve();
};

////////////////////////////////////////////////////////////

struct freecell : public pile {
    freecell();
};

////////////////////////////////////////////////////////////

}
