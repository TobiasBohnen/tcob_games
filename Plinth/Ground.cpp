// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Ground.hpp"

auto ground::at(usize id) const -> ground_settings const&
{
    static ground_settings const none {};
    return id < _settings.size() ? _settings[id] : none;
}

void ground::set(usize id, ground_settings const& settings)
{
    if (id >= _settings.size()) { _settings.resize(id + 1); }
    _settings[id] = settings;
}
