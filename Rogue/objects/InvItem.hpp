// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

enum class inv_type {
    Potion,
    Weapon,
    Armor
};

class inv_item {
public:
    virtual ~inv_item() = default;

    auto virtual type() const -> inv_type = 0;
    auto virtual name() const -> string   = 0;
};

////////////////////////////////////////////////////////////

class potion : public inv_item {
public:
    auto type() const -> inv_type final
    {
        return inv_type::Potion;
    }

    auto name() const -> string override
    {
        return "potion";
    }
};

////////////////////////////////////////////////////////////

class weapon : public inv_item {
public:
    auto type() const -> inv_type final
    {
        return inv_type::Weapon;
    }

    auto name() const -> string override
    {
        return "weapon";
    }
};

////////////////////////////////////////////////////////////

class armor : public inv_item {
public:
    auto type() const -> inv_type final
    {
        return inv_type::Armor;
    }

    auto name() const -> string override
    {
        return "armor";
    }
};
}
