// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

class object {
public:
    virtual ~object() = default;

    point_i Position {};

    auto virtual symbol() const -> string     = 0;
    auto virtual colors() const -> color_pair = 0;

    auto virtual is_blocking() const -> bool = 0;

    auto virtual can_interact(player& player) const -> bool = 0;
    auto virtual interact(player& player) -> string         = 0;

    auto virtual can_pickup(player& player) const -> bool = 0;
    auto virtual pickup(player& player) -> string         = 0;
};

class feature : public object {
public:
    auto can_pickup(player& player) const -> bool override { return false; }
    auto pickup(player& player) -> string override { return ""; }
};

class item : public object {
public:
    auto is_blocking() const -> bool override { return false; }
    auto can_interact(player& player) const -> bool override { return false; }
    auto interact(player& player) -> string override { return ""; }
};

////////////////////////////////////////////////////////////

class door : public feature {
public:
    auto symbol() const -> string override;
    auto colors() const -> color_pair override;

    auto is_blocking() const -> bool override;

    auto can_interact(player& player) const -> bool override;
    auto interact(player& player) -> string override;

private:
    bool _open {false};
};

////////////////////////////////////////////////////////////

class gold : public item {
public:
    explicit gold(i32 amount);

    auto symbol() const -> string override;
    auto colors() const -> color_pair override;

    auto can_pickup(player& player) const -> bool override;
    auto pickup(player& player) -> string override;

private:
    i32 _amount;
};

}
