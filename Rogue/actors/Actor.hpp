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

    signal<>                  FinishedAction;
    signal<log_message const> Log;

    virtual auto position() const -> point_i = 0;
    virtual auto symbol() const -> string    = 0;
    virtual auto color() const -> color      = 0;

    auto strength_check(f32 prob) const -> bool;
    auto intelligence_check(f32 prob) const -> bool;
    auto vitality_check(f32 prob) const -> bool;
    auto agility_check(f32 prob) const -> bool;
    auto dexterity_check(f32 prob) const -> bool;

protected:
    actor(master_control& parent, profile profile);

    auto check(i32 attr, f32 prob) const -> bool;

    auto get_profile() const -> profile const&;
    auto get_profile() -> profile&;

    auto hp_max() const -> i32;
    auto mp_max() const -> i32;

    auto parent() -> master_control&;

private:
    master_control& _parent;
    profile         _profile;
};

}