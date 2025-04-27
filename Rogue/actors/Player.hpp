// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp"
#include "Actor.hpp"
#include "Profile.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

class player : public actor {
public:
    explicit player(master_control& parent);

    signal<> EndTurn;
    signal<> FinishedPath;

    point_i Position {};

    void update(milliseconds deltaTime);
    auto busy() const -> bool;

    auto symbol() const -> string;
    auto color() const -> tcob::color;
    auto light_color() const -> tcob::color;

    auto current_level() const -> i32;
    auto current_profile() -> profile;

    auto hp_max() const -> i32;
    auto mp_max() const -> i32;
    auto count_gold() const -> i32;

    auto try_pickup(std::shared_ptr<item> const& item) -> bool;

    void start_path(std::vector<point_i> const& path);
    auto try_move(point_i pos) -> bool;

private:
    profile _profile;

    milliseconds         _animationTimer {};
    std::vector<point_i> _path;

    master_control& _parent;
};

}