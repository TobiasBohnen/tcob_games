// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Actor.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

auto actor::current_profile() -> profile&
{
    // TODO: add inventory, effects, etc.
    return _profile;
}

auto actor::current_profile() const -> profile const&
{
    return _profile;
}

auto actor::symbol() const -> string
{
    return "@";
}

auto actor::color() const -> tcob::color
{
    f32 const ratio {static_cast<f32>(_profile.HP) / static_cast<f32>(hp_max())};
    return color::Lerp(colors::Red, colors::White, ratio);
}

auto actor::level() const -> i32
{
    return static_cast<i32>((1.0f + std::sqrt(1.0f + (4.0f * (static_cast<f32>(_profile.XP) / XP_SCALE)))) * 0.5f);
}

auto actor::hp_max() const -> i32
{
    // Base 100 HP + 10 * Vitality per level
    return 100 + ((level() - 1) * VIT_SCALE * _profile.Attributes.Vitality);
}

auto actor::mp_max() const -> i32
{
    // Base 50 MP + 15 * Intelligence per level
    return 50 + ((level() - 1) * INT_SCALE * _profile.Attributes.Intelligence);
}
}
