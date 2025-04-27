// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

struct magic {
    i32 Earth {1};
    i32 Wind {2};
    i32 Fire {3};
    i32 Water {4};
    i32 Life {5};
    i32 Energy {6};
};

struct attributes {
    i32 Strength {10};
    i32 Intelligence {20};
    i32 Vitality {5};
    i32 Agility {3};
    i32 Dexterity {9};
};

////////////////////////////////////////////////////////////

struct profile {
    string Name {"Tim"};

    i32 HP {25};
    i32 MP {25};

    i32 XP {85};

    inventory Inventory;

    f32 VisualRange {7};

    magic      Magic;
    attributes Attributes;

    auto static constexpr xp_required_for(i32 level) -> i32
    {
        return XP_SCALE * (level - 1) * level;
    }
};

}
