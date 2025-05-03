// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include "../Common.hpp"
#include "Profile.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

class actor {
public:
    virtual ~actor() = default;

    auto virtual position() const -> point_i = 0;
    auto virtual symbol() const -> string    = 0;
    auto virtual color() const -> color      = 0;

protected:
    auto current_profile() -> profile&;
    auto current_profile() const -> profile const&;
    auto level() const -> i32;
    auto hp_max() const -> i32;
    auto mp_max() const -> i32;

private:
    profile _profile;
};

}