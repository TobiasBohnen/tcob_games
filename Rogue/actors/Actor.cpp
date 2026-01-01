// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Actor.hpp"

#include "../MasterControl.hpp"

namespace Rogue {
////////////////////////////////////////////////////////////

actor::actor(master_control& parent, profile profile)
    : _parent {parent}
    , _profile {std::move(profile)}
{
    _profile.HP = hp_max();
    _profile.MP = mp_max();
}

auto actor::strength_check(f32 prob) const -> bool
{
    return check(_profile.Attributes.Strength, prob);
}

auto actor::intelligence_check(f32 prob) const -> bool
{
    return check(_profile.Attributes.Intelligence, prob);
}

auto actor::vitality_check(f32 prob) const -> bool
{
    return check(_profile.Attributes.Vitality, prob);
}

auto actor::agility_check(f32 prob) const -> bool
{
    return check(_profile.Attributes.Agility, prob);
}

auto actor::dexterity_check(f32 prob) const -> bool
{
    return check(_profile.Attributes.Dexterity, prob);
}

auto actor::check(i32 attr, f32 prob) const -> bool
{
    prob = std::clamp(prob, 0.0f, 1.0f);
    return _parent.rand() < prob * (static_cast<f32>(attr) / 10.0f);
}

auto actor::get_profile() const -> profile const&
{
    return _profile;
}

auto actor::get_profile() -> profile&
{
    // TODO: add inventory, effects, etc.
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

auto actor::hp_max() const -> i32
{
    return _profile.Attributes.Vitality + _profile.Magic.Life;
}

auto actor::mp_max() const -> i32
{
    return _profile.Attributes.Intelligence + _profile.Magic.Energy;
}

auto actor::parent() -> master_control&
{
    return _parent;
}

}
