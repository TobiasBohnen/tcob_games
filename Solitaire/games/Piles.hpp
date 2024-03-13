// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "Cards.hpp"
#include "Common.hpp" // IWYU pragma: keep

#include <deque>

namespace solitaire {

////////////////////////////////////////////////////////////

using build_func = std::function<bool(card const&, card const&, i32, bool)>;
using move_func  = std::function<bool(games::base_game const*, pile const*, isize)>;
using empty_func = std::function<bool(card const&, isize)>;

auto build_none(card const&, card const&, i32, bool) -> bool;
auto move_top(games::base_game const*, pile const* target, isize idx) -> bool;
auto empty_none(card const&, isize) -> bool;

struct rule {
    std::string BuildHint;
    build_func  Build {build_none};

    bool      IsPlayable {true};
    bool      IsSequence {false};
    move_func Move {move_top};

    empty_func Base {empty_none};

    i32  Interval {1};
    bool Wrap {false};
    i32  Limit {-1};
};

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
    void set_active(bool b, isize idx, color color);

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

    auto build(isize targetIndex, card const& drop, isize numCards) const -> bool;

    auto operator==(pile const& other) const -> bool
    {
        return this == &other;
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
