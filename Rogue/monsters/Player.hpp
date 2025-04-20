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

    struct profile {
        string Name {"Tim"};

        i32 HP {75};
        i32 MP {25};
        i32 XP {85};
        i32 Gold {0};

        f32 VisualRange {7};

        struct magic {
            i32 Earth {0};
            i32 Wind {0};
            i32 Fire {0};
            i32 Water {0};
            i32 Life {0};
            i32 Light {0};
        } Magic;

        struct attributes {
            i32 Strength {0};
            i32 Intelligence {0};
            i32 Vitality {0};
            i32 Agility {0};
            i32 Dexterity {0};
        } Attributes;

        auto level() const -> i32;
        auto xp_required_for(i32 level) const -> i32;
        auto hp_max() const -> i32;
        auto mp_max() const -> i32;
    };

    ////////////////////////////////////////////////////////////

    player();

    point_i Position {};

    void update(milliseconds deltaTime);

    auto stats() const -> profile const&;

    auto symbol() const -> string;
    auto color() const -> color;

    auto try_move(point_i pos, level const& level) -> bool;

    auto can_add_item(inv_item const& item) const -> bool;
    void add_item(std::shared_ptr<inv_item> const& item);

    void add_gold(i32 amount);

private:
    profile                                _stats;
    std::vector<std::shared_ptr<inv_item>> _inventory;
};

}