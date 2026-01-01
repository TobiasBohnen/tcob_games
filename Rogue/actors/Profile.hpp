// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

constexpr i32 MAGIC_MIN {0};
constexpr i32 MAGIC_MAX {99};

struct magic {
    i32 Earth {MAGIC_MIN};
    i32 Wind {MAGIC_MIN};
    i32 Fire {MAGIC_MIN};
    i32 Water {MAGIC_MIN};
    i32 Life {MAGIC_MIN};
    i32 Energy {MAGIC_MIN};
};

constexpr i32 ATTRI_MIN {1};
constexpr i32 ATTRI_MAX {10};

struct attributes {
    i32 Strength {ATTRI_MIN};
    i32 Intelligence {ATTRI_MIN};
    i32 Vitality {ATTRI_MIN};
    i32 Agility {ATTRI_MIN};
    i32 Dexterity {ATTRI_MIN};
};

////////////////////////////////////////////////////////////

struct profile {
    string Name {"Tim"};

    i32 HP {0};
    i32 MP {0};

    inventory Inventory;

    f32 VisualRange {7};

    magic      Magic;
    attributes Attributes;
};

}
