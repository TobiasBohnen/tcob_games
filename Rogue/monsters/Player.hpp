// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

class player {
public:
    ////////////////////////////////////////////////////////////

    player();

    point_i Position {};

    void update(milliseconds deltaTime);

    auto symbol() const -> string;
    auto color() const -> color;

    auto current_level() const -> i32;
    auto current_profile() const -> profile;

    auto hp_max() const -> i32;
    auto mp_max() const -> i32;

    auto inventory() const -> std::vector<std::shared_ptr<inv_item>> const&;
    auto can_add_item(inv_item const& item) const -> bool;
    void add_item(std::shared_ptr<inv_item> const& item);

    void add_gold(i32 amount);

    auto try_move(point_i pos, level const& level) -> bool;

private:
    profile                                _profile;
    std::vector<std::shared_ptr<inv_item>> _inventory;
};

}